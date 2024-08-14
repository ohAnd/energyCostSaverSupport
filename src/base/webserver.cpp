#include <base/webserver.h>

uint8_t rebootRequestedInSec = 0;
boolean rebootRequested = false;
boolean wifiScanIsRunning = false;

size_t content_len;

ESPwebserver::ESPwebserver()
{
    // Constructor implementation
}

ESPwebserver::~ESPwebserver()
{
    stop();                  // Ensure the server is stopped and resources are cleaned up
    webServerTimer.detach(); // Stop the timer
}

void ESPwebserver::backgroundTask(ESPwebserver *instance)
{
    if (rebootRequested)
    {
        if (rebootRequestedInSec-- == 1)
        {
            rebootRequestedInSec = 0;
            rebootRequested = false;
            Serial.println(F("WEB:\t\t backgroundTask - reboot requested"));
            Serial.flush();
            ESP.restart();
        }
        Serial.println(F("WEB:\t\t backgroundTask - reboot requested with delay"));
    }
    // if (updateInfo.updateRunning)
    // {
    //     Serial.println("OTA UPDATE:\t Progress: " + String(updateInfo.updateProgress, 1) + " %");
    // }
}

void ESPwebserver::start()
{
    // Initialize the web server and define routes as before
    Serial.println(F("WEB:\t\t setup webserver"));
    // base web pages
    asyncEspWebServer.on("/", HTTP_GET, handleRoot);
    asyncEspWebServer.on("/jquery.min.js", HTTP_GET, handleJqueryMinJs);
    asyncEspWebServer.on("/style.css", HTTP_GET, handleCSS);

    // user config requests
    asyncEspWebServer.on("/updateWifiSettings", handleUpdateWifiSettings);
    asyncEspWebServer.on("/updateErnergyCostSettings", handleUpdateErnergyCostSettings);
    asyncEspWebServer.on("/updateDeviceDataSettings", handleUpdateDeviceDataSettings);

    // admin config requests
    asyncEspWebServer.on("/config", handleConfigPage);

    // direct settings
    asyncEspWebServer.on("/getWifiNetworks", handleGetWifiNetworks);

    // api GETs
    asyncEspWebServer.on("/api/data.json", handleDataJson);
    asyncEspWebServer.on("/api/info.json", handleInfojson);

    // OTA direct update
    asyncEspWebServer.on("/updateOTASettings", handleUpdateOTASettings);
    asyncEspWebServer.on("/updateGetInfo", handleUpdateInfoRequest);

    // OTA update
    asyncEspWebServer.on("/doupdate", HTTP_POST, [](AsyncWebServerRequest *request) {}, [](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
                         { handleDoUpdate(request, filename, index, data, len, final); });
    asyncEspWebServer.on("/updateState", HTTP_GET, handleUpdateProgress);

    asyncEspWebServer.onNotFound(notFound);

    asyncEspWebServer.begin(); // Start the web server
    webServerTimer.attach(1, ESPwebserver::backgroundTask, this);
#ifdef ESP32
    Update.onProgress(printProgress);
#endif
}

void ESPwebserver::stop()
{
    asyncEspWebServer.end(); // Stop the web server
    webServerTimer.detach(); // Stop the timer
}

// base pages
void ESPwebserver::handleRoot(AsyncWebServerRequest *request)
{
    request->send_P(200, "text/html", INDEX_HTML);
}
void ESPwebserver::handleCSS(AsyncWebServerRequest *request)
{
    request->send_P(200, "text/html", STYLE_CSS);
}
void ESPwebserver::handleJqueryMinJs(AsyncWebServerRequest *request)
{
    request->send_P(200, "text/html", JQUERY_MIN_JS);
}

// ota update
void ESPwebserver::handleDoUpdate(AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
{
    if (!index)
    {
        updateInfo.updateRunning = true;
        updateInfo.updateState = UPDATE_STATE_PREPARE;
        Serial.println("OTA UPDATE:\t Update Start with file: " + filename);
        Serial.println("OTA UPDATE:\t waiting to stop services");
        delay(500);
        Serial.println("OTA UPDATE:\t services stopped - start update");
        content_len = request->contentLength();
        // if filename includes spiffs, update the spiffs partition
        int cmd = (filename.indexOf("spiffs") > -1) ? U_PART : U_FLASH;
#ifdef ESP8266
        Update.runAsync(true);
        if (!Update.begin(content_len, cmd))
        {
#else
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, cmd))
        {
#endif
            Update.printError(Serial);
            updateInfo.updateState = UPDATE_STATE_FAILED;
        }
    }

    if (Update.write(data, len) != len)
    {
        Update.printError(Serial);
        updateInfo.updateState = UPDATE_STATE_FAILED;
#ifdef ESP8266
    }
    else
    {
        updateInfo.updateProgress = (Update.progress() * 100) / Update.size();
        // Serial.println("OTA UPDATE:\t ESP8266 Progress: " + String(updateInfo.updateProgress, 1) + " %");
#endif
    }

    if (final)
    {
        // AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Please wait while the device reboots");
        // response->addHeader("Refresh", "20");
        // response->addHeader("Location", "/");
        // request->send(response);
        if (!Update.end(true))
        {
            Update.printError(Serial);
            updateInfo.updateState = UPDATE_STATE_FAILED;
        }
        else
        {
            Serial.println("OTA UPDATE:\t Update complete");
            updateInfo.updateState = UPDATE_STATE_DONE;
            updateInfo.updateRunning = false;
            Serial.flush();
            rebootRequestedInSec = 3;
            rebootRequested = true;
            // delay(1000);
            // ESP.restart();
        }
    }
}
void ESPwebserver::printProgress(size_t prg, size_t sz)
{
    updateInfo.updateProgress = (prg * 100) / content_len;
    // Serial.println("OTA UPDATE:\t ESP32 Progress: " + String(updateInfo.updateProgress, 1) + " %");
}
void ESPwebserver::handleUpdateProgress(AsyncWebServerRequest *request)
{
    String JSON = "{";
    JSON += "\"updateRunning\": " + String(updateInfo.updateRunning) + ",";
    JSON += "\"updateState\": " + String(updateInfo.updateState) + ",";
    JSON += "\"updateProgress\": " + String(updateInfo.updateProgress);
    JSON += "}";
    request->send(200, "application/json", JSON);
}

// admin config
void ESPwebserver::handleConfigPage(AsyncWebServerRequest *request)
{
    JsonDocument doc;
    bool gotUserChanges = false;
    if (request->params() && request->hasParam("local.wifiAPstart", true))
    {
        if (request->getParam("local.wifiAPstart", true)->value() == "false")
        {
            Serial.println(F("WEB:\t\t handleConfigPage - got user changes"));
            gotUserChanges = true;
            for (unsigned int i = 0; i < request->params(); i++)
            {
                String key = request->argName(i);
                String value = request->arg(key);

                String key1 = key.substring(0, key.indexOf("."));
                String key2 = key.substring(key.indexOf(".") + 1);

                if (value == "false" || value == "true")
                {
                    bool boolValue = (value == "true");
                    doc[key1][key2] = boolValue;
                }
                else
                    doc[key1][key2] = value;
            }
        }
    }

    String html = configManager.getWebHandler(doc);
    request->send_P(200, "text/html", html.c_str());
    if (gotUserChanges)
    {
        Serial.println(F("WEB:\t\t handleConfigPage - got User Changes - sent back config page ack - and restart ESP in 2 seconds"));
        rebootRequestedInSec = 3;
        rebootRequested = true;
    }
}

// serve json as api
void ESPwebserver::handleDataJson(AsyncWebServerRequest *request)
{
    String JSON = "{";
    JSON = JSON + "\"ntpStamp\": " + String(platformData.currentNTPtimeGMT) + ",";
    JSON = JSON + "\"starttime\": " + String(platformData.espStarttimeGMT) + ",";
    JSON = JSON + "\"lastResponse\": " + String(platformData.lastCostDataUpdateGMT) + ",";
    JSON = JSON + "\"energyCostsLastEntry\": " + String(platformData.pricePerKWhLast) + ",";
    JSON = JSON + "\"energyCosts\": [";
    // rund through platformData.pricePerKWh and add to JSON
    for (size_t i = 0; i < 24; i++)
    {
        JSON += "{\"value\": " + String(platformData.pricePerKWh[i]) + "}";
        if (i != 24 - 1)
            JSON += ",";
        }
    JSON = JSON + "],";

    JSON = JSON + "\"energyCostSettings\": {",
    JSON = JSON + "\"fixedPricePerKWh\": " + String(userConfig.fixedPricePerKWh,5) + ",";
    JSON = JSON + "\"fixedTaxPricePerKWh\": " + String(userConfig.fixedTaxPricePerKWh,5) + ",";
    JSON = JSON + "\"taxVarPricePerKWH\": " + String(userConfig.taxVarPricePerKWH); 
    JSON = JSON + "},",

    JSON = JSON + "\"device\": {",
    JSON = JSON + "\"deviceName\": \"" + String(userConfig.deviceName) + "\",";
    JSON = JSON + "\"maxWaitTime\": " + String(userConfig.maxWaitTime) + ",";
    JSON = JSON + "\"tgtDurationInHours\": " + String(userConfig.tgtDurationInHours);    
    JSON = JSON + "},",

    JSON = JSON + "\"result\": {",
    JSON = JSON + "\"currentHour\": " + String(platformData.currentHour) + ",";
    JSON = JSON + "\"tgtStartHour\": " + String(platformData.tgtStartHour) + ",";
    JSON = JSON + "\"tgtDelayHours\": " + String(platformData.tgtDelayHours) + ",";
    JSON = JSON + "\"energyCostNow\": " + String(platformData.energyCostNow) + ",";
    JSON = JSON + "\"energyCostSave\": " + String(platformData.energyCostSave);
    JSON = JSON + "}",

    JSON = JSON + "}";

    request->send(200, "application/json; charset=utf-8", JSON);
}

void ESPwebserver::handleInfojson(AsyncWebServerRequest *request)
{
    String JSON = "{";
    JSON = JSON + "\"chipid\": " + String(platformData.chipID) + ",";
    JSON = JSON + "\"host\": \"" + platformData.espUniqueName + "\",";
    JSON = JSON + "\"initMode\": " + userConfig.wifiAPstart + ",";

    JSON = JSON + "\"firmware\": {";
    JSON = JSON + "\"version\": \"" + String(platformData.fwVersion) + "\",";
    JSON = JSON + "\"versiondate\": \"" + String(platformData.fwBuildDate) + "\",";
    JSON = JSON + "\"versionServer\": \"" + String(updateInfo.versionServer) + "\",";
    JSON = JSON + "\"versiondateServer\": \"" + String(updateInfo.versiondateServer) + "\",";
    JSON = JSON + "\"versionServerRelease\": \"" + String(updateInfo.versionServerRelease) + "\",";
    JSON = JSON + "\"versiondateServerRelease\": \"" + String(updateInfo.versiondateServerRelease) + "\",";
    JSON = JSON + "\"selectedUpdateChannel\": \"" + String(userConfig.selectedUpdateChannel) + "\",";
    JSON = JSON + "\"updateAvailable\": " + updateInfo.updateAvailable;
    JSON = JSON + "},";

    JSON = JSON + "\"wifiConnection\": {";
    JSON = JSON + "\"wifiSsid\": \"" + String(userConfig.wifiSsid) + "\",";
    JSON = JSON + "\"wifiPassword\": \"" + String(userConfig.wifiPassword) + "\",";
    JSON = JSON + "\"rssiGW\": " + platformData.wifi_rssi_gateway + ",";
    JSON = JSON + "\"wifiScanIsRunning\": " + wifiScanIsRunning + ",";
    JSON = JSON + "\"networkCount\": " + platformData.wifiNetworkCount + ",";
    JSON = JSON + "\"foundNetworks\":" + platformData.wifiFoundNetworks;
    JSON = JSON + "}";

    JSON = JSON + "}";

    request->send(200, "application/json; charset=utf-8", JSON);
}

// user config
void ESPwebserver::handleUpdateWifiSettings(AsyncWebServerRequest *request)
{
    if (request->hasParam("wifiSSIDsend", true) && request->hasParam("wifiPASSsend", true))
    {
        String wifiSSIDUser = request->getParam("wifiSSIDsend", true)->value(); // server.arg("wifiSSIDsend"); // retrieve message from webserver
        String wifiPassUser = request->getParam("wifiPASSsend", true)->value(); // server.arg("wifiPASSsend"); // retrieve message from webserver
        Serial.println("WEB:\t\t handleUpdateWifiSettings - got WifiSSID: " + wifiSSIDUser + " - got WifiPass: " + wifiPassUser);

        wifiSSIDUser.toCharArray(userConfig.wifiSsid, sizeof(userConfig.wifiSsid));
        wifiPassUser.toCharArray(userConfig.wifiPassword, sizeof(userConfig.wifiPassword));

        // after saving from user entry - no more in init state
        if (userConfig.wifiAPstart)
        {
            userConfig.wifiAPstart = false;
            // after first startup reset to current display
            if (userConfig.displayConnected == 0)
            {
                userConfig.displayConnected = 1;
                // displayTFT.setup(); // new setup to get blank screen
            }
            else if (userConfig.displayConnected == 1)
                userConfig.displayConnected = 0;

            // and schedule a reboot to start fresh with new settings
            rebootRequestedInSec = 2;
            rebootRequested = true;
        }

        configManager.saveConfig(userConfig);

        String JSON = "{";
        JSON = JSON + "\"wifiSSIDUser\": \"" + userConfig.wifiSsid + "\",";
        JSON = JSON + "\"wifiPassUser\": \"" + userConfig.wifiPassword + "\",";
        JSON = JSON + "}";

        request->send(200, "application/json", JSON);

        // reconnect with new values
        // WiFi.disconnect();
        // WiFi.mode(WIFI_STA);
        // checkWifiTask();

        Serial.println("WEB:\t\t handleUpdateWifiSettings - send JSON: " + String(JSON));
    }
    else
    {
        request->send(400, "text/plain", "handleUpdateWifiSettings - ERROR requested without the expected params");
        Serial.println(F("handleUpdateWifiSettings - ERROR without the expected params"));
    }
}

void ESPwebserver::handleUpdateErnergyCostSettings(AsyncWebServerRequest *request)
{
    if (request->hasParam("fixedPricePerKWhSend", true) &&
        request->hasParam("fixedTaxPricePerKWhSend", true) &&
        request->hasParam("taxVarPricePerKWHSend", true))
    {
        String fixedPricePerKWhSend = request->getParam("fixedPricePerKWhSend", true)->value(); // retrieve message from webserver
        String fixedTaxPricePerKWhSend = request->getParam("fixedTaxPricePerKWhSend", true)->value();           // retrieve message from webserver
        String taxVarPricePerKWHSend = request->getParam("taxVarPricePerKWHSend", true)->value();         // retrieve message from webserver
        
        Serial.println("WEB:\t\t handleUpdateErnergyCostSettings - got fixedPricePerKWhSend: " + fixedPricePerKWhSend + " - got fixedTaxPricePerKWhSend: " + fixedTaxPricePerKWhSend + " - got taxVarPricePerKWHSend: " + taxVarPricePerKWHSend);

        userConfig.fixedPricePerKWh = fixedPricePerKWhSend.toFloat();
        userConfig.fixedTaxPricePerKWh = fixedTaxPricePerKWhSend.toFloat();
        userConfig.taxVarPricePerKWH = taxVarPricePerKWHSend.toFloat();

        configManager.saveConfig(userConfig);

        String JSON = "{";
        JSON = JSON + "\"fixedPricePerKWh\": " + String(userConfig.fixedPricePerKWh,5) + ",";
        JSON = JSON + "\"fixedTaxPricePerKWh\": " + String(userConfig.fixedTaxPricePerKWh,5) + ",";
        JSON = JSON + "\"taxVarPricePerKWH\": " + String(userConfig.taxVarPricePerKWH);
        JSON = JSON + "}";

        request->send(200, "application/json", JSON);
        Serial.println("WEB:\t\t handleUpdateErnergyCostSettings - send JSON: " + String(JSON));
        platformData.userSettingsChanged = true;
    }
    else
    {
        request->send(400, "text/plain", "handleUpdateErnergyCostSettings - ERROR requested without the expected params");
        Serial.println(F("WEB:\t\t handleUpdateErnergyCostSettings - ERROR without the expected params"));
    }
}

void ESPwebserver::handleUpdateDeviceDataSettings(AsyncWebServerRequest *request)
{
    if (request->hasParam("deviceNameSend", true) &&
        request->hasParam("deviceRuntimeSend", true) &&
        request->hasParam("deviceMaxWaittimeSend", true))
    {
        String deviceNameSend = request->getParam("deviceNameSend", true)->value(); // retrieve message from webserver
        String deviceRuntimeSend = request->getParam("deviceRuntimeSend", true)->value();
        String deviceMaxWaittimeSend = request->getParam("deviceMaxWaittimeSend", true)->value();

        userConfig.deviceName = deviceNameSend;
        userConfig.tgtDurationInHours = deviceRuntimeSend.toInt();
        userConfig.maxWaitTime = deviceMaxWaittimeSend.toInt();

        configManager.saveConfig(userConfig);



        String JSON = "{";
        JSON = JSON + "\"deviceName\": \"" + userConfig.deviceName + "\",";
        JSON = JSON + "\"tgtDurationInHours\": " + String(userConfig.tgtDurationInHours) + ",";
        JSON = JSON + "\"maxWaitTime\": " + String(userConfig.maxWaitTime);
        JSON = JSON + "}";

        request->send(200, "application/json", JSON);
        Serial.println("WEB:\t\t handleUpdateDeviceDataSettings - send JSON: " + String(JSON));
        platformData.userSettingsChanged = true;
    }
    else
    {
        request->send(400, "text/plain", "handleUpdateDeviceDataSettings - ERROR request without the expected params");
        Serial.println(F("WEB:\t\t handleUpdateDeviceDataSettings - ERROR without the expected params"));
    }
}

// direct wifi scan
void ESPwebserver::setWifiScanIsRunning(bool state)
{
    wifiScanIsRunning = state;
}

void ESPwebserver::handleGetWifiNetworks(AsyncWebServerRequest *request)
{
    if (!wifiScanIsRunning)
    {
        WiFi.scanNetworks(true);

        request->send(200, "application/json", "{\"wifiNetworks\": \"scan started\"}");
        Serial.println(F("ESPwebserver:\t -> WIFI_SCAN: start async scan"));
        wifiScanIsRunning = true;
    }
    else
    {
        request->send(200, "application/json", "{\"wifiNetworks\": \"scan already running\"}");
        Serial.println(F("ESPwebserver:\t -> WIFI_SCAN: scan already running"));
    }
}

// OTA settings
void ESPwebserver::handleUpdateOTASettings(AsyncWebServerRequest *request)
{
    if (request->hasParam("releaseChannel", true))
    {
        String releaseChannel = request->getParam("releaseChannel", true)->value(); // retrieve message from webserver
        Serial.println("WEB:\t\t handleUpdateOTASettings - got releaseChannel: " + releaseChannel);

        userConfig.selectedUpdateChannel = releaseChannel.toInt();

        //   configManager.saveConfig(userConfig);

        String JSON = "{";
        JSON = JSON + "\"releaseChannel\": \"" + userConfig.selectedUpdateChannel + "\"";
        JSON = JSON + "}";

        request->send(200, "application/json", JSON);
        Serial.println("WEB:\t\t handleUpdateDtuSettings - send JSON: " + String(JSON));

        // trigger new update info with changed release channel
        // getUpdateInfo(AsyncWebServerRequest *request);
        updateInfo.updateInfoRequested = true;
    }
    else
    {
        request->send(400, "text/plain", "handleUpdateOTASettings - ERROR requested without the expected params");
        Serial.println(F("WEB:\t\t handleUpdateOTASettings - ERROR without the expected params"));
    }
}

void ESPwebserver::handleUpdateInfoRequest(AsyncWebServerRequest *request)
{
    updateInfo.updateInfoRequested = true;
    request->send(200, "application/json", "{\"updateInfo.updateInfoRequested\": \"done\"}");
}

// default

void ESPwebserver::notFound(AsyncWebServerRequest *request)
{
    request->send(404, "text/plain", "Not found");
}

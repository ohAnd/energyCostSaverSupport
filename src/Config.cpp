// Config.cpp
#include "Config.h"

struct UserConfig userConfig;

// Default values
UserConfigManager::UserConfigManager(const char *filePath, const UserConfig &defaultConfig)
    : filePath(filePath), defaultConfig(defaultConfig) {}

bool UserConfigManager::begin()
{
    if (!LittleFS.begin())
    {

        Serial.println(F("UserConfigManager::begin - An error has occurred while mounting LittleFS"));
#if defined(ESP32)
        // specific to ESP32 because it uses the ESP32-specific LittleFS.begin(true) function to format the filesystem if mounting fails.
        if (!LittleFS.begin(true))
        {
            Serial.println(F("... tried to format filesystem also failed."));
        }
        else
        {
            Serial.println(F("... successfully formatted filesystem."));
        }
#endif
        return false;
    }

    UserConfig config;
    if (!loadConfig(config))
    {
        Serial.println(F("UserConfigManager::begin - First run: Initializing config"));
        saveConfig(defaultConfig);
    }
    else
    {
        Serial.println(F("UserConfigManager::begin - Config loaded successfully"));
    }
    return true;
}

bool UserConfigManager::loadConfig(UserConfig &config)
{
    if (!LittleFS.exists(filePath))
    {
        return false;
    }
    File file = LittleFS.open(filePath, "r");
    if (!file)
    {
        Serial.println("UserConfigManager::loadConfig - Failed to open file for reading");
        return false;
    }
    // file.read((uint8_t *)&config, sizeof(UserConfig));
    JsonDocument doc;

    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
        Serial.print("UserConfigManager::loadConfig - deserializeJson() fehlgeschlagen: ");
        Serial.println(error.c_str());
    }

    mappingJsonToStruct(doc);
    if (userConfig.espUpdateTime == 0)
    {
        Serial.println(F("UserConfigManager::loadConfig --- ERROR: config corrupted, reset to default"));
        saveConfig(defaultConfig);
    }
    else
    {
        Serial.println("UserConfigManager::loadConfig - config loaded from json: " + String(filePath));
    }

    file.close();
    return true;
}

void UserConfigManager::saveConfig(const UserConfig &config)
{
    File file = LittleFS.open(filePath, "w");
    if (!file)
    {
        Serial.println(F("Failed to open file for writing"));
        return;
    }
    JsonDocument doc;
    doc = mappingStructToJson(config);
    serializeJson(doc, file);

    Serial.println("config saved to json: " + String(filePath));

    file.close();
}

void UserConfigManager::resetConfig()
{
    if (LittleFS.remove(filePath))
    {
        Serial.println(F("Config file deleted successfully"));
    }
    else
    {
        Serial.println(F("Failed to delete config file"));
    }
}

void UserConfigManager::printConfigdata()
{
    // Configuration has been written before
    Serial.print(F("\n--------------------------------------\n"));
    Serial.print(F("Configuration loaded from config file: '/userconfig.json'\n"));
    Serial.print(F("init (wifiAPstart): \t"));
    Serial.println(userConfig.wifiAPstart);

    Serial.print(F("wifi ssid: \t\t"));
    Serial.println(userConfig.wifiSsid);
    Serial.print(F("wifi pass: \t\t"));
    Serial.println(userConfig.wifiPassword);

    Serial.print(F("update channel: \t\t"));
    Serial.println(userConfig.selectedUpdateChannel);

    Serial.print(F("timezone offset: \t"));
    Serial.println(userConfig.timezoneOffest);

    Serial.print(F("espUpdateTime: \t\t"));
    Serial.println(userConfig.espUpdateTime);

    Serial.print(F("display connected: \t"));
    Serial.println(userConfig.displayConnected);

    Serial.print(F("\ndevice data\n"));
    Serial.print(F("device name: \t\t\t"));
    Serial.println(userConfig.deviceName);
    Serial.print(F("max wait time: \t\t\t"));
    Serial.println(userConfig.maxWaitTime);
    Serial.print(F("deviceDelayModeForward: \t\t"));
    Serial.println(userConfig.deviceDelayModeForward);
    Serial.print(F("tgt duration in hours: \t\t"));
    Serial.println(userConfig.tgtDurationInHours);
    Serial.print(F("tgtDurationConsumption: \t\t"));
    Serial.println(userConfig.tgtDurationConsumption);


    Serial.print(F("\nenergy cost settings\n"));
    Serial.print(F("fixed tax price per kwh: \t"));
    Serial.println(String(userConfig.fixedTaxPricePerKWh,5));
    Serial.print(F("fixed price per kwh: \t\t"));
    Serial.println(String(userConfig.fixedPricePerKWh,5));
    Serial.print(F("tax fixed preced per kwh: \t"));
    Serial.println(String(userConfig.taxVarPricePerKWH, 2));

    Serial.print(F("--------------------------------------\n"));
}

JsonDocument UserConfigManager::mappingStructToJson(const UserConfig &config)
{
    JsonDocument doc;

    doc["wifi"]["ssid"] = config.wifiSsid;
    doc["wifi"]["pass"] = config.wifiPassword;

    doc["display"]["type"] = config.displayConnected;

    doc["esp"]["updateTime"] = config.espUpdateTime;

    doc["local"]["selectedUpdateChannel"] = config.selectedUpdateChannel;
    doc["local"]["wifiAPstart"] = config.wifiAPstart;
    doc["local"]["timezoneOffest"] = config.timezoneOffest;

    doc["device"]["deviceName"] = config.deviceName;
    doc["device"]["maxWaitTime"] = config.maxWaitTime;
    doc["device"]["deviceDelayModeForward"] = config.deviceDelayModeForward;
    doc["device"]["tgtDurationInHours"] = config.tgtDurationInHours;
    doc["device"]["tgtDurationConsumption"] = config.tgtDurationConsumption;

    doc["energyCostSettings"]["fixedTaxPricePerKWh"] = config.fixedTaxPricePerKWh;
    doc["energyCostSettings"]["fixedPricePerKWh"] = config.fixedPricePerKWh;
    doc["energyCostSettings"]["taxVarPricePerKWH"] = config.taxVarPricePerKWH;
    
    return doc;
}

void UserConfigManager::mappingJsonToStruct(JsonDocument doc)
{
    String(doc["wifi"]["ssid"].as<String>()).toCharArray(userConfig.wifiSsid, sizeof(userConfig.wifiSsid));
    String(doc["wifi"]["pass"].as<String>()).toCharArray(userConfig.wifiPassword, sizeof(userConfig.wifiPassword));

    userConfig.displayConnected = doc["display"]["type"];

    userConfig.espUpdateTime = doc["esp"]["updateTime"].as<int>();

    userConfig.selectedUpdateChannel = doc["local"]["selectedUpdateChannel"].as<int>();
    userConfig.wifiAPstart = doc["local"]["wifiAPstart"].as<bool>();
    userConfig.timezoneOffest = doc["local"]["timezoneOffest"].as<int>();

    userConfig.deviceName = doc["device"]["deviceName"].as<String>();
    userConfig.maxWaitTime = doc["device"]["maxWaitTime"].as<uint8_t>();
    userConfig.deviceDelayModeForward = doc["device"]["deviceDelayModeForward"].as<bool>();
    userConfig.tgtDurationInHours = doc["device"]["tgtDurationInHours"].as<uint8_t>();
    userConfig.tgtDurationConsumption = doc["device"]["tgtDurationConsumption"];

    userConfig.fixedTaxPricePerKWh = doc["energyCostSettings"]["fixedTaxPricePerKWh"];
    userConfig.fixedPricePerKWh = doc["energyCostSettings"]["fixedPricePerKWh"];
    userConfig.taxVarPricePerKWH = doc["energyCostSettings"]["taxVarPricePerKWH"];
    
    return;
}

// reused from https://github.com/Tvde1/ConfigTool/blob/master/src/ConfigTool.cpp

String UserConfigManager::createWebPage(bool updated)
{
    // Serial.println(F("\nCONFIG web - START generate html page for config interface"));

    const String beginHtml = F("<html><head><title>dtuGateway Configuration Interface</title><link rel=\"stylesheet\"href=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.0/css/bootstrap.min.css\"integrity=\"sha384-9gVQ4dYFwwWSjIDZnLEWnxCjeSWFphJiwGPXr1jddIhOegiu1FwO5qRGvFXOdJZ4\"crossorigin=\"anonymous\"><script src=\"https://code.jquery.com/jquery-3.3.1.slim.min.js\"integrity=\"sha384-q8i/X+965DzO0rT7abK41JStQIAqVgRVzpbzo5smXKp4YfRvH+8abtTE1Pi6jizo\"crossorigin=\"anonymous\"></script><script src=\"https://cdnjs.cloudflare.com/ajax/libs/popper.js/1.14.0/umd/popper.min.js\"integrity=\"sha384-cs/chFZiN24E4KMATLdqdvsezGxaGsi4hLGOzlXwp5UZB1LY//20VyM2taTB4QvJ\"crossorigin=\"anonymous\"></script><script src=\"https://stackpath.bootstrapcdn.com/bootstrap/4.1.0/js/bootstrap.min.js\"integrity=\"sha384-uefMccjFJAIv6A+rW+L4AHf99KvxDjWSu1z9VI8SKNVmz4sk7buKt/6v9KI65qnm\"crossorigin=\"anonymous\"></script></head><body><div class=\"container\"><div class=\"jumbotron\"style=\"width:100%\"><h1>dtuGateway Configuration Interface</h1><p>Edit the config variables here and click save.<br>After the configuration is saved, a reboot will be triggered. </p></div>");
    const String continueHtml = F("<form method=\"POST\" action=\"\">");
    const String savedAlert = F("<div class=\"alert alert-success\" role=\"alert\"><button type=\"button\" class=\"close\" data-dismiss=\"alert\" aria-label=\"Close\"><span aria-hidden=\"true\">&times;</span></button>The config has been saved. And the device will be rebooted</div>");

    // const String endHtml = "<div class=\"form-group row\"><div class=\"col-sm-1\"><button class=\"btn btn-primary\" type=\"submit\">Save</button></div><div class=\"col-sm-1 offset-sm-0\"><button type=\"button\" class=\"btn btn-danger\" onclick=\"reset()\">Reset</button></div></div></form></div></body><script>function reset(){var url=window.location.href;if(url.indexOf('?')>0){url=url.substring(0,url.indexOf('?'));}url+='?reset=true';window.location.replace(url);}</script></html>";
    const String endHtml = F("<div class=\"form-group row\"><div class=\"col-sm-1\"><button class=\"btn btn-primary\" type=\"submit\">Save</button></div></div></form></div></body><script>function reset(){var url=window.location.href;if(url.indexOf('?')>0){url=url.substring(0,url.indexOf('?'));}url+='?reset=true';window.location.replace(url);}</script></html>");

    String result = beginHtml;

    if (updated)
    {
        result += savedAlert;
    }

    result += continueHtml;

    JsonDocument doc;
    doc = mappingStructToJson(userConfig);

    JsonObject obj = doc.as<JsonObject>();

    for (JsonPair kv : obj)
    {
        String mainKey = String(kv.key().c_str());
        result += "<div><label><h4>" + mainKey + "</h4></label></div>";
        // 1 layer below
        JsonObject obj1 = (kv.value()).as<JsonObject>();
        for (JsonPair kv1 : obj1)
        {
            String key = kv1.key().c_str();
            String value = kv1.value();
            result += "<div class=\"form-group row\"><div class=\"col-2\"><label>" + key + "</label></div><div class=\"col-10\"><input name=\"" + mainKey + "." + key + "\" class=\"form-control\" type=\"text\" value=\"" + value + "\" /></div></div>";
        }
    }

    result += endHtml;

    // Serial.println(F("\nCONFIG web - END generate html page for config interface"));

    return result;
}

String UserConfigManager::getWebHandler(JsonDocument doc)
{
    // JsonDocument docConfigNew;
    bool updated = false;
    if (!doc.isNull())
    {
        File file = LittleFS.open("/userconfig.json", "w");
        if (!file)
        {
            Serial.println(F("Failed to open file for writing"));
            return "<html><body>ERROR - failed to open /userconfig.json</body></html>";
        }
        serializeJson(doc, file);
        // serializeJsonPretty(doc, Serial);
        Serial.println("WEBconfig - config saved to json: " + String(filePath));
        file.close();

        loadConfig(userConfig);
        printConfigdata();
        updated = true;
    }
    else
    {
        Serial.println(F("\nCONFIG web - show current config"));
    }

    String html = createWebPage(updated);
    return html;
}
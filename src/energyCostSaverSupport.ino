#include <UnixTime.h>
#if defined(ESP8266)
#include <ESP8266TimerInterrupt.h>
#include <ESP8266_ISR_Timer.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#elif defined(ESP32)
#include <ESP32TimerInterrupt.h>
#include <ESP32_ISR_Timer.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <map>
#endif

#include <WiFiUdp.h>
#include <NTPClient.h>

#include <ArduinoJson.h>

#include <base/webserver.h>
#include <base/platformData.h>

#include <display.h>
#include <displayTFT.h>

#include "Config.h"

// ---> START initializing here and publishishing allover project over platformData.h
baseDataStruct platformData;

baseUpdateInfoStruct updateInfo;

const long interval50ms = 50;   // interval (milliseconds)
const long interval100ms = 100; // interval (milliseconds)
const long intervalShort = 1;   // interval (seconds)
const long interval5000ms = 5;  // interval (seconds)
const long intervalLong = 60;   // interval (seconds)
unsigned long previousMillis50ms = 0;
unsigned long previousMillis100ms = 0;
unsigned long previousMillisShort = 1704063600;  // in seconds
unsigned long previousMillis5000ms = 1704063600; // in seconds
// costDataUpdateCycleInSeconds = 1704063600; -> with platformData
unsigned long previousMillisLong = 1704063600;

#define WIFI_RETRY_TIME_SECONDS 30
#define WIFI_RETRY_TIMEOUT_SECONDS 15
#define RECONNECTS_ARRAY_SIZE 50
unsigned long reconnects[RECONNECTS_ARRAY_SIZE];
int reconnectsCnt = -1; // first needed run inkrement to 0

// intervall for getting and sending temp
// Select a Timer Clock
#define USING_TIM_DIV1 false   // for shortest and most accurate timer
#define USING_TIM_DIV16 true   // for medium time and medium accurate timer
#define USING_TIM_DIV256 false // for longest timer but least accurate. Default

struct controls
{
  boolean wifiSwitch = true;
  boolean getDataAuto = true;
  boolean getDataOnce = false;
  boolean dataFormatJSON = false;
};
controls globalControls;

// wifi functions
boolean wifi_connecting = false;
int wifiTimeoutShort = WIFI_RETRY_TIMEOUT_SECONDS;
int wifiTimeoutLong = WIFI_RETRY_TIME_SECONDS;

// <--- END initializing here and published over platformData.h

// blink code for status display
#if defined(ESP8266)
// #define LED_BLINK LED_BUILTIN
#define LED_BLINK 2
#define LED_BLINK_ON LOW
#define LED_BLINK_OFF HIGH
#elif defined(ESP32)
#define LED_BLINK 2 // double occupancy with TFT display SPI DC pin
#define LED_BLINK_ON HIGH
#define LED_BLINK_OFF LOW
#endif

#define BLINK_NORMAL_CONNECTION 0    // 1 Hz blip - normal connection and running
#define BLINK_WAITING_NEXT_TRY_DTU 1 // 1 Hz - waiting for next try to connect to DTU
#define BLINK_WIFI_OFF 2             // 2 Hz - wifi off
#define BLINK_TRY_CONNECT_DTU 3      // 5 Hz - try to connect to DTU
#define BLINK_PAUSE_CLOUD_UPDATE 4   // 0,5 Hz blip - DTO - Cloud update
int8_t blinkCode = BLINK_WIFI_OFF;

// user config
UserConfigManager configManager;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP); // By default 'pool.ntp.org' is used with 60 seconds update interval

ESPwebserver espWebServer;

Display displayOLED;
DisplayTFT displayTFT;

// Init ESP8266 only and only Timer 1
#if defined(ESP8266)
ESP8266Timer ITimer;
#elif defined(ESP32)
ESP32Timer ITimer(0);
#endif
#define TIMER_INTERVAL_MS 1000

// user config
uint8_t tgtConsupmtionFactor = 1.2; // average consumption per hour e.g. washing machine needs 3.6 kWh in 3 hours -> factor = 1.2
uint8_t maxDelayHours = 12;

boolean checkWifiTask()
{
  if (WiFi.status() != WL_CONNECTED && !wifi_connecting) // start connecting wifi
  {
    // reconnect counter - and reset to default
    reconnects[reconnectsCnt++] = platformData.currentNTPtime;
    if (reconnectsCnt >= 25)
    {
      reconnectsCnt = 0;
      Serial.println(F("CheckWifi:\t  no Wifi connection after 25 tries!"));
      // after 20 reconnects inner 7 min - write defaults
      if ((platformData.currentNTPtime - reconnects[0]) < (WIFI_RETRY_TIME_SECONDS * 1000)) //
      {
        Serial.println(F("CheckWifi:\t no Wifi connection after 5 tries and inner 5 minutes"));
      }
    }

    // try to connect with current values
    Serial.println("CheckWifi:\t No Wifi connection! Connecting... try to connect to wifi: '" + String(userConfig.wifiSsid) + "' with pass: '" + userConfig.wifiPassword + "'");

    WiFi.disconnect();
    WiFi.begin(userConfig.wifiSsid, userConfig.wifiPassword);
    wifi_connecting = true;
    blinkCode = BLINK_TRY_CONNECT_DTU;

    // startServices();
    return false;
  }
  else if (WiFi.status() != WL_CONNECTED && wifi_connecting && wifiTimeoutShort > 0) // check during connecting wifi and decrease for short timeout
  {
    // Serial.printf("CheckWifi:\t connecting - timeout: %i ", wifiTimeoutShort);
    // Serial.print(".");
    wifiTimeoutShort--;
    if (wifiTimeoutShort == 0)
    {
      Serial.println("CheckWifi:\t still no Wifi connection - next try in " + String(wifiTimeoutLong) + " seconds (current retry count: " + String(reconnectsCnt) + ")");
      WiFi.disconnect();
      blinkCode = BLINK_WAITING_NEXT_TRY_DTU;
    }
    return false;
  }
  else if (WiFi.status() != WL_CONNECTED && wifi_connecting && wifiTimeoutShort == 0 && wifiTimeoutLong-- <= 0) // check during connecting wifi and decrease for short timeout
  {
    Serial.println(F("CheckWifi:\t state 'connecting' - wait time done"));
    wifiTimeoutShort = WIFI_RETRY_TIMEOUT_SECONDS;
    wifiTimeoutLong = WIFI_RETRY_TIME_SECONDS;
    wifi_connecting = false;
    return false;
  }
  else if (WiFi.status() == WL_CONNECTED && wifi_connecting) // is connected after connecting
  {
    Serial.println(F("CheckWifi:\t is now connected after state: 'connecting'"));
    wifi_connecting = false;
    wifiTimeoutShort = WIFI_RETRY_TIMEOUT_SECONDS;
    wifiTimeoutLong = WIFI_RETRY_TIME_SECONDS;
    startServices();
    return true;
  }
  else if (WiFi.status() == WL_CONNECTED) // everything fine & connected
  {
    // Serial.println(F("CheckWifi:\t Wifi connection: checked and fine ..."));
    blinkCode = BLINK_NORMAL_CONNECTION;
    return true;
  }
  else
  {
    return false;
  }
}

// scan network for first settings or change
boolean scanNetworksResult()
{
  int networksFound = WiFi.scanComplete();
  // print out Wi-Fi network scan result upon completion
  if (networksFound > 0)
  {
    Serial.print(F("WIFI_SCAN:\t done: "));
    Serial.println(String(networksFound) + " wifi's found");
    platformData.wifiNetworkCount = networksFound;
    platformData.wifiFoundNetworks = "[";
    for (int i = 0; i < networksFound; i++)
    {
      int wifiPercent = 2 * (WiFi.RSSI(i) + 100);
      if (wifiPercent > 100)
      {
        wifiPercent = 100;
      }
      // Serial.printf("%d: %s, Ch:%d (%ddBm, %d) %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), wifiPercent, WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "");
      platformData.wifiFoundNetworks = platformData.wifiFoundNetworks + "{\"name\":\"" + WiFi.SSID(i).c_str() + "\",\"wifi\":" + wifiPercent + ",\"rssi\":" + WiFi.RSSI(i) + ",\"chan\":" + WiFi.channel(i) + "}";
      if (i < networksFound - 1)
      {
        platformData.wifiFoundNetworks = platformData.wifiFoundNetworks + ",";
      }
    }
    platformData.wifiFoundNetworks = platformData.wifiFoundNetworks + "]";
    WiFi.scanDelete();
    espWebServer.setWifiScanIsRunning(false);
    return true;
  }
  else
  {
    // Serial.println(F("no networks found after scanning!"));
    return false;
  }
}

String getTimeStringByTimestamp(unsigned long timestamp, boolean isShort = false)
{
  UnixTime stamp(1);
  char buf[30];
  // prevent bad display of time
  if (timestamp > 4000000000)
  {
    sprintf(buf, "--.--.-- - --:--:--");
  }
  else
  {
    stamp.getDateTime(timestamp);
    // Serial.println("getTimeStringByTimestamp:\t\t got: " + String(timestamp) + " -> " + stamp.day + "." + stamp.month + "." + stamp.year + " - " + stamp.hour + ":" + stamp.minute + ":" + stamp.second);
    if (isShort)
      sprintf(buf, "%02i.%02i.%02i - %02i:%02i", stamp.day, stamp.month, stamp.year - 2000, stamp.hour, stamp.minute);
    else
      sprintf(buf, "%02i.%02i.%02i - %02i:%02i:%02i", stamp.day, stamp.month, stamp.year - 2000, stamp.hour, stamp.minute, stamp.second);
  }
  return String(buf);
}

// void getJsonDataAsync()
// {
//   Serial.println("getJsonDataAsync:\t[HTTP] getJSONdocument start getting data");
//   // String url = "/v1/marketdata?start=" + String(startTime) + "&end=" + String(endTime);
//   if (client.connected())
//   {
//     Serial.println("getJsonDataAsync:\tClient already connected");
//     return;
//   }
//   client.onConnect([](void *arg, AsyncClient *client)
//                    {
//                      Serial.println("getJsonDataAsync:\tConnected to server");
//                      // get the right timestamps for the request
//                      uint64_t startTime = static_cast<uint64_t>(timeClient.getEpochTime() - userConfig.timezoneOffest - 3600) * 1000;
//                      uint64_t endTime = startTime + 86400000 + 3600000; // 24 hours + 1 hour
//                      // String url = "/v1/marketdata?start=" + String(startTime) + "&end=" + String(endTime);
//                      String urlNew = "/static/webtest/price.json";
//                      Serial.print("getJsonDataAsync:\tRequesting URL: ");
//                      Serial.println("https://" + String(host) + ":" + String(httpsPort) + urlNew);
//                      String url = "/static/webtest/price.json";
//                      String request = String("GET ") + url + " HTTP/1.1\r\n" +
//                                       "Host: " + host + "\r\n" +
//                                       "Connection: close\r\n\r\n";
//                      client->write(request.c_str(), request.length()); },
//                    nullptr);
//
//   client.onData([](void *arg, AsyncClient *client, void *data, size_t len)
//                 {
//       Serial.println("getJsonDataAsync:\tData received");
//       String payload = String((char*)data).substring(0, len);
//       Serial.println("getJsonDataAsync:\tResponse:");
//       Serial.println(payload);
//
//       // Add debug statements to identify the error
//       Serial.print("getJsonDataAsync:\tData length: ");
//       Serial.println(len);
//       Serial.print("getJsonDataAsync:\tData content: ");
//       for (size_t i = 0; i < len; i++)
//       {
//           Serial.print((char)((char *)data)[i]);
//       }
//       Serial.println();
//
//       // Check if we have received the full headers
//       int headerEnd = response.indexOf("\r\n\r\n");
//       if (headerEnd != -1) {
//         String headers = payload.substring(0, headerEnd);
//         Serial.println("Headers:");
//         Serial.println(headers);
//
//         // Check HTTP status code
//         int statusCodeStart = payload.indexOf(" ") + 1;
//         int statusCodeEnd = payload.indexOf(" ", statusCodeStart);
//         String statusCode = payload.substring(statusCodeStart, statusCodeEnd);
//         Serial.print("HTTP Status Code: ");
//         Serial.println(statusCode);
//       }
//
//     // Parse JSON
//     JsonDocument doc;
//     DeserializationError error = deserializeJson(doc, payload);
//
//     if (error) {
//       Serial.print("getJsonDataAsync:\tdeserializeJson() failed: ");
//       Serial.println(error.c_str());
//       return;
//     }
//
//     // Assuming the JSON has a field "exampleField"
//     const char* exampleField = doc["url"];
//     Serial.print("getJsonDataAsync:\turl: ");
//     Serial.println(exampleField); }, nullptr);
//
//   client.onDisconnect([](void *arg, AsyncClient *client)
//                       { Serial.println("getJsonDataAsync:\tDisconnected from server"); }, nullptr);
//
//   client.onError([](void *arg, AsyncClient *client, int8_t error)
//                  {
//     Serial.print("getJsonDataAsync:\tConnection error: ");
//     Serial.println(error); }, nullptr);
//
//   client.connect(host, httpsPort);
// }

// JsonDocument getJSONdocument()
// {
//   JsonDocument doc;
//   if (WiFi.status() == WL_CONNECTED)
//   {
//     // #if defined(ESP8266)
//     std::unique_ptr<BearSSL::WiFiClientSecure> secClient(new BearSSL::WiFiClientSecure);
//     secClient->setInsecure();
//     // #elif defined(ESP32)
//     //     WiFiClientSecure secClient;
//     //     secClient.setInsecure();
//     // #endif
//     HTTPClient https;
//     // get the right tiemstamps for the request
//     Serial.println("GET_DATA:\t\t [HTTP] getJSONdocument start getting data");
//     uint64_t startTime = static_cast<uint64_t>(timeClient.getEpochTime() - userConfig.timezoneOffest - 3600) * 1000;
//     uint64_t endTime = startTime + 86400000 + 3600000; // 24 hours + 1 hour
//     String url = "";
//     url = "https://api.awattar.de/v1/marketdata?start=" + String(startTime) + "&end=" + String(endTime);
//     Serial.println("GET_DATA:\t\t [HTTP] getJSONdocument getting data: " + url);
//     Serial.println("GET_DATA:\t\t [HTTP] getJSONdocument start: " + getTimeStringByTimestamp(startTime / 1000) + " end: " + getTimeStringByTimestamp(endTime / 1000));
//     url = "https://192.168.1.30:8443/static/webtest/price.json";
//     https.setTimeout(2000);
//     // #if defined(ESP8266)
//     if (https.begin(*secClient, url))
//     {
//       // #elif defined(ESP32)
//       //     if (https.begin(secClient, url))
//       //     {
//       // #endif
//       Serial.println("GET_DATA:\t\t [HTTP] connected to " + url);
//       https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS); // allow redirects
//       int httpCode = https.GET();
//       if (httpCode == HTTP_CODE_OK)
//       {
//         // payload = http.getString();
//         String payload = https.getString();
//         Serial.println(">>>>>> HTTPS get data: " + payload);
//         DeserializationError error = deserializeJson(doc, payload);
//         // Test if parsing succeeds.
//         if (error)
//         {
//           Serial.print(F("deserializeJson() failed: "));
//           Serial.println(error.f_str());
//         }
//         else
//         {
//           Serial.println("GET_DATA:\t\t [HTTP] getJSONdocument data received");
//         }
//       }
//       else
//       {
//         // Serial.println("GET_DATA:\t\t [HTTP] getJSONdocument HTTP error: " + http.errorToString(httpCode));
//         Serial.println("GET_DATA:\t\t [HTTP] getJSONdocument HTTP error: " + https.errorToString(httpCode));
//       }
//       // #if defined(ESP8266)
//       secClient->stop();
//       // #elif defined(ESP32)
//       //       secClient.stop();
//       // #endif
//       https.end();
//       return doc;
//     }
//     else
//     {
//       Serial.println("GET_DATA:\t\t [HTTP] getJSONdocument Unable to connect " + url);
//       return doc;
//     }
//   }
//   return doc;
// }

JsonDocument getJSONdocumentInsecure()
{
  WiFiClient client;
  HTTPClient http;
  JsonDocument doc;
  if (WiFi.status() == WL_CONNECTED)
  {

    // get the right tiemstamps for the request
    Serial.println("GET_DATA:\t\t [HTTP] getJSONdocumentInsecure start getting data");
    uint64_t startTime = static_cast<uint64_t>(timeClient.getEpochTime() - userConfig.timezoneOffest - 3600) * 1000;
    uint64_t endTime = startTime + 86400000 + 3600000; // 24 hours + 1 hour
    String url = "";
    url = "http://api.awattar.de/v1/marketdata?start=" + String(startTime) + "&end=" + String(endTime);
    Serial.println("GET_DATA:\t\t [HTTP] getJSONdocumentInsecure getting data: " + url);
    Serial.println("GET_DATA:\t\t [HTTP] getJSONdocumentInsecure start: " + getTimeStringByTimestamp(startTime / 1000) + " end: " + getTimeStringByTimestamp(endTime / 1000));
    // url = "http://192.168.1.30:8080/static/webtest/price.json";
    http.setTimeout(2000); // prevent blocking of progam
    if (http.begin(client, url))
    {
      Serial.println("GET_DATA:\t\t [HTTP] connected to " + url);
      int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK)
      {
        String payload = http.getString();
        // Serial.println(">>>>>> HTTPS get data: " + payload);
        DeserializationError error = deserializeJson(doc, payload);
        // Test if parsing succeeds.
        if (error)
        {
          Serial.print(F("deserializeJson() failed: "));
          Serial.println(error.f_str());
        }
      }
      else
      {
        Serial.println("GET_DATA:\t\t [HTTP] getJSONdocumentInsecure HTTP error: " + http.errorToString(httpCode));
      }
      http.end();
    }
    else
    {
      Serial.println("GET_DATA:\t\t [HTTP] getJSONdocumentInsecure Unable to connect " + url);
    }
  }
  return doc;
}

float getCurrentPriceList()
{
  JsonDocument doc = getJSONdocumentInsecure();

  float nextHourPrice = 0.0;
  JsonArray data = doc["data"];
  if (data.size() > 0)
  {
    float maxPrice = 0;
    for (size_t i = 0; i < data.size(); i++)
    {
      JsonObject nextHourData = data[i];
      float price = getCostForEpexPrice(static_cast<float>(nextHourData["marketprice"]) / 1000.0f);
      if (price > maxPrice)
        maxPrice = price;
    }
    // clear array before filling
    for (uint8_t i = 0; i < 24; i++)
    {
      platformData.pricePerKWh[i] = maxPrice + (maxPrice * 0.05); // maxprice + 5% as default
    }
    for (size_t i = 0; i < data.size(); i++)
    {
      JsonObject nextHourData = data[i];
      nextHourPrice = static_cast<float>(nextHourData["marketprice"]) / 1000.0f;
      uint64_t start = (static_cast<uint64_t>(nextHourData["start_timestamp"])) / 1000 + 3600;
      uint64_t end = (static_cast<uint64_t>(nextHourData["end_timestamp"])) / 1000 + 3600;
      platformData.pricePerKWh[i] = getCostForEpexPrice(nextHourPrice);
      Serial.println("GET_DATA: (" + String(i) + ")\t brutto: " + String(nextHourPrice, 4) + "€/kWh (netto: " + String(platformData.pricePerKWh[i], 4) + "€/kWh) - start: " + getTimeStringByTimestamp(start) + " end: " + getTimeStringByTimestamp(end));
    }
  }
  else
  {
    Serial.println("GET_DATA:\t\t [HTTP] getCurrentPriceList no data found in JSON");
  }
  return nextHourPrice;
}

void calculateCurrentCost()
{
  float pricePerKWhNow = platformData.pricePerKWh[0];
  Serial.println("calculateCurrentCost\t     price per kWh now: " + String(pricePerKWhNow, 4) + " €/kWh");

  platformData.priceSumNow = 0;
  for (uint8_t i = 0; i < platformData.tgtDurationInHours; i++)
  {
    platformData.priceSumNow = platformData.priceSumNow + platformData.pricePerKWh[i];
  }
  platformData.priceSumNow = tgtConsupmtionFactor * platformData.priceSumNow;
  Serial.println("calculateCurrentCost\tsum price for duration: " + String(platformData.priceSumNow, 4) + " €/kWh");
}

void calculateBestCost()
{
  // Variables to track the lowest cost phase
  float minCost = 1000000;
  platformData.currentHour = timeClient.getHours();

  Serial.println("TEST --- currentHour: " + String(platformData.currentHour) + " h --- tgtDurationInHours: " + String(platformData.tgtDurationInHours) + " h");

  if (maxDelayHours > 23)
    maxDelayHours = 23;

  // Iterate through the array to find the phase with the lowest total cost
  for (uint8_t i = 0; i <= maxDelayHours; i++)
  {
    float totalCost = 0;
    for (uint8_t j = 0; j < platformData.tgtDurationInHours; j++)
    {
      totalCost = totalCost + (platformData.pricePerKWh[(i + j) % 24]);
    }
    if (totalCost < minCost)
    {
      minCost = totalCost;
      platformData.minStartHour = i;
    }

    // debug
    uint8_t startHour = i + platformData.currentHour;
    if (startHour > 24)
      startHour = startHour - 24;

    // Serial.println("TEST (" + String(i) + ")\ttotalCost: " + String(totalCost, 4) + " € --- hour: " + String(startHour) + " h" + " -\t cost at hour: " + String(platformData.pricePerKWh[i], 5) + " €/kWh");
  }
  // get the lowest cost for the whole duration
  platformData.priceSumSave = tgtConsupmtionFactor * minCost;

  platformData.minStartHour = platformData.minStartHour + platformData.currentHour;
  if (platformData.minStartHour > 24)
  {
    platformData.minStartHour = platformData.minStartHour - 24;
  }
  Serial.println("TEST --- minCost: " + String(minCost, 4) + " € --- minStartHour: " + String(platformData.minStartHour) + " h");

  if (platformData.minStartHour < platformData.currentHour)
  {
    platformData.tgtDelayHours = 24 - platformData.currentHour + platformData.minStartHour;
  }
  else
    platformData.tgtDelayHours = platformData.minStartHour - platformData.currentHour;

  Serial.println("TEST --- price sum now: " + String(platformData.priceSumNow, 4) + " € --- price sum save: " + String(platformData.priceSumSave, 4) + " €");
}

float getCostForEpexPrice(float rawPricePerKWH)
{
  float fixedTaxPricePerKWh = 0.0205 + 0.0157;
  float fixedPricePerKWh = 0.13135;
  float taxFixedPrecedPerKWH = 16;
  return (rawPricePerKWH + fixedTaxPricePerKWh + fixedPricePerKWh + ((rawPricePerKWH + fixedPricePerKWh) * taxFixedPrecedPerKWH / 100));
}

void calculateCost()
{
  getCurrentPriceList();
  calculateCurrentCost();
  calculateBestCost();
  platformData.lastCostDataUpdate = timeClient.getEpochTime() - 3600;
}

// ****

void setup()
{
// switch off SCK LED
// pinMode(14, OUTPUT);
// digitalWrite(14, LOW);

// shortend chip id for ESP32  based on MAC - to be compliant with ESP8266 ESP.getChipId() output
#if defined(ESP32)
  platformData.chipID = 0;
  for (int i = 0; i < 17; i = i + 8)
  {
    platformData.chipID |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
  }
  platformData.espUniqueName = String(AP_NAME_START) + "_" + platformData.chipID;
#endif

  // initialize digital pin LED_BLINK as an output.
  pinMode(LED_BLINK, OUTPUT);
  digitalWrite(LED_BLINK, LED_BLINK_OFF); // turn the LED off by making the voltage LOW

  Serial.begin(115200);
  Serial.print(F("\n\nBooting - with firmware version "));
  Serial.println(platformData.fwVersion);
  Serial.println(F("------------------------------------------------------------------"));

  if (!configManager.begin())
  {
    Serial.println(F("Failed to initialize UserConfigManager"));
    return;
  }

  if (configManager.loadConfig(userConfig))
    configManager.printConfigdata();
  else
    Serial.println(F("Failed to load user config"));
  // ------- user config loaded --------------------------------------------

  // init display according to userConfig
  if (userConfig.displayConnected == 0)
  {
    displayOLED.setup();
    // delete &displayTFT;
  }
  else if (userConfig.displayConnected == 1)
  {
    displayTFT.setup();
    // delete &displayOLED;
  }

  if (userConfig.wifiAPstart)
  {
    Serial.println(F("\n+++ device in 'first start' mode - have to be initialized over own served wifi +++\n"));

    WiFi.scanNetworks();
    scanNetworksResult();

    // Connect to Wi-Fi as AP
    WiFi.mode(WIFI_AP);
    WiFi.softAP(platformData.espUniqueName);
    Serial.println("\n +++ serving access point with SSID: '" + platformData.espUniqueName + "' +++\n");

    // IP Address of the ESP8266 on the AP network
    IPAddress apIP = WiFi.softAPIP();
    Serial.print(F("AP IP address: "));
    Serial.println(apIP);

    MDNS.begin("energyCostSaver");
    MDNS.addService("http", "tcp", 80);
    Serial.println(F("Ready! Open http://energyCostSaver.local in your browser"));

    // display - change every reboot in first start mode
    if (userConfig.displayConnected == 0)
    {
      displayOLED.drawFactoryMode(String(platformData.fwVersion), platformData.espUniqueName, apIP.toString());
      userConfig.displayConnected = 1;
    }
    else if (userConfig.displayConnected == 1)
    {
      displayTFT.drawFactoryMode(String(platformData.fwVersion), platformData.espUniqueName, apIP.toString());
      userConfig.displayConnected = 0;
    }
    configManager.saveConfig(userConfig);

    espWebServer.start();
  }
  else
  {
    WiFi.mode(WIFI_STA);
  }

  // Interval in microsecs
  if (ITimer.setInterval(TIMER_INTERVAL_MS * 1000, timer1000MilliSeconds))
  {
    unsigned long lastMillis = millis();
    Serial.print(F("ISR_TIMER:\t starting  ITimer OK, millis() = "));
    Serial.println(lastMillis);
  }
  else
    Serial.println(F("Can't set ITimer correctly. Select another freq. or interval"));
  // delay for startup background tasks in ESP
  delay(2000);
}

// after startup or reconnect with wifi
void startServices()
{
  if (WiFi.waitForConnectResult() == WL_CONNECTED)
  {
    Serial.print(F("WIFIclient:\t connected! IP address: "));
    platformData.dtuGatewayIP = WiFi.localIP();
    Serial.println((platformData.dtuGatewayIP).toString());
    Serial.print(F("WIFIclient:\t IP address of gateway: "));
    Serial.println(WiFi.gatewayIP());

    MDNS.begin(platformData.espUniqueName);
    MDNS.addService("http", "tcp", 80);
    Serial.println("MDNS:\t\t ready! Open http://" + platformData.espUniqueName + ".local in your browser");

    // ntp time - offset in summertime 7200 else 3600
    timeClient.begin();
    timeClient.setTimeOffset(userConfig.timezoneOffest);
    // get first time
    timeClient.update();
    platformData.espStarttime = timeClient.getEpochTime();
    Serial.print(F("NTPclient:\t got time from time server: "));
    Serial.println(String(platformData.espStarttime));

    espWebServer.start();

    // first run
    calculateCost();
  }
  else
  {
    Serial.println(F("WIFIclient:\t connection failed"));
  }
}

uint16_t ledCycle = 0;
void blinkCodeTask()
{
  int8_t ledOffCount = 2;
  int8_t ledOffReset = 11;

  ledCycle++;
  if (blinkCode == BLINK_NORMAL_CONNECTION) // Blip every 5 sec
  {
    ledOffCount = 2;  // 200 ms
    ledOffReset = 50; // 5000 ms
  }
  else if (blinkCode == BLINK_WAITING_NEXT_TRY_DTU) // 0,5 Hz
  {
    ledOffCount = 10; // 1000 ms
    ledOffReset = 20; // 2000 ms
  }
  else if (blinkCode == BLINK_WIFI_OFF) // long Blip every 5 sec
  {
    ledOffCount = 5;  // 500 ms
    ledOffReset = 50; // 5000 ms
  }
  else if (blinkCode == BLINK_TRY_CONNECT_DTU) // 5 Hz
  {
    ledOffCount = 2; // 200 ms
    ledOffReset = 2; // 200 ms
  }
  else if (blinkCode == BLINK_PAUSE_CLOUD_UPDATE) // Blip every 2 sec
  {
    ledOffCount = 2;  // 200 ms
    ledOffReset = 21; // 2000 ms
  }

  if (ledCycle == 1)
  {
    digitalWrite(LED_BLINK, LED_BLINK_ON); // turn the LED on
  }
  else if (ledCycle == ledOffCount)
  {
    digitalWrite(LED_BLINK, LED_BLINK_OFF); // turn the LED off
  }
  if (ledCycle >= ledOffReset)
  {
    ledCycle = 0;
  }
}

// serial comm
String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++)
  {
    if (data.charAt(i) == separator || i == maxIndex)
    {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void serialInputTask()
{
  // Check to see if anything is available in the serial receive buffer
  if (Serial.available() > 0)
  {
    static char message[20];
    static unsigned int message_pos = 0;
    char inByte = Serial.read();
    if (inByte != '\n' && (message_pos < 20 - 1))
    {
      message[message_pos] = inByte;
      message_pos++;
    }
    else // Full message received...
    {
      // Add null character to string
      message[message_pos] = '\0';
      // Print the message (or do other things)
      Serial.print(F("GotCmd: "));
      Serial.println(message);
      getSerialCommand(getValue(message, ' ', 0), getValue(message, ' ', 1));
      // Reset for the next message
      message_pos = 0;
    }
  }
}

void getSerialCommand(String cmd, String value)
{
  int val = value.toInt();
  Serial.print(F("CmdOut: "));
  if (cmd == "getDataAuto")
  {
    Serial.print(F("'getDataAuto' to "));
    if (val == 1)
    {
      globalControls.getDataAuto = true;
      Serial.print(F(" 'ON' "));
    }
    else
    {
      globalControls.getDataAuto = false;
      Serial.print(F(" 'OFF' "));
    }
  }
  else if (cmd == "getDataOnce")
  {
    Serial.print(F("'getDataOnce' to "));
    if (val == 1)
    {
      globalControls.getDataOnce = true;
      Serial.print(F(" 'ON' "));
    }
    else
    {
      globalControls.getDataOnce = false;
      Serial.print(F(" 'OFF' "));
    }
  }
  else if (cmd == "dataFormatJSON")
  {
    Serial.print(F("'dataFormatJSON' to "));
    if (val == 1)
    {
      globalControls.dataFormatJSON = true;
      Serial.print(F(" 'ON' "));
    }
    else
    {
      globalControls.dataFormatJSON = false;
      Serial.print(F(" 'OFF' "));
    }
  }
  else if (cmd == "setWifi")
  {
    Serial.print(F("'setWifi' to "));
    if (val == 1)
    {
      globalControls.wifiSwitch = true;
      Serial.print(F(" 'ON' "));
    }
    else
    {
      globalControls.wifiSwitch = false;
      blinkCode = BLINK_WIFI_OFF;
      Serial.print(F(" 'OFF' "));
    }
  }
  else if (cmd == "setInterval")
  {
    userConfig.espUpdateTime = long(val);
    Serial.print("'setInterval' to " + String(userConfig.espUpdateTime));
  }
  else if (cmd == "getInterval")
  {
    Serial.print("'getInterval' => " + String(userConfig.espUpdateTime));
  }
  else if (cmd == "resetToFactory")
  {
    Serial.print(F("'resetToFactory' to "));
    if (val == 1)
    {
      configManager.resetConfig();
      Serial.print(F(" reinitialize UserConfig data and reboot ... "));
      ESP.restart();
    }
  }
  else if (cmd == "rebootDevice")
  {
    Serial.print(F(" rebootDevice "));
    if (val == 1)
    {
      Serial.print(F(" ... rebooting ... "));
      ESP.restart();
    }
  }
  else if (cmd == "selectDisplay")
  {
    Serial.print(F(" selected Display"));
    if (val == 0)
    {
      userConfig.displayConnected = 0;
      Serial.print(F(" OLED"));
    }
    else if (val == 1)
    {
      userConfig.displayConnected = 1;
      Serial.print(F(" ROUND TFT 1.28"));
    }
    configManager.saveConfig(userConfig);
    configManager.printConfigdata();
    Serial.println(F("restart the device to make the changes take effect"));
    ESP.restart();
  }
  else
  {
    Serial.print(F("Cmd not recognized\n"));
  }
  Serial.print(F("\n"));
}

// main

// get precise localtime - increment
#if defined(ESP8266)
void IRAM_ATTR timer1000MilliSeconds()
{
#elif defined(ESP32)
bool IRAM_ATTR timer1000MilliSeconds(void *timerNo)
{
#endif
  // localtime counter - increase every second
  platformData.currentTimestamp++;
#if defined(ESP32)
  return true;
#endif
}

void loop()
{
  unsigned long currentMillis = millis();
  // skip all tasks if update is running
  if (updateInfo.updateState != UPDATE_STATE_IDLE)
  {
    if (updateInfo.updateState == UPDATE_STATE_PREPARE)
    {
      if (userConfig.displayConnected == 0)
        displayOLED.drawUpdateMode("update running ...");
      else if (userConfig.displayConnected == 1)
        displayTFT.drawUpdateMode("update running ...");
      updateInfo.updateState = UPDATE_STATE_INSTALLING;
    }
    if (updateInfo.updateState == UPDATE_STATE_DONE)
    {
      if (userConfig.displayConnected == 0)
        displayOLED.drawUpdateMode("update done", "rebooting ...");
      else if (userConfig.displayConnected == 1)
        displayTFT.drawUpdateMode("update done", "rebooting ...");
      updateInfo.updateState = UPDATE_STATE_RESTART;
    }
    return;
  }
  // check for wifi networks scan results
  scanNetworksResult();

#if defined(ESP8266)
  // serving domain name
  MDNS.update();
#endif

  // 50ms task
  if (currentMillis - previousMillis50ms >= interval50ms)
  {
    previousMillis50ms = currentMillis;
    // -------->
    if (!userConfig.wifiAPstart)
    {
      // display tasks every 50ms = 20Hz
      if (userConfig.displayConnected == 0)
        // displayOLED.renderScreen(getTimeStringByTimestamp(timeClient.getEpochTime() - 3600), String("FW: " + platformData.fwVersion), platformData.tgtDelayHours, platformData.pricePerKWhNow, platformData.priceSumSave);
        displayOLED.renderScreen(getTimeStringByTimestamp(timeClient.getEpochTime() - 3600), "Stand: " + getTimeStringByTimestamp(platformData.lastCostDataUpdate, true), platformData.tgtDelayHours, platformData.priceSumNow, platformData.priceSumSave);
      else if (userConfig.displayConnected == 1)
        displayTFT.renderScreen(timeClient.getFormattedTime(), String(platformData.fwVersion));
    }
  }

  // 100ms task
  if (currentMillis - previousMillis100ms >= interval100ms)
  {
    previousMillis100ms = currentMillis;
    // -------->
    blinkCodeTask();
    serialInputTask();

    platformData.currentNTPtime = timeClient.getEpochTime();
    platformData.currentNTPtimeFormatted = timeClient.getFormattedTime();
  }

  // CHANGE to precise 1 second timer increment
  currentMillis = platformData.currentTimestamp;

  // short task
  if (currentMillis - previousMillisShort >= intervalShort)
  {
    // Serial.printf("\n>>>>> %02is task - state --> ", int(intervalShort));
    // Serial.print("local: " + getTimeStringByTimestamp(platformData.currentTimestamp));
    // Serial.print(" --- NTP: " + timeClient.getFormattedTime() + " --- currentMillis " + String(currentMillis) + " --- ");
    previousMillisShort = currentMillis;
    // Serial.print(F("free mem: "));
    // Serial.print(ESP.getFreeHeap());
    // Serial.print(F(" - heap fragm: "));
    // Serial.print(ESP.getHeapFragmentation());
    // Serial.print(F(" - max free block size: "));
    // Serial.print(ESP.getMaxFreeBlockSize());
    // Serial.print(F(" - free cont stack: "));
    // Serial.print(ESP.getFreeContStack());
    // Serial.print(F(" \n"));

    // -------->

    if (!userConfig.wifiAPstart)
    {
      if (globalControls.wifiSwitch)
        checkWifiTask();
      else
      {
        // stopping connection to DTU before go wifi offline
        WiFi.disconnect();
      }
    }
  }

  // 5s task
  if (currentMillis - previousMillis5000ms >= interval5000ms)
  {
    Serial.printf(">>>>> %02is task - state --> ", int(interval5000ms));
    Serial.print("local: " + String(platformData.currentTimestamp));
    Serial.println(" --- NTP: " + timeClient.getFormattedTime());

    // Serial.print(" --- currentMillis " + String(currentMillis) + " --- ");
    previousMillis5000ms = currentMillis;
    // -------->
    // -----------------------------------------
    if (WiFi.status() == WL_CONNECTED)
    {
      // get current RSSI to AP
      int wifiPercent = 2 * (WiFi.RSSI() + 100);
      if (wifiPercent > 100)
        wifiPercent = 100;
      platformData.wifi_rssi_gateway = wifiPercent;
      // Serial.print(" --- RSSI to AP: '" + String(WiFi.SSID()) + "': " + String(platformData.wifi_rssi_gateway) + " %");
    }
  }

  // mid task
  if (currentMillis - platformData.costDataUpdateCycleInSeconds >= userConfig.espUpdateTime)
  {
    Serial.printf(">>>>> %02is task - state --> ", int(userConfig.espUpdateTime));
    Serial.println(" --- NTP: " + timeClient.getFormattedTime() + "\n");

    platformData.costDataUpdateCycleInSeconds = currentMillis;
    // -------->
    calculateCost();
  }

  // long task
  if (currentMillis - previousMillisLong >= intervalLong)
  {
    // Serial.printf("\n>>>>> %02is task - state --> ", int(interval5000ms));
    // Serial.print("local: " + getTimeStringByTimestamp(platformData.currentTimestamp));
    // Serial.print(" --- NTP: " + timeClient.getFormattedTime() + " --- currentMillis " + String(currentMillis) + " --- ");

    previousMillisLong = currentMillis;
    // -------->
    if (WiFi.status() == WL_CONNECTED)
    {
      timeClient.update();
    }
  }
}
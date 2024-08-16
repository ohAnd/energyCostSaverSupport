#include <UnixTime.h>
#if defined(ESP8266)
#include <ESP8266_ISR_Timer.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <ESPAsyncTCP.h>
#elif defined(ESP32)
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
const long intervalShort = 1 * 1000;   // interval (milliseconds)
const long interval5000ms = 5 * 1000;  // interval (milliseconds)
const long intervalLong = 60 * 1000;   // interval (milliseconds)
unsigned long previousMillis50ms = 0;
unsigned long previousMillis100ms = 0;
unsigned long previousMillisShort = 0;
unsigned long previousMillis5000ms = 0;
unsigned long previousMillisSpecial = 0;
unsigned long previousMillisLong = 0;

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

// user config
uint8_t tgtConsupmtionFactor = 1.2; // average consumption per hour e.g. washing machine needs 3.6 kWh in 3 hours -> factor = 1.2

boolean checkWifiTask()
{
  if (WiFi.status() != WL_CONNECTED && !wifi_connecting) // start connecting wifi
  {
    // reconnect counter - and reset to default
    reconnects[reconnectsCnt++] = timeClient.getEpochTime();
    if (reconnectsCnt >= 25)
    {
      reconnectsCnt = 0;
      Serial.println(F("CheckWifi:\t  no Wifi connection after 25 tries!"));
      // after 20 reconnects inner 7 min - write defaults
      if ((timeClient.getEpochTime() - reconnects[0]) < (WIFI_RETRY_TIME_SECONDS * 1000)) //
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
  // prevent wrong display of time
  if (timestamp > 4000000000 || timestamp < 1704067200)
  {
    sprintf(buf, "--.--.-- - --:--:--");
  }
  else
  {
    stamp.getDateTime(timestamp - 3600);
    // Serial.println("getTimeStringByTimestamp:\t\t got: " + String(timestamp) + " -> " + stamp.day + "." + stamp.month + "." + stamp.year + " - " + stamp.hour + ":" + stamp.minute + ":" + stamp.second);
    if (isShort)
      sprintf(buf, "%02i.%02i.%02i - %02i:%02i", stamp.day, stamp.month, stamp.year - 2000, stamp.hour, stamp.minute);
    else
      sprintf(buf, "%02i.%02i.%02i - %02i:%02i:%02i", stamp.day, stamp.month, stamp.year - 2000, stamp.hour, stamp.minute, stamp.second);
  }
  return String(buf);
}

JsonDocument getEPEXjson()
{
  WiFiClient client;
  HTTPClient http;
  JsonDocument doc;
  if (WiFi.status() == WL_CONNECTED)
  {

    // get the right tiemstamps for the request
    Serial.println("getEPEXjson:\t start getting data");
     // using local time - offset - 1 h to get the prices also for the current hour
    uint64_t startTime = static_cast<uint64_t>(timeClient.getEpochTime() - userConfig.timezoneOffest - 3600) * 1000;
    uint64_t endTime = startTime + 86400000 + 3600000; // 24 hours + 1 hour
    String url = "";
    url = "http://api.awattar.de/v1/marketdata?start=" + String(startTime) + "&end=" + String(endTime);
    Serial.println("getEPEXjson:\t using url: " + url);
    Serial.println("getEPEXjson:\t request data from: " + getTimeStringByTimestamp(startTime + userConfig.timezoneOffest / 1000) + " to: " + getTimeStringByTimestamp(endTime + userConfig.timezoneOffest / 1000));
    // url = "http://192.168.1.30:8080/static/webtest/price.json";
    http.setTimeout(2000); // prevent blocking of progam
    if (http.begin(client, url))
    {
      Serial.println("getEPEXjson:\t [HTTP] connected to " + url);
      int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK)
      {
        String payload = http.getString();
        // Serial.println(">>>>>> HTTPS get data: " + payload);
        DeserializationError error = deserializeJson(doc, payload);
        // Test if parsing succeeds.
        if (error)
        {
          Serial.print(F("getEPEXjson:\t deserializeJson() failed: "));
          Serial.println(error.f_str());
        }
      }
      else
      {
        Serial.println("getEPEXjson:\t [HTTP] error: " + http.errorToString(httpCode));
      }
      http.end();
    }
    else
    {
      Serial.println("getEPEXjson:\t [HTTP] Unable to connect to " + url);
    }
  }
  return doc;
}

float getCurrentPriceList()
{
  JsonDocument doc = getEPEXjson();

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
      uint64_t start = (static_cast<uint64_t>(nextHourData["start_timestamp"])) / 1000 + userConfig.timezoneOffest;
      uint64_t end = (static_cast<uint64_t>(nextHourData["end_timestamp"])) / 1000 + userConfig.timezoneOffest;
      platformData.pricePerKWh[i] = getCostForEpexPrice(nextHourPrice);
      Serial.println("getCurrentPriceList:\t(" + String(i) + ")\t brutto: " + String(nextHourPrice, 4) + "€/kWh (netto: " + String(platformData.pricePerKWh[i], 4) + "€/kWh) - start: " + getTimeStringByTimestamp(start) + " end: " + getTimeStringByTimestamp(end));
    }
    platformData.pricePerKWhLast = data.size() - 1;
  }
  else
  {
    Serial.println("getCurrentPriceList:\t no data found in JSON");
  }
  return nextHourPrice;
}

void calculateCurrentCost()
{
  float pricePerKWhNow = platformData.pricePerKWh[0];
  Serial.println("calculateCurrentCost\t price per kWh now: " + String(pricePerKWhNow, 4) + " €/kWh");

  platformData.energyCostNow = 0;
  for (uint8_t i = 0; i < userConfig.tgtDurationInHours; i++)
  {
    platformData.energyCostNow = platformData.energyCostNow + platformData.pricePerKWh[i];
  }
  platformData.energyCostNow = tgtConsupmtionFactor * platformData.energyCostNow;
  Serial.println("calculateCurrentCost\t sum price for duration: " + String(platformData.energyCostNow, 4) + " €/kWh");
}

void calculateBestCost()
{
  // Variables to track the lowest cost phase
  float minCost = 1000000;
  platformData.currentHour = timeClient.getHours();

  Serial.println("calculateBestCost\t currentHour: " + String(platformData.currentHour) + " h --- tgtDurationInHours: " + String(userConfig.tgtDurationInHours) + " h");

  if (userConfig.maxWaitTime > 23)
    userConfig.maxWaitTime = 23;

  // Iterate through the array to find the phase with the lowest total cost
  for (uint8_t i = 0; i <= (userConfig.maxWaitTime - userConfig.tgtDurationInHours); i++)
  {
    float totalCost = 0;
    for (uint8_t j = 0; j < userConfig.tgtDurationInHours; j++)
    {
      totalCost = totalCost + (platformData.pricePerKWh[(i + j) % 24]);
    }
    if (totalCost < minCost)
    {
      minCost = totalCost;
      platformData.tgtStartHour = i;
    }

    // debug
    uint8_t startHour = i + platformData.currentHour;
    if (startHour > 24)
      startHour = startHour - 24;

    // Serial.println("calculateBestCost\t (" + String(i) + ")\ttotalCost: " + String(totalCost, 4) + " € --- hour: " + String(startHour) + " h" + " -\t cost at hour: " + String(platformData.pricePerKWh[i], 5) + " €/kWh");
  }
  // get the lowest cost for the whole duration
  platformData.energyCostSave = tgtConsupmtionFactor * minCost;

  platformData.tgtStartHour = platformData.tgtStartHour + platformData.currentHour;
  if (platformData.tgtStartHour >= 24)
  {
    platformData.tgtStartHour = platformData.tgtStartHour - 24;
  }
  Serial.println("calculateBestCost\t minCost: " + String(minCost, 4) + " € --- tgtStartHour: " + String(platformData.tgtStartHour) + " h");

  if (platformData.tgtStartHour < platformData.currentHour)
  {
    platformData.tgtDelayHours = 24 - platformData.currentHour + platformData.tgtStartHour;
  }
  else
    platformData.tgtDelayHours = platformData.tgtStartHour - platformData.currentHour;

  Serial.println("calculateBestCost\t price sum now: " + String(platformData.energyCostNow, 4) + " € --- price sum save: " + String(platformData.energyCostSave, 4) + " €");
}

float getCostForEpexPrice(float rawPricePerKWH)
{
  return (rawPricePerKWH + userConfig.fixedTaxPricePerKWh + userConfig.fixedPricePerKWh + ((rawPricePerKWH + userConfig.fixedPricePerKWh) * userConfig.taxVarPricePerKWH / 100));
}

void calculateCost()
{
  getCurrentPriceList();
  calculateCurrentCost();
  calculateBestCost();
  platformData.lastCostDataUpdateGMT = timeClient.getEpochTime() - userConfig.timezoneOffest;
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
  // if (userConfig.displayConnected == 0)
  // {
  displayOLED.setup();
  //   // delete &displayTFT;
  // }
  // else if (userConfig.displayConnected == 1)
  // {
  //   displayTFT.setup();
  //   // delete &displayOLED;
  // }

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
    // if (userConfig.displayConnected == 0)
    // {
    displayOLED.drawFactoryMode(String(platformData.fwVersion), platformData.espUniqueName, apIP.toString());
    userConfig.displayConnected = 1;
    // }
    // else if (userConfig.displayConnected == 1)
    // {
    //   displayTFT.drawFactoryMode(String(platformData.fwVersion), platformData.espUniqueName, apIP.toString());
    //   userConfig.displayConnected = 0;
    // }
    configManager.saveConfig(userConfig);

    espWebServer.start();
  }
  else
  {
    WiFi.mode(WIFI_STA);
  }

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
    platformData.espStarttimeGMT = timeClient.getEpochTime() - userConfig.timezoneOffest;
    Serial.print(F("NTPclient:\t got time from time server: "));
    Serial.println(String(platformData.espStarttimeGMT + userConfig.timezoneOffest) + " -> " + getTimeStringByTimestamp(platformData.espStarttimeGMT));

    espWebServer.start();

    // wait for timeClient
    delay(1);

    // first run and reset for next run
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

  if (cmd == "setWifi")
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
  // else if (cmd == "selectDisplay")
  // {
  //   Serial.print(F(" selected Display"));
  //   if (val == 0)
  //   {
  //     userConfig.displayConnected = 0;
  //     Serial.print(F(" OLED"));
  //   }
  //   else if (val == 1)
  //   {
  //     userConfig.displayConnected = 1;
  //     Serial.print(F(" ROUND TFT 1.28"));
  //   }
  //   configManager.saveConfig(userConfig);
  //   configManager.printConfigdata();
  //   Serial.println(F("restart the device to make the changes take effect"));
  //   ESP.restart();
  // }
  else
  {
    Serial.print(F("Cmd not recognized\n"));
  }
  Serial.print(F("\n"));
}

// main

void loop()
{
  unsigned long currentMillis = millis();
  // skip all tasks if update is running
  if (updateInfo.updateState != UPDATE_STATE_IDLE)
  {
    if (updateInfo.updateState == UPDATE_STATE_PREPARE)
    {
      // if (userConfig.displayConnected == 0)
      displayOLED.drawUpdateMode("update running ...");
      // else if (userConfig.displayConnected == 1)
      //   displayTFT.drawUpdateMode("update running ...");
      updateInfo.updateState = UPDATE_STATE_INSTALLING;
    }
    if (updateInfo.updateState == UPDATE_STATE_DONE)
    {
      // if (userConfig.displayConnected == 0)
      displayOLED.drawUpdateMode("update done", "rebooting ...");
      // else if (userConfig.displayConnected == 1)
      //   displayTFT.drawUpdateMode("update done", "rebooting ...");
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
        displayOLED.renderScreen(getTimeStringByTimestamp(timeClient.getEpochTime()), "Stand: " + getTimeStringByTimestamp(platformData.lastCostDataUpdateGMT  + userConfig.timezoneOffest, true), platformData.tgtDelayHours, platformData.energyCostNow, platformData.energyCostSave);
    }
  }

  // 100ms task
  if (currentMillis - previousMillis100ms >= interval100ms)
  {
    previousMillis100ms = currentMillis;
    // -------->
    blinkCodeTask();
    serialInputTask();

    platformData.currentNTPtimeGMT = timeClient.getEpochTime() - userConfig.timezoneOffest;
  }

  // short task
  if (currentMillis - previousMillisShort >= intervalShort)
  {
    // Serial.printf("\n>>>>> %02is task - state --> ", int(intervalShort/1000));
    // Serial.print(" --- NTP: " + timeClient.getFormattedTime() + " --- currentMillis " + String(currentMillis) + " --- ");
    previousMillisShort = currentMillis;
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
    if(platformData.userSettingsChanged) {
      calculateCost();
      platformData.userSettingsChanged = false;
    }
  }

  // 5s task
  if (currentMillis - previousMillis5000ms >= interval5000ms)
  {
    Serial.printf(">>>>> %02is task - ", int(interval5000ms/1000));
    Serial.println("NTP: " + timeClient.getFormattedTime());

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
  if (currentMillis - previousMillisSpecial >= (userConfig.espUpdateTime * 1000))
  {
    Serial.printf(">>>>> %02is task - ", int(userConfig.espUpdateTime));
    Serial.println("NTP: " + timeClient.getFormattedTime() + "\n");

    previousMillisSpecial = currentMillis;
    // -------->
    calculateCost();
  }

  // long task
  if (currentMillis - previousMillisLong >= intervalLong)
  {
    // Serial.printf("\n>>>>> %02is task - state --> ", int(intervalLong/1000));
    // Serial.print(" --- NTP: " + timeClient.getFormattedTime() + " --- currentMillis " + String(currentMillis) + " --- ");

    previousMillisLong = currentMillis;
    // -------->
    if (WiFi.status() == WL_CONNECTED)
    {
      timeClient.update();
    }
  }
}
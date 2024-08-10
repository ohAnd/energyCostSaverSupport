// Config.h
#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

#define CONFIG_FILE_PATH "/userconfig.json"

struct UserConfig
{
    char wifiSsid[64]             = "mySSID";
    char wifiPassword[64]         = "myPassword";
      
    uint8_t displayConnected      = 0; // OLED default

    unsigned int espUpdateTime    = 31;

    boolean wifiAPstart           = true;
    int selectedUpdateChannel     = 0; // 0 - release 1 - snapshot
    int timezoneOffest            = 7200; // default CEST
};

extern UserConfig userConfig;

// Define the UserConfigManager class
class UserConfigManager {
    public:
        UserConfigManager(const char *filePath = CONFIG_FILE_PATH, const UserConfig &defaultConfig = UserConfig());
        bool begin();
        bool loadConfig(UserConfig &config);
        void saveConfig(const UserConfig &config);
        void resetConfig();
        void printConfigdata();
        // String getWebHandler(keyAndValue_t* keyValueWebClient, unsigned int size);
        String getWebHandler(JsonDocument doc);
        

    private:
        const char *filePath;
        UserConfig defaultConfig;
        JsonDocument mappingStructToJson(const UserConfig &config);
        void mappingJsonToStruct(JsonDocument doc);
        String createWebPage(bool updated);
};

extern UserConfigManager configManager;

#endif // CONFIG_H
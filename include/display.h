#ifndef DISPLAY_H
#define DISPLAY_H

#include <U8g2lib.h>

// OLED display

#define BRIGHTNESS_MIN 10
#define BRIGHTNESS_MAX 250

#define EURO_SYMBOL 0x20AC
#define CENT_SYMBOL 0x00A2
#define DOLLAR_SYMBOL 0x0024

struct DisplayData {
    uint8_t delayHours=0;
    float saveCosts=0.0;
    float origCosts=0.0;
    const char *formattedTime=nullptr;
    const char *version=nullptr;
    uint8_t rssiGW=0;
};

class Display {
    public:
        Display();
        void setup();
        void renderScreen(String time, String version, uint8_t delayHours, float price, float priceSave);
        void drawFactoryMode(String version, String apName, String ip);
        void drawUpdateMode(String text,String text2="");
    private:
        void drawScreen();
        void drawHeader();
        void drawFooter();
        void drawMain();

        void screenSaver();
        void checkChangedValues();
        // private member variables
        DisplayData lastDisplayData;
        uint8_t brightness=BRIGHTNESS_MAX;
        u8g2_uint_t offset_x = 0; // shifting for anti burn in effect
        u8g2_uint_t offset_y = 0; // shifting for anti burn in effect
        bool valueChanged = false;
        uint16_t displayTicks = 0; // local timer state machine
};

#endif // DISPLAY_H
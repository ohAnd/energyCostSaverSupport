#include <display.h>

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

Display::Display() {}

void Display::setup()
{
    u8g2.begin();
    Serial.println(F("OLED display:\t initialized"));
}

void Display::renderScreen(String time, String version, uint8_t delayHours, float price, float priceSave)
{
    displayTicks++;
    if (displayTicks > 1200)
        displayTicks = 0; // after 1 minute restart

    lastDisplayData.version = version.c_str();
    lastDisplayData.formattedTime = time.c_str();
    lastDisplayData.delayHours = delayHours;
    lastDisplayData.saveCosts = priceSave;
    lastDisplayData.origCosts = price;

    checkChangedValues();

    if (valueChanged)
    {
        brightness = BRIGHTNESS_MAX;
        drawScreen(); // draw once to update values on screen
    }
    else if (brightness > BRIGHTNESS_MIN)
    {
        brightness = brightness - 5;
        // u8g2.setContrast(brightness);
        // u8g2.sendBuffer();
    }

    if (displayTicks % 20 == 0)
        drawScreen(); // draw every 1 second

    if (displayTicks == 0)
        screenSaver(); // every minute shift screen to avoid burn in
}

void Display::drawScreen()
{
    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.setFontPosTop();
    u8g2.setFontDirection(0);
    u8g2.setFontRefHeightExtendedText();

    drawHeader();
    drawMain();
    drawFooter();

    // set current choosen contrast
    u8g2.setContrast(brightness);

    u8g2.sendBuffer();
}

void Display::drawMain()
{
    // main screen

    String delayHours = String(lastDisplayData.delayHours); //

    u8g2.setFont(u8g2_font_logisoso28_tf);
    u8g2_uint_t width = u8g2.getUTF8Width(delayHours.c_str());
    int delayHours_xpos = (49 - width) / 2;
    u8g2.drawStr(delayHours_xpos + offset_x, 19 + offset_y, delayHours.c_str());

    u8g2.setFont(u8g2_font_logisoso28_tf);
    u8g2.drawStr(43 + offset_x, 19 + offset_y, "h");

    //
    // u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.setFont(u8g2_font_7x13_tf);
    u8g2.drawStr(74 + offset_x, 18 + offset_y, (String(lastDisplayData.origCosts, 2)).c_str());
    u8g2.drawStr(74 + offset_x, 40 + offset_y, (String(lastDisplayData.saveCosts, 2)).c_str());

    u8g2.setFont(u8g2_font_4x6_tf);
    String rightAligndText = "JETZT";
    width = u8g2.getUTF8Width(rightAligndText.c_str());
    u8g2.drawStr((127 - 4 - width) + offset_x, 12 + offset_y, rightAligndText.c_str());
    rightAligndText = "SPÄTER";
    width = u8g2.getUTF8Width(rightAligndText.c_str());
    u8g2.drawStr((127 - 4 - width) + offset_x, 34 + offset_y, rightAligndText.c_str());

    u8g2.setFont(u8g2_font_6x12_t_symbols);
    // todo: change currency symbol given by user settings
    uint16_t currency_symbol = EURO_SYMBOL;
    u8g2.drawGlyph(127 - 4 - 5 + offset_x, 20 + offset_y, currency_symbol);
    u8g2.drawGlyph(127 - 4 - 5 + offset_x, 43 + offset_y, currency_symbol);
}

void Display::drawFactoryMode(String version, String apName, String ip)
{
    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.setFontPosTop();
    u8g2.setFontDirection(0);
    u8g2.setFontRefHeightExtendedText();

    Serial.println(F("OLED display:\t showing factory mode"));
    // header
    u8g2.setFont(u8g2_font_5x7_tf);
    u8g2.drawStr(0 + offset_x, 0 + offset_y, "enerCoSa");
    u8g2.drawStr(55 + offset_x, 0 + offset_y, version.c_str());

    u8g2.drawHLine(0, 9, 128);

    // main screen
    u8g2.setFont(u8g2_font_siji_t_6x10);

    String centerString = "first start";
    u8g2_uint_t width = u8g2.getUTF8Width(centerString.c_str());
    int centerString_xpos = (128 - width) / 2;
    u8g2.drawStr(centerString_xpos + offset_x, 10 + offset_y, centerString.c_str());

    u8g2.setFont(u8g2_font_5x7_tf);
    centerString = "connect with wifi:";
    width = u8g2.getUTF8Width(centerString.c_str());
    centerString_xpos = (128 - width) / 2;
    u8g2.drawStr(centerString_xpos + offset_x, 20 + offset_y, centerString.c_str());
    centerString = apName.c_str();
    width = u8g2.getUTF8Width(centerString.c_str());
    centerString_xpos = (128 - width) / 2;
    u8g2.drawStr(centerString_xpos + offset_x, 31 + offset_y, centerString.c_str());
    centerString = "and open in your browser:";
    width = u8g2.getUTF8Width(centerString.c_str());
    centerString_xpos = (128 - width) / 2;
    u8g2.drawStr(centerString_xpos + offset_x, 43 + offset_y, centerString.c_str());

    u8g2.setFont(u8g2_font_siji_t_6x10);

    centerString = ("http://" + ip).c_str();
    width = u8g2.getUTF8Width(centerString.c_str());
    centerString_xpos = (128 - width) / 2;
    u8g2.drawStr(centerString_xpos + offset_x, 54 + offset_y, centerString.c_str());

    u8g2.setContrast(255);

    u8g2.sendBuffer();
}

void Display::drawHeader()
{
    // header
    // header - content center
    u8g2.setFont(u8g2_font_6x10_tf);
    // u8g2.drawStr(5 * 7 + offset_x, -1 + offset_y, "dtuGateway");
    u8g2.drawStr(10 + offset_x, -1 + offset_y, lastDisplayData.formattedTime);

    // // header - content left
    // u8g2.setFont(u8g2_font_siji_t_6x10);
    // // 0xE217 - wifi off
    // // 0xE218 - wifi 1
    // // 0xE219 - wifi 2
    // // 0xE21A - wifi 3
    // uint16_t wifi_symbol = 0xE217;
    // if (lastDisplayData.rssiGW > 80)
    //     wifi_symbol = 0xE21A;
    // else if (lastDisplayData.rssiGW > 50)
    //     wifi_symbol = 0xE219;
    // else if (lastDisplayData.rssiGW > 10)
    //     wifi_symbol = 0xE218;
    // u8g2.drawGlyph(3 + offset_x, 0 + offset_y, wifi_symbol);

    // header - bootom line
    // u8g2.drawRFrame(0 + offset_x, -11 + offset_y, 127, 22, 4);
}

void Display::drawFooter()
{
    // frame left
    u8g2.drawRFrame(0 + offset_x, 10 + offset_y, 66, 45, 4);
    // frame upper right
    u8g2.drawRFrame(70 + offset_x, 10 + offset_y, 127 - 71, 22, 4);
    // frame lower right
    u8g2.drawRFrame(70 + offset_x, 33 + offset_y, 127 - 71, 22, 4);
    // footer
    // footer - content
    u8g2.setFont(u8g2_font_5x7_tf);
    String version = String(lastDisplayData.version);
    u8g2_uint_t width = u8g2.getUTF8Width(version.c_str());
    int version_xpos = (127 - width) / 2;
    u8g2.drawStr(version_xpos + offset_x, 56 + offset_y, version.c_str());
}

void Display::drawUpdateMode(String text, String text2)
{
    uint8_t y1 = 25;
    Serial.println("OLED display:\t update mode");

    u8g2.clearBuffer();
    u8g2.setDrawColor(1);
    u8g2.setFontPosTop();
    u8g2.setFontDirection(0);
    u8g2.setFontRefHeightExtendedText();
    u8g2.setFont(u8g2_font_7x13_tf);

    if (text2 != "")
    {
        Serial.println("OLED display:\t update mode done");
        y1 = 17;
        u8g2_uint_t width = u8g2.getUTF8Width(text2.c_str());
        int text2_xpos = (128 - width) / 2;
        u8g2.drawStr(text2_xpos + offset_x, 32 + offset_y, text2.c_str());
    }

    u8g2_uint_t width = u8g2.getUTF8Width(text.c_str());
    int text_xpos = (128 - width) / 2;
    u8g2.drawStr(text_xpos + offset_x, y1 + offset_y, text.c_str());

    u8g2.setContrast(255);

    u8g2.sendBuffer();
}

void Display::screenSaver()
{
    if (offset_x == 0 && offset_y == 0)
    {
        offset_x = 1;
        offset_y = 0;
    }
    else if (offset_x == 1 && offset_y == 0)
    {
        offset_x = 1;
        offset_y = 1;
    }
    else if (offset_x == 1 && offset_y == 1)
    {
        offset_x = 0;
        offset_y = 1;
    }
    else if (offset_x == 0 && offset_y == 1)
    {
        offset_x = 0;
        offset_y = 0;
    }
    // contrast_value = contrast_value + 5;
    // if(contrast_value > 255) contrast_value = 100;
    // if(brightness == 255) brightness = 4;
    // else if(brightness != 255) brightness = 255;
}

void Display::checkChangedValues()
{
    valueChanged = false;
}

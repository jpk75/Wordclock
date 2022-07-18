#pragma once
#include "app-config.h"   // include at top
#include <cstdint>
#include <map>
#include <vector>
//#include <Adafruit_NeoPixel.h>
#include <NeoPixelBrightnessBus.h>
#include "parameter.h"

class WordClock
{
public:
    using WC_DESC = struct { int32_t start_pix; int32_t end_pix;};
    using WC_WORD_MAP_t = std::map<String, std::vector<WC_DESC>>;
    using WC_COLOR = union { uint16_t raw[3]; struct { uint16_t r; uint16_t g; uint16_t b;} col; };
    enum { DISP_MODE_STATE_UNDEF = -1, DISP_MODE_STATE_OFF = 0, DISP_MODE_STATE_ON = 1, DISP_MODE_STATE_MAX = 2};

private:
    static const WC_WORD_MAP_t mapWords;
    Parameter& parameter;   // reference to parameter
//    Adafruit_NeoPixel pixels; // object to control LED stripe
    NeoPixelBrightnessBus<NeoGrbFeature, NeoEsp8266Dma800KbpsMethod> pixels; // object to control LED stripe
//    NeoPixelBrightnessBus<NeoRgbFeature, NeoEsp8266BitBangWs2812xMethod> pixels; // object to control LED stripe
//    NeoPixelBrightnessBus<NeoRgbFeature, Neo800KbpsMethod> pixels; // object to control LED stripe
    int32_t ledCount;       // number of LEDs
    int32_t ledCols;        // number of columns
    int32_t ledRows;        // number of rows
    uint8_t ledBrightness;  // Brightness of display (0...255)
    uint8_t ledRed;         // red portion of resulting color (0...255)
    uint8_t ledGreen;       // green portion of resulting color (0...255)
    uint8_t ledBlue;        // blue portion of resulting color (0...255)
    uint8_t ledColorMode;   // colorMode
    uint8_t ledColorChangeInterval;   // color change interval (0...255) [s]
    int8_t ledColChangeState;
    WC_COLOR ledColorCur;   // current color (16bit)
    WC_COLOR ledColorNext;  // next color (16bit)
    WC_COLOR ledColorStep;  // steps for transition of each color
    int32_t  ledColorTransitionTime_ms; // time for color changes transition for one step
    bool    showDate;       // flag, indicating that showing date on display
    bool    displayDate;    // show date on display
    int32_t dateInterval_s; // Interval in seconds to show date
    bool    dispModeAuto;   // flag indicating auto on/off mode of display
    uint8_t dispModeHourOn; // hour to turn display on
    uint8_t dispModeMinOn;  // Minute to turn display on
    uint8_t dispModeHourOff;// hour to turn display off
    uint8_t dispModeMinOff; // Minute to turn display off
    DateTime dispModeNextEvent; // time object for next event
    int8_t  dispModeState;
    DateTime curTime;       // hold the current time
    bool testEnable;

#if CALC_SINELUT
    uint8_t lutSineLights[360];
    uint8_t ledColorSteps;
#else
    static const uint8_t lutSineLights[360];
    static const uint8_t ledColorSteps;
#endif
public:
    WordClock(Parameter& param, uint16_t count, int32_t cols, int32_t rows, int16_t pin/*, neoPixelType type = (NEO_GRB + NEO_KHZ800)*/);
    ~WordClock();
    void setup();
    void handle();
    void off(bool update = true);
    void on(bool update = true);
    void showIp();
    void onParameterChanged();

    void updateBrightness(uint8_t brightness);
    void updateColor(uint8_t colorMode, uint8_t interval, uint8_t red, uint8_t green, uint8_t blue);
    void updateDate(bool on, int32_t interval);
    void updateDisplayMode(uint8_t offHour, uint8_t offMin, uint8_t onHour, uint8_t onMin);
private:
    void showCurrentTime();
    void showMinutes(int32_t minutes);
    void showHours(int32_t hours, int32_t minutes);
    void showCurrentDate();
    void checkAndSetColor();
    bool checkDisplayOnOff();
    void handleColorModeRandom();

    void setLed(std::vector<WordClock::WC_DESC> vd, bool on);
    void setLed(int32_t first, int32_t last, bool on);
    void setLed(const RgbColor& col, int32_t first, int32_t last, bool on);
    void setLedLine(int32_t firstX, int32_t lastX, int32_t y, bool on);
    int32_t ledXY (int32_t x, int32_t y);
    void printAt (int32_t number, int32_t x, int32_t y);
    std::vector<WC_DESC> getWordDesc(const String& word);
};
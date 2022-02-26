#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "wordclock.h"
#include "debuglog.h"


extern RTC_DS3231 rtc;
extern BH1750 lightMeter;

#if !CALC_SINELUT
const uint8_t WordClock::ledColorSteps = 120;
const uint8_t WordClock::lutSineLights[360] = {
  0,   0,   0,   0,   0,   1,   1,   2,
  2,   3,   4,   5,   6,   7,   8,   9,
 11,  12,  13,  15,  17,  18,  20,  22,
 24,  26,  28,  30,  32,  35,  37,  39,
 42,  44,  47,  49,  52,  55,  58,  60, 
 63,  66,  69,  72,  75,  78,  81,  85,
 88,  91,  94,  97, 101, 104, 107, 111,
114, 117, 121, 124, 127, 131, 134, 137,
141, 144, 147, 150, 154, 157, 160, 163,
167, 170, 173, 176, 179, 182, 185, 188, 
191, 194, 197, 200, 202, 205, 208, 210,
213, 215, 217, 220, 222, 224, 226, 229,
231, 232, 234, 236, 238, 239, 241, 242,
244, 245, 246, 248, 249, 250, 251, 251,
252, 253, 253, 254, 254, 255, 255, 255, 
255, 255, 255, 255, 254, 254, 253, 253,
252, 251, 251, 250, 249, 248, 246, 245,
244, 242, 241, 239, 238, 236, 234, 232,
231, 229, 226, 224, 222, 220, 217, 215, 
213, 210, 208, 205, 202, 200, 197, 194,
191, 188, 185, 182, 179, 176, 173, 170,
167, 163, 160, 157, 154, 150, 147, 144,
141, 137, 134, 131, 127, 124, 121, 117,
114, 111, 107, 104, 101,  97,  94,  91, 
 88,  85,  81,  78,  75,  72,  69,  66,
 63,  60,  58,  55,  52,  49,  47,  44,
 42,  39,  37,  35,  32,  30,  28,  26,
 24,  22,  20,  18,  17,  15,  13,  12, 
 11,   9,   8,   7,   6,   5,   4,   3,
  2,   2,   1,   1,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0, 
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0,
  0,   0,   0,   0,   0,   0,   0,   0
};
#endif

const WordClock::WC_WORD_MAP_t WordClock::mapWords = {
     {"it", { {102, 103}} }
    ,{"is", { {105, 107}} }
    ,{"clock", { {1, 3}} }
    ,{"quarter", { {83, 89}} }
    ,{"to", { {76, 78}} }
    ,{"past", { {68, 71}} }
    ,{"half", { {57, 60}} }
    ,{"1", { {101, 101}} }
    ,{"2", { {113, 113}} }
    ,{"3", { {0, 0}} }
    ,{"4", { {12, 12}} }
    ,{"5", { {109, 112}} }
    ,{"10", { {97, 100}} }
    ,{"20", { {90, 96}} }
    ,{"one", { {54, 56}} }
    ,{"ones", { {53, 56}} }
    ,{"two", { {46, 49}} }
    ,{"three", { {35, 38}} }
    ,{"four", { {42, 45}} }
    ,{"five", { {64, 67}} }
    ,{"six", { {30, 34}} }
    ,{"seven", { {13, 18}} }
    ,{"eight", { {24, 27}} }
    ,{"nine", { {5, 8}} }
    ,{"ten", { {8, 11}} }
    ,{"eleven", { {62, 64}} }
    ,{"twelve", { {19, 23}} }
};
WordClock::WordClock(Parameter& param, uint16_t count, int32_t cols, int32_t rows, int16_t pin/*, neoPixelType type*/)
    : parameter(param)
    , pixels(count, pin)
    , ledCount(count)
    , ledCols(cols)
    , ledRows(rows)
    , ledBrightness(50)
    , ledRed(128)
    , ledGreen(128)
    , ledBlue(128)
    , ledColorMode(Parameter::COLOR_MODE_SINGLE)
    , ledColorChangeInterval(PARAM_COLOR_CHANGE_INTERVAL_DEFAULT)
    , ledColChangeState(0)
    , showDate(true)
    , displayDate(false)
    , dateInterval_s(30)
    , dispModeAuto(false)
    , dispModeHourOn(0)
    , dispModeMinOn(0)
    , dispModeHourOff(0)
    , dispModeMinOff(0)
    , dispModeNextEvent((uint32_t)0)
    , dispModeState(DISP_MODE_STATE_UNDEF)
    , curTime((uint32_t)0)
    , testEnable(false)
{
}

WordClock::~WordClock()
{

}

void WordClock::setup(){
  time_t tm = {};
  time(&tm);
  randomSeed((uint32_t)tm);

  // calc sine LUT
#if CALC_SINELUT
//sine wave
  ledColorSteps = 120;
  uint16_t i = 0;
  Serial.println("const uint8_t lutSineLights[360]={");
  for (float k=PI; k<(3*PI); k=k+PI/float(ledColorSteps)) 
  {
    lutSineLights[i]=int((cos(k)+1)*127.7); // I use cosinus
    if (lutSineLights[i]<10) Serial.print(" "); // I like to keep the table clean
    if (lutSineLights[i]<100) Serial.print(" "); // told you I use them a lot!
    Serial.print(lutSineLights[i]);
    if (i<255) Serial.print(", "); //'if' portion is useful if you're not filling the
                                      // rest of the table with zeroes. It makes sure there's
                                      // no extra comma after the final value of array.
                                      // I keep it here just for that situation
    if (i%8==7) Serial.println();  // new line for cleaner table!
    i++;
  }

  for (; i<360; i++) // fill the rest with zeroes
  {
    lutSineLights[i] = 0;
    Serial.print("  0");
    if (i<359) Serial.print(", ");
    if ((i%8)==7) Serial.println();
  }
  Serial.println("};");
#endif

  parameter.registerChangedCallback([&]{ this->onParameterChanged();});
  pixels.Begin();
  pixels.SetBrightness(ledBrightness);
  pixels.ClearTo(Neo3ByteElements::ColorObject(0,0,0));
  pixels.Show();
}

void WordClock::handle(){    
    // get current time
    time_t now = rtc.now().unixtime();
    struct tm ct = {};
    localtime_r(&now, &ct);
    curTime = DateTime(ct.tm_year+1900, ct.tm_mon+1,ct.tm_mday, ct.tm_hour, ct.tm_min, ct.tm_sec);

    if (testEnable){
      setLed(RgbColor(255,255,255), 0, ledCount-1, 1);
      pixels.Show();
    }
    else if (checkDisplayOnOff()){
      showCurrentDate();
        if (!displayDate){
            showCurrentTime();
        }
    }
    else{
      off();
    }
}

void WordClock::updateBrightness(uint8_t brightness){  ledBrightness = brightness; pixels.SetBrightness(ledBrightness); }
void WordClock::updateColor(uint8_t colorMode, uint8_t interval, uint8_t red, uint8_t green, uint8_t blue){
  ledColorMode = colorMode;
  ledColorChangeInterval = interval;
  ledColChangeState = 0;

  if (Parameter::COLOR_MODE_SINGLE == ledColorMode){
    ledRed = red;
    ledGreen = green;
    ledBlue = blue;
  }
  else if (Parameter::COLOR_MODE_RANDOM == ledColorMode){
    ledRed = (rand() % 256);
    ledGreen = (rand() % 256);
    ledBlue = (rand() % 256);
  } else {
    ledRed = red;
    ledGreen = green;
    ledBlue = blue;
  }
}
void WordClock::updateDate(bool on, int32_t interval){ showDate = on; dateInterval_s = interval; }
void WordClock::updateDisplayMode(uint8_t offHour, uint8_t offMin, uint8_t onHour, uint8_t onMin){
    dispModeAuto = !((offHour==onHour)&&(offMin==onMin)); // if on and off time are the same -> auto display mode is disabled
    dispModeHourOff = offHour;
    dispModeMinOff  = offMin;
    dispModeHourOn  = onHour;
    dispModeMinOn   = onMin;
    dispModeState = DISP_MODE_STATE_UNDEF;
    DebugLog.print(F("Display Mode Parameter: AutoMode = ")); DebugLog.print(dispModeAuto);
    DebugLog.print(F(", Off time = ")); DebugLog.print(dispModeHourOff); DebugLog.print(F(":")); DebugLog.print(dispModeMinOff);
    DebugLog.print(F(", On time = ")); DebugLog.print(dispModeHourOn); DebugLog.print(F(":")); DebugLog.print(dispModeMinOn);
    DebugLog.println("");
}

///////////////////////////////////////////////////////////////////////////////////////////
// Funtion to check if color change is needed accordingly to color mode and color change
// interval.
///////////////////////////////////////////////////////////////////////////////////////////
void WordClock::checkAndSetColor(){
  if (Parameter::COLOR_MODE_RANDOM == ledColorMode){
    handleColorModeRandom();
  }
}

void WordClock::handleColorModeRandom(){
#if 1
  static uint8_t count = 0;
  static uint8_t step = 0;
  static uint32_t timeLast = millis();

  switch(ledColChangeState){
    case 0:   // init state -> do calulations
    {
      // set current color
      ledColorCur.col.r = ((uint16_t)ledRed) << 8;
      ledColorCur.col.g = ((uint16_t)ledGreen) << 8;
      ledColorCur.col.b = ((uint16_t)ledBlue) << 8;

      // calc transition time for one step
      ledColorTransitionTime_ms = (int32_t)ledColorChangeInterval * 1000 / ledColorSteps;
      step = 0;
      ledColChangeState = 1;  // change state to set next color
    }
    case 1:  // set next color
    {
      uint8_t a0, a1, a2;
      a0=random(240);
      ledColorNext.raw[count]=lutSineLights[a0]<<8;
      a1=random(1);
      a2=((!a1)+count+1)%3;
      a1=(count+a1+1)%3;
      ledColorNext.raw[a1]=lutSineLights[(a0+100)%240]<<8;
      ledColorNext.raw[a2]=0;
      count++; // to avoid repeating patterns
      count%=3;

      // calc transisitions for colors
      ledColorStep.col.r = (ledColorNext.col.r - ledColorCur.col.r) / 255;
      ledColorStep.col.g = (ledColorNext.col.g - ledColorCur.col.g) / 255;
      ledColorStep.col.b = (ledColorNext.col.b - ledColorCur.col.b) / 255;

      ledColChangeState = 2; // change state to wait state
    }
    case 2:   // wait state
    {
      // get current time
      time_t now = time(nullptr);
      if (0 == (now % ledColorChangeInterval)){
        ledColChangeState = 3; // change state to transition state
      }
    }
    case 3: // transition state
    {
      if (step >= ledColorSteps){
        ledColChangeState = 4;  // change state to finalize state
      } else {
        if (millis()>(timeLast+ledColorTransitionTime_ms)){
          timeLast = millis();
          ledRed   = (ledColorCur.col.r + (ledColorStep.col.r * lutSineLights[step])) >> 8;
          ledGreen = (ledColorCur.col.g + (ledColorStep.col.g * lutSineLights[step])) >> 8;
          ledBlue  = (ledColorCur.col.b + (ledColorStep.col.b * lutSineLights[step])) >> 8;
          step++;
        }
      }
      break;
    }
    case 4: // finalize state
    {
      ledColorCur = ledColorNext;
      ledRed   = ledColorNext.col.r >> 8;
      ledGreen = ledColorNext.col.g >> 8;
      ledBlue  = ledColorNext.col.b >> 8;
      step = 0;
      ledColChangeState = 1;
    }
  } 
#else
  static bool changed = false;
  static uint8_t count = 0;

  // get current time
  time_t now = time(nullptr);
  if (0 == (now % ledColorChangeInterval)){
    if (!changed){
      uint8_t color[3] = {};
      uint8_t a0, a1, a2;
      a0=random(240);
      color[count]=lutSineLights[a0];
      a1=random(1);
      a2=((!a1)+count+1)%3;
      a1=(count+a1+1)%3;
      color[a1]=lutSineLights[(a0+100)%240];
      color[a2]=0;
      count++; // to avoid repeating patterns
      count%=3;

      ledRed   = color[0];
      ledGreen = color[1];
      ledBlue  = color[2];
      changed  = true;
    }
  }
  else{
    changed = false;
  }
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////
// Funtion to check display mode state
// returns 
// false -> turn display off
// true -> turn display on 
///////////////////////////////////////////////////////////////////////////////////////////
bool WordClock::checkDisplayOnOff(){
    bool result = true;

    if (dispModeAuto){
      int32_t secOff = ((int32_t)dispModeHourOff*3600) + ((int32_t)dispModeMinOff*60);
      int32_t secOn = ((int32_t)dispModeHourOn*3600) + ((int32_t)dispModeMinOn*60);
      int32_t secDiff = secOn-secOff;

      if (dispModeNextEvent == DateTime((uint32_t)0) || (DISP_MODE_STATE_UNDEF == dispModeState)){
        // if next event is not set -> set it
        DateTime dtOn = DateTime(curTime.year(), curTime.month(), curTime.day(), dispModeHourOn, dispModeMinOn, 0);
        DateTime dtOff = DateTime(curTime.year(), curTime.month(), curTime.day(), dispModeHourOff, dispModeMinOff, 0);

        if (dtOn < dtOff){
          if (curTime < dtOn){
            dispModeNextEvent = dtOn;
            dispModeState = DISP_MODE_STATE_OFF;  //set current display state
          }else if (curTime > dtOn && curTime < dtOff){  // on
            dispModeNextEvent = dtOff;
            dispModeState = DISP_MODE_STATE_ON;  //set current display state
          } else {  // off
            dispModeNextEvent = dtOn + TimeSpan(1,0,0,0); 
            dispModeState = DISP_MODE_STATE_OFF;  //set current display state
          }
        } else if (dtOn > dtOff){
          if (curTime < dtOff){
            dispModeNextEvent = dtOff;
            dispModeState = DISP_MODE_STATE_ON;  //set current display state
          }else if (curTime > dtOff && curTime < dtOn){  // off
            dispModeNextEvent = dtOn;
            dispModeState = DISP_MODE_STATE_OFF;  //set current display state
          } else {  // on
            dispModeNextEvent = dtOff + TimeSpan(1,0,0,0);
            dispModeState = DISP_MODE_STATE_ON;  //set current display state
          }
        }

        DebugLog.println(String(F("Time on  = ")) + dtOn.year() + String(F("-")) + dtOn.month() + String(F("-")) + dtOn.day() + String(F(" T")) + dtOn.hour() + String(F(":")) + dtOn.minute());
        DebugLog.println(String(F("Time off = ")) + dtOff.year() + String(F("-")) + dtOff.month() + String(F("-")) + dtOff.day() + String(F(" T")) + dtOff.hour() + String(F(":")) + dtOff.minute());
        DebugLog.println(String(F("Next Event = ")) + dispModeNextEvent.year() + String("-") + dispModeNextEvent.month() + String(F("-")) + dispModeNextEvent.day() + String(F(" T")) + dispModeNextEvent.hour() + String(F(":")) + dispModeNextEvent.minute());
      }

      if (curTime > dispModeNextEvent){
        DebugLog.println(F("checkDisplayOnOff event triggered"));
        result = (DISP_MODE_STATE_OFF == dispModeState) ? true : false;
        if (DISP_MODE_STATE_OFF == dispModeState){
          secDiff = secOff-secOn;
          dispModeState = DISP_MODE_STATE_ON;
        }
        else{
          secDiff = secOn-secOff;
          dispModeState = DISP_MODE_STATE_OFF;
        }
        dispModeNextEvent = dispModeNextEvent + TimeSpan(secDiff);
        if (0 > secDiff) dispModeNextEvent = dispModeNextEvent + TimeSpan(1,0,0,0);
        DebugLog.println(String(F("Next Event = ")) + dispModeNextEvent.year() + String(F("-")) + dispModeNextEvent.month() + String(F("-")) + dispModeNextEvent.day() + String(F(" T")) + dispModeNextEvent.hour() + String(F(":")) + dispModeNextEvent.minute());

      }
      else{
//        DebugLog.println(F("checkDisplayOnOff keep state"));
      }
      result = (DISP_MODE_STATE_OFF == dispModeState) ? false : true;
    }

    return result;
}

void WordClock::onParameterChanged(){
    DebugLog.println(F("onParameterChanged"));
    testEnable = parameter.getTestEnable();
    updateBrightness(parameter.brightness);
    updateColor(parameter.colorMode, parameter.colorChangeInterval, parameter.red, parameter.green, parameter.blue);
    updateDate(parameter.showDate, parameter.dateInterval);
    updateDisplayMode(parameter.dispOffHour, parameter.dispOffMinute, parameter.dispOnHour, parameter.dispOnMinute);
}

void WordClock::off(bool update){
    pixels.ClearTo(RgbColor(0,0,0));
    if (update)  pixels.Show();
}

void WordClock::on(bool update){
    pixels.ClearTo(RgbColor(ledRed, ledGreen, ledBlue));
    if (update)  pixels.Show();
}

void WordClock::showCurrentTime(){
    pixels.ClearTo(RgbColor(0,0,0));

    // Default text
    setLed(getWordDesc(F("it")), true);
    setLed(getWordDesc(F("is")), true);

    // show minutes
    showMinutes(curTime.minute());

    // show hours
    showHours(curTime.hour(), curTime.minute());

    pixels.Show();
}

void WordClock::showMinutes(int32_t minutes){
    int32_t m = minutes % 5;
    int32_t min_div = (minutes / 5);

    // minutes 1 ... 4
    for(int32_t i=1; i <= m; i++){
        auto vd = getWordDesc(String(i));
        setLed(vd, true);
    }


    bool bMin5  = (min_div==1)||(min_div==5)||(min_div==7)||(min_div==11);
    bool bMin10 = (min_div==2)||(min_div==10);
    bool bMin20 = (min_div==4)||(min_div==8);
    bool bQuarter = (min_div==3)||(min_div==9);
    bool bHalf = (min_div==5)||(min_div==6)||(min_div==7);
    setLed(getWordDesc(F("5")), bMin5);
    setLed(getWordDesc(F("10")), bMin10);
    setLed(getWordDesc(F("20")), bMin20);
    setLed(getWordDesc(F("quarter")), bQuarter);
    setLed(getWordDesc(F("half")), bHalf);

    // qualifier
    bool bTo = (min_div==5)||(min_div==8)||(min_div==9)||(min_div==10)||(min_div==11);
    bool bPast = (min_div==1)||(min_div==2)||(min_div==3)||(min_div==4)||(min_div==7);
    setLed(getWordDesc(F("to")), bTo);
    setLed(getWordDesc(F("past")), bPast);
}

void WordClock::showHours(int32_t hours, int32_t minutes){
    const char* szHours[] = {"ones", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "ten", "eleven", "twelve"};
    int32_t h = hours % 12;
    bool bClock = (minutes < 5);

    // handle special cases
    if (0 == h) h = 12;
    if (minutes >= 25){
        if (12 == h) h = 1;
        else h++;
    }
    if((1 == h) && (4 < minutes)) h = 0;

//    DebugLog.println(String("hours = ") + String(hours));
//    DebugLog.println(String("h = ") + String(h));
    setLed(getWordDesc(szHours[h]), true);
    setLed(getWordDesc(F("clock")), bClock);
}

/**
 * Show current date on clock with moving digits
 */
void WordClock::showCurrentDate() {
    static int32_t dateState = 0;
    static int32_t iDay = 0;
    static int32_t iMonth = 0;
    static int32_t iYear = 0;
    static int32_t x = 11;
    static uint32_t lastUpdate = 0;
    switch(dateState){
    case 0:     // check state
    {
        time_t now = time(nullptr);                       // read the current time
        displayDate = showDate && ((now % dateInterval_s)==0);
        if (displayDate){
            dateState = 1;  // change to init state
            break;
            // no break here -> directly fall through to init state
        } else {
            break;  // leave state machine here
        }
    }
    case 1: // init state
    {
        iDay = curTime.day();
        iMonth = curTime.month();
        iYear = curTime.year();
        x = 11;

        dateState = 2;  // change to display state
        // break;   <- no break here -> directly step over to state display
    }
    case 2: // display date
    {
        pixels.ClearTo(RgbColor(0,0,0));
        printAt (iDay / 10,   x,    2);
        printAt (iDay % 10,   x + 6,  2);
        setLed(ledXY(x + 12, 2), ledXY(x + 12, 2), -1); //sets first point
        printAt (iMonth / 10, x + 13, 2);
        printAt (iMonth % 10, x + 19, 2);
        if (iYear < 1000)
            iYear = iYear + 2000;
        setLed(ledXY(x + 25, 2), ledXY(x + 25, 2), -1); //sets second point
        printAt (iYear / 1000, x + 28, 2);
        iYear = iYear % 1000;
        printAt (iYear / 100,  x + 34, 2);
        iYear = iYear % 100;
        printAt (iYear / 10,  x + 40, 2);
        printAt (iYear % 10,  x + 46, 2);
        pixels.Show();

        x--;    // update x position
        lastUpdate = millis();
        dateState = 3;      // change to state wait

    }
    case 3:     // wait state
    {
        if ( 150 < (millis()-lastUpdate)){
            if (-53 > x){
                // end state
                dateState = 4;
            }
            else{
                dateState = 2;
            }
        }
        break;
    }
    case 4:     // end state
    {
        displayDate = false;
        dateState = 0;  // return to check state
        break;
    }
    default:    // invalid state
    {
        dateState = 0;  // reset state machine
    }
    }
}

void WordClock::showIp () {
    IPAddress ip = WiFi.localIP();

    DebugLog.print(F("Displaying IP address "));
    DebugLog.println(ip);

    for (int32_t x = ledCols; x > -75; x--) {
    pixels.ClearTo(RgbColor(0,0,0));

    int32_t offSet = x;

    // Loop over 4 bytes of IP Address
    for (int32_t idx = 0; idx < 4; idx++) {

        uint8_t octet = ip[idx];
        bool is100Zero = ((octet/100) == 0);
        bool is10Zero  = is100Zero && (((octet%100)/10) == 0);
        if (!is100Zero) {
            printAt (octet / 100, offSet, 2);
            offSet = offSet + 6;
       }
        octet = octet % 100;
        if (!is10Zero) {
            printAt (octet / 10, offSet, 2);
            offSet = offSet + 6;
        }
        printAt (octet % 10, offSet, 2);
        offSet = offSet + 6;

        // add delimiter between octets
        if (idx < 3) {
        setLed(ledXY(offSet, 2), ledXY(offSet, 2), -1);  //sets point
        offSet+=2;
        }
    }

    pixels.Show();
    delay (300);    // set speed of timeshift
    }
}


/**
 * Turn a sequence of LEDs ON or OFF, depending on boolean value on
 */
void WordClock::setLed(std::vector<WordClock::WC_DESC> vd, bool on){
    for(auto& d : vd){
        setLed(d.start_pix, d.end_pix, on);
    }
}

/**
 * Turn a series of LEDs ON or OFF, depending on boolean value on
 */
void WordClock::setLed(int32_t first, int32_t last, bool on){
    int32_t f = std::min(first, last);
    int32_t l = std::max(first, last);

    if (on){
        for(int32_t i=f; i <=l; i++){
            if ((0 <= i) && (ledCount > i)){
                checkAndSetColor();
                pixels.SetPixelColor(i, RgbColor(ledRed, ledGreen, ledBlue));
            }
        }
    }
}

void WordClock::setLed(const RgbColor& col, int32_t first, int32_t last, bool on){
    int32_t f = std::min(first, last);
    int32_t l = std::max(first, last);

    if (on){
        for(int32_t i=f; i <=l; i++){
            if ((0 <= i) && (ledCount > i)){
                checkAndSetColor();
                pixels.SetPixelColor(i, col);
            }
        }
    }
}

/**
 * Turn a horizontal sequence of LEDs from a given line y ON or OFF, depending on boolean value on
 */
void WordClock::setLedLine(int32_t firstX, int32_t lastX, int32_t y, bool on) {
    int32_t fx = std::min(firstX, lastX);
    int32_t lx = std::max(firstX, lastX);


    for (int x = fx; x <= lx; x++) {
        setLed(ledXY(x, y), ledXY(x, y), on);
    }
}

/**
 * Convert x/y coordinates into LED number. Origin is bottom left
 * return -1 for invalid coordinate
 */
int32_t WordClock::ledXY (int32_t x, int32_t y) {

  // Test for valid coordinates
  // If outside panel return -1
  if ((x < 0)  || (x >= ledCols) || (y < 0)  || (y >= ledRows))
    return -1;

  int32_t ledNr = (y) * ledCols;

  if ((y % 2) == 0)
    ledNr = ledNr + (ledCols-1) - x;
  else
    ledNr = ledNr + x;

  ledNr ++;
  ledNr += (y>0) ? 1 : 0;
  ledNr += (y==(ledRows-1)) ? 1 : 0;

  return ledNr;
}

/**
 * sets, where the numbers from 1 to 9 are printed
 */
void WordClock::printAt (int32_t number, int32_t x, int32_t y) {
  switch (number) {

    case 0:   //number 0
      setLedLine(x + 1, x + 3, y + 6, -1); //-1 is true, so switchOn
      for (int yd = 1; yd < 6; yd++) {
        setLed(ledXY(x,   y + yd), ledXY(x,   y + yd), -1);
        setLed(ledXY(x + 4, y + yd), ledXY(x + 4, y + yd), -1);
      }
      setLedLine(x + 1, x + 3, y, -1);
      break;

    case 1:   // number 1
      setLed(ledXY(x + 3, y + 5), ledXY(x + 3, y + 5), -1);
      setLed(ledXY(x + 2, y + 4), ledXY(x + 2, y + 4), -1);
      for (int yd = 0; yd <= 6; yd++) {
        setLed(ledXY(x + 4,   y + yd), ledXY(x + 4,   y + yd), -1);
      }
      break;

    case 2:   // number 2
      for (int d = 1; d <= 4; d++) {
        setLed(ledXY(x + d,   y + d), ledXY(x + d,   y + d), -1);
      }
      setLedLine(x, x + 4, y, -1);
      setLed(ledXY(x, y + 5),   ledXY(x, y + 5), -1);
      setLed(ledXY(x + 4, y + 5), ledXY(x + 4, y + 5), -1);
      setLedLine(x + 1, x + 3, y + 6, -1);
      break;

    case 3:    // number 3
      for (int yd = 1; yd <= 2; yd++) {
        setLed(ledXY(x + 4, y + yd + 3), ledXY(x + 4, y + yd + 3), -1);
        setLed(ledXY(x + 4, y + yd),   ledXY(x + 4, y + yd), -1);
      }
      for (int yd = 0; yd < 7; yd = yd + 3) {
        setLedLine(x + 1, x + 3, y + yd, -1);
      }
      setLed(ledXY(x, y + 1), ledXY(x, y + 1), -1);
      setLed(ledXY(x, y + 5), ledXY(x, y + 5), -1);
      break;

    case 4:     // number 4
      for (int d = 0; d <= 3; d++) {
        setLed(ledXY(x + d,   y + d + 3), ledXY(x + d,   y + d + 3), -1);
      }
      for (int yd = 0; yd <= 3; yd++) {
        setLed(ledXY(x + 3,   y + yd), ledXY(x + 3,   y + yd), -1);
      }
      setLedLine(x, x + 4, y + 2, -1);
      break;

    case 5:     // number 5
      setLedLine(x, x + 4, y + 6, -1);
      setLed(ledXY(x  , y + 5), ledXY(x  , y + 5), -1);
      setLed(ledXY(x  , y + 4), ledXY(x  , y + 4), -1);
      setLedLine(x, x + 3, y + 3, -1);
      setLed(ledXY(x + 4, y + 2), ledXY(x + 4, y + 2), -1);
      setLed(ledXY(x + 4, y + 1), ledXY(x + 4, y + 1), -1);
      setLedLine(x, x + 3, y, -1);
      break;
    case 6:     // number 6
      for (int d = 0; d <= 3; d++) {
        setLed(ledXY(x + d,   y + d + 3), ledXY(x + d,   y + d + 3), -1);
      }
      for (int yd = 0; yd < 4; yd = yd + 3) {
        setLedLine(x + 1, x + 3, y + yd, -1);
      }
      setLed(ledXY(x, y + 1), ledXY(x, y + 1), -1);
      setLed(ledXY(x, y + 2), ledXY(x, y + 2), -1);
      setLed(ledXY(x + 4, y + 1), ledXY(x + 4, y + 1), -1);
      setLed(ledXY(x + 4, y + 2), ledXY(x + 4, y + 2), -1);
      break;

    case 7:     // number 7
      for (int yd = 0; yd <= 6; yd++) {
        setLed(ledXY(x + 3,   y + yd), ledXY(x + 3,   y + yd), -1);
      }
      setLedLine(x + 1, x + 4, y + 3, -1);
      setLedLine(x, x + 3, y + 6, -1);
      break;

    case 8:     // number 8
      for (int yd = 1; yd <= 2; yd++) {
        setLed(ledXY(x + 4, y + yd + 3), ledXY(x + 4, y + yd + 3), -1);
        setLed(ledXY(x + 4, y + yd),   ledXY(x + 4, y + yd), -1);
        setLed(ledXY(x, y + yd + 3),   ledXY(x, y + yd + 3), -1);
        setLed(ledXY(x, y + yd),     ledXY(x, y + yd), -1);
      }
      for (int yd = 0; yd < 7; yd = yd + 3) {
        setLedLine(x + 1, x + 3, y + yd, -1);
      }
      break;

    case 9:     // number 9
      for (int d = 0; d <= 3; d++) {
        setLed(ledXY(x + d + 1,   y + d),  ledXY(x + d + 1,   y + d), -1);
      }
      for (int yd = 4; yd <= 5; yd++) {
        setLed(ledXY(x, y + yd),     ledXY(x, y + yd), -1);
        setLed(ledXY(x + 4, y + yd),   ledXY(x + 4, y + yd), -1);
      }
      setLedLine(x + 1, x + 3, y + 6, -1);
      setLedLine(x + 1, x + 4, y + 3, -1);

      break;
  }

}

std::vector<WordClock::WC_DESC> WordClock::getWordDesc(const String& word){
    auto v = mapWords.find(word);
    if (v != mapWords.end()){
        return v->second;
    }
    else{
        return std::vector<WordClock::WC_DESC>();
    }
}
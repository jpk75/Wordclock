#include "wordclock-config.h"   // include at top
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <RTClib.h>
#include <BH1750.h>
#include <time.h>
#include <LittleFS.h>
#include <debug.h>
#include "stringex.h"

//#include <Wire.h>
#include "debuglog.h"
#include "parameter.h"
#include "wordclock.h"
#include "webservice.h"
#include "timehandler.h"
#include "WebWordclock.h"

const char* VERSION = WORDCLOCK_VERSION;
const char* HOSTNAME_BASE = WORDCLOCK_HOSTNAME;

// I2C adress of the RTC  DS3231 (Chip on ZS-042 Board)
const int32_t RTC_I2C_ADDRESS = I2C_ADR_RTC;

WiFiManager wifiManager;

Parameter parameter;

String timeZone = NTP_TIMEZONE_DEFAULT;
String ntpServer = NTP_SERVER_DEFAULT;

// rtc communication object
RTC_DS3231 rtc;
bool rtcStarted = false;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// NTP available flag
TimeHandler thTime(parameter);

// lightmeter
BH1750 lightMeter(I2C_ADR_BH1750);
bool bh1750Started = false;

// web service object
WebService webService(parameter);

// NeoPixel control object
WordClock wordclock(parameter, NEOPIXEL_NUMPIXELS, WORDCLOCK_COLS, WORDCLOCK_ROWS, NEOPIXEL_PIN);
WebWordclock webWordclock(parameter, webService);

/**
 * get RTC - if any
 */
int32_t checkRTC() {
   // Initialize & check RTC
   if (!rtcStarted) {
      if (rtc.begin()) {
      rtcStarted = true;
      if (rtc.lostPower()) {
         DebugLog.println("RTC lost power, let's set the time!");
         // When time needs to be set on a new device, or after a power loss, the
         // following line sets the RTC to the date & time this sketch was compiled
         rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
         // This line sets the RTC with an explicit date & time, for example to set
         // January 21, 2014 at 3am you would call:
         // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
      }

      // start RTC Communication via Wire.h library
      DebugLog.println("Start RTC communication");
      Wire.begin();
      } else {
   //      DebugLog.println("Couldn't find RTC");
         rtcStarted = false;
      }
  }
  else{
      DebugLog.println("Checking RTC time");
      if (thTime.isNtpValid()){
         DebugLog.println("NTP Server available");
         if (thTime.epoch() != rtc.now().unixtime()){
            // time differs between system time (NTP) and RTC
            // => adjust RTC
            rtc.adjust(DateTime((uint32_t)thTime.epoch()));
            DebugLog.println("Adjusting RTC time");
            DebugLog.println(String("RTC Time set to ") + DateTime((uint32_t)thTime.epoch()).timestamp());
         }
      }
      else{
         DebugLog.println("NTP server not available - don't touch RTC time");
      }
  }

  return rtcStarted;
}

void startBH1750(){
   if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
      bh1750Started = true;
      DebugLog.println(F("BH1750 Advanced begin"));
   }
   else {
      bh1750Started = false;
      DebugLog.println(F("Error initialising BH1750"));
   }
}

// extern "C" uint8_t sntp_getreachability(uint8_t);
// bool getNtpServer(bool reply = false) {   
//    ntpAvailable = sntp_getreachability(0) ? true : false;
//    if (!ntpAvailable) configTime(parameter.getTimezone().c_str(), parameter.getNtpServer().c_str());   // set timezone https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv

//    DebugLog.print("Timezone = "); DebugLog.println(parameter.getTimezone());
//    DebugLog.print("ntpServer = "); DebugLog.println(parameter.getNtpServer());
//    DebugLog.print("NTP Server available = "); DebugLog.println(ntpAvailable);

//    return ntpAvailable;
// }


// setup time
void setupTime(){
   String filename(F("/zones.json"));
   String tz = PARAM_TIMEZONE_CODE_DEFAULT;
   // get timezone from json file
   auto f = LittleFS.open(filename, "r");
   if (f){
      DebugLog.println(F("Open File '") + filename + F("' successful"));
      DebugLog.println(F("parsing file ..."));
      while(f.available()){
         String s = f.readStringUntil('\n');
         String split[2];
         int32_t n = StringEx::StringSplit(s, ':', split, 2);
         if (2 == n) {
            split[0].replace(F("\""), F(""));
            if (split[0] == parameter.getTimezone()){
               split[1].replace(F("\","), F(""));  // 1st remove chars '",'
               split[1].replace(F("\""), F(""));   // 2nd remove char '"'
               tz = split[1];
               DebugLog.println(F("Timezone found: Zone = '") + split[0] + F("', Code = ") + split[1]);
            }
         }
      } 
      DebugLog.println(F("closing file ..."));
      f.close();
   } else {
      DebugLog.println(F("Failed to open File '") + filename + F("'!"));
   }

   DebugLog.println(F("Set Timezone ") + parameter.getTimezone() + F(" (Code = ") + tz + F(")"));
   setTZ(tz.c_str());
   thTime.setNtpServer(parameter.getNtpServer());
   thTime.setUpdateInterval(NTP_UPDATE_INTERVAL);
   thTime.begin();
   checkRTC(); // check RTC time against systemtime
}

void onParameterChanged(){
   setupTime();
}

void setup() {
//   gdbstub_init();
   Serial.begin(115200);
   Serial.setDebugOutput(true);
   Serial.println();
   Serial.print(F("Wordclock ")); DebugLog.println(VERSION);

   LittleFS.begin();

    //Initialize DebugLog
   DebugLog.init();
   
   // create default host name based on MAC-Address
   String hn = String(HOSTNAME_BASE) + String(F("_")) + WiFi.macAddress();
   hn.replace(":", "");
   parameter.setHostNameDefault(hn);
   parameter.registerChangedCallback(onParameterChanged);

   if (!parameter.read()){
      DebugLog.println(F("Failed to read parameter from EEPROM -> using default values"));
   }

   if (parameter.getHostName().isEmpty()){
      parameter.setHostName(hn);
   }

   String hostName = parameter.getHostName();

   Serial.println(String(F("Hostname = ")) + parameter.getHostName());

   checkRTC();
   startBH1750();

   wordclock.setup();
   parameter.parameterChanged(false);


   // Set timeout for AP
   WiFi.persistent(true);
   wifiManager.setConfigPortalTimeout(AP_TIMEOUT);

   // start Wifi Manager
   wifiManager.autoConnect(hostName.c_str());

   strncpy(parameter.SSID, WiFi.SSID().c_str(), sizeof(parameter.SSID));
   strncpy(parameter.password, WiFi.psk().c_str(), sizeof(parameter.password));

   DebugLog.println(String(F("Connected to SSID ")) + String(parameter.SSID));
   DebugLog.println(WiFi.localIP());

   // setup time
   setupTime();

   if (parameter.showIP) wordclock.showIp();

   // setup web service
   webService.setup();

   // setup web interface
   webWordclock.setup();

}

void loop() {
   static uint32_t lastNtpUpdate_ms = millis();

   DBG_STATE(1);


   thTime.loop();
   DBG_STATE(2);
   
   DebugLog.handle();

   DBG_STATE(3);
   
   wordclock.handle();

   DBG_STATE(4);

   webService.handle();
   DBG_STATE(5);

   if ((millis()-lastNtpUpdate_ms) > NTP_UPDATE_INTERVAL){
      lastNtpUpdate_ms = millis();
      checkRTC(); // check RTC time against systemtime

   }

   DBG_STATE(6);

   webWordclock.handle();

   DBG_STATE(7);
}

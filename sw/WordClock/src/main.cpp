#include "wordclock-config.h"   // include at top
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <time.h>
#include <LittleFS.h>
#include <debug.h>
#include "stringex.h"
#include "debuglog.h"
#include "parameter.h"
#include "timehandler.h"
#include "lighthandler.h"
#include "wordclock.h"
#include "webservice.h"
#include "WebWordclock.h"

const char* VERSION = WORDCLOCK_VERSION;
const char* HOSTNAME_BASE = WORDCLOCK_HOSTNAME;

WiFiManager wifiManager;

Parameter parameter;

String timeZone = NTP_TIMEZONE_DEFAULT;
String ntpServer = NTP_SERVER_DEFAULT;

// NTP available flag
TimeHandler thTime(parameter);

// lightmeter
LightHandler lightMeter(parameter); 

// web service object
WebService webService(parameter);

// NeoPixel control object
WordClock wordclock(parameter, NEOPIXEL_NUMPIXELS, WORDCLOCK_COLS, WORDCLOCK_ROWS, NEOPIXEL_PIN);
WebWordclock webWordclock(parameter, webService);

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
   Wire.begin();

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

   lightMeter.begin();

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
   lightMeter.loop();

   DBG_STATE(2);
   
   DebugLog.handle();

   DBG_STATE(3);
   
   wordclock.handle();

   DBG_STATE(4);

   webService.handle();
   DBG_STATE(5);

   if ((millis()-lastNtpUpdate_ms) > NTP_UPDATE_INTERVAL){
      lastNtpUpdate_ms = millis();
   }

   DBG_STATE(6);

   webWordclock.handle();

   DBG_STATE(7);
}

////////////////////////////////////////////////////////////////////////////////////////////
// TimeHandler.cpp
// Class definition of time handler
////////////////////////////////////////////////////////////////////////////////////////////
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <timehandler.h>
#include <debuglog.h>

#define NTP_SERVER_TIMEOUT 16000

// char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

TimeHandler::TimeHandler(Parameter& param)
    : m_parameter(param)
    , m_ntp(m_ntpUdp)
    , m_server(F("")), m_local_time(0)
    , m_autoDST(true), m_timezone_hour(1), m_timezone_minute(0)
    , m_update_interval_ms(60000), m_ntp_available(false)
{
    
}

TimeHandler::~TimeHandler(){

}

void TimeHandler::begin(){
  // ntp.isDST(false);
  // ntp.timeZone(1);
  // ntp.offset(0, 0, 0, 0);
    DebugLog.println(F("Start NTP service "));
    if (m_server.isEmpty()){
        DebugLog.println(F("Start NTP service failed (no NTP server defined)"));
        return;
    }

    if (!WiFi.isConnected() || (WiFi.getMode()== WIFI_AP)){
        DebugLog.println(F("Start NTP service failed (not connected to wifi)"));
        return;
    }

    DebugLog.println(String("Using NTP server: ") + m_server);
    m_ntp.ntpServer(m_server.c_str());

    if (m_autoDST){
        m_ntp.ruleDST("CEST", Last, Sun, Mar, 2, 120); // last sunday in march 2:00, timetone +120min (+1 GMT + 1h summertime offset)
        m_ntp.ruleSTD("CET", Last, Sun, Oct, 3, 60); // last sunday in october 3:00, timezone +60min (+1 GMT)
    } 

    m_ntp.updateInterval(1000);
    m_ntp.begin(false);
    bool bNtpSuccess = false;
    auto timeout = millis();
    while(((millis()-timeout)< NTP_SERVER_TIMEOUT) && !bNtpSuccess){
        bNtpSuccess = m_ntp.update();
    }
    m_ntp_available = bNtpSuccess;
    if (m_ntp_available) m_ntp.updateInterval(m_update_interval_ms);

    String time = String("Time synchronized to ") + m_ntp.formattedTime("%d. %B %Y / %A %T");
    DebugLog.println(time);

#if USE_RTC
    initRTC();
#endif
}

void TimeHandler::loop(){
    if (m_ntp.update()){
#if USE_RTC
        checkRTC();
#endif
    }
}

void TimeHandler::start(){   

}

void TimeHandler::setNtpServer(const String& name){ m_server = name;}
String TimeHandler::getNtpServer() const{ return m_server;}
void TimeHandler::setUpdateInterval(uint32_t interval_ms) { m_update_interval_ms = interval_ms; m_ntp.updateInterval(interval_ms);}
uint32_t TimeHandler::getUpdateInterval() const { return m_update_interval_ms;}

bool TimeHandler::isNtpValid() const { return m_ntp_available; }

uint32_t TimeHandler::uptime_ms(){
    return millis();
}
uint32_t TimeHandler::uptime_us(){
    return micros();
}
uint64_t TimeHandler::uptime_ns(){
    uint64_t time = micros64();
    time *= 1000;
    return time;
}

bool    TimeHandler::isDST(){   return m_ntp.isDST();}
time_t  TimeHandler::epoch(){   return m_ntp.epoch();}
int16_t TimeHandler::year(){    return m_ntp.year();}
int8_t  TimeHandler::month(){   return m_ntp.month();}
int8_t  TimeHandler::day(){ return m_ntp.day();}
int8_t  TimeHandler::weekDay(){ return m_ntp.weekDay();}
int8_t  TimeHandler::hours(){   return m_ntp.hours();}
int8_t  TimeHandler::minutes(){ return m_ntp.minutes();}
int8_t  TimeHandler::seconds()   { return m_ntp.seconds();}
char*   TimeHandler::formattedTime(const char *format){ return m_ntp.formattedTime(format);}
String TimeHandler::formattedTime(const String& format) { return String(m_ntp.formattedTime(format.c_str()));}

bool TimeHandler::initRTC(){
#if USE_RTC
    // Initialize & check RTC
    DebugLog.print("Init RTC ...");
    if (!m_rtcStarted) {
       if (m_rtc.begin()) {
          DebugLog.println("success");
          m_rtcStarted = true;
          if (m_rtc.lostPower()) {
             DebugLog.println("RTC lost power, let's set the time!");
             // When time needs to be set on a new device, or after a power loss, the
             // following line sets the RTC to the date & time this sketch was compiled
             if (isNtpValid()){
                uint32_t t = (uint32_t)epoch();
                m_rtc.adjust(DateTime(t));
             }
             else
                m_rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
          }
       } else {
          DebugLog.println("failed");
          DebugLog.println("Couldn't find RTC");
          m_rtcStarted = false;
       }
    } else {
          DebugLog.println("success (already initialized)");
    }
#else
   m_rtcStarted = false;
#endif
   return m_rtcStarted;
}

bool TimeHandler::checkRTC(){
#if USE_RTC
   // Initialize & check RTC
   DebugLog.println("Check RTC");
   if (!m_rtcStarted) {
       initRTC();
   }

   if (m_rtcStarted){
      DebugLog.println("Checking RTC time");
      if (isNtpValid()){
         DebugLog.println("NTP Server available");
         if (epoch() != m_rtc.now().unixtime()){
            // time differs between system time (NTP) and RTC
            // => adjust RTC
            m_rtc.adjust(DateTime((uint32_t)epoch()));
            DebugLog.println("Adjusting RTC time");
            DebugLog.println(String("RTC Time set to ") + DateTime((uint32_t)epoch()).timestamp());
         }
      }
      else{
         DebugLog.println("NTP server not available - don't touch RTC time");
      }
      return true;
   } else {
      return true;
   }
#else
   return false;
#endif
}
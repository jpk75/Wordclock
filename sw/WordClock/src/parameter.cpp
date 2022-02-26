#include "wordclock-config.h"   // include at top
#include "parameter.h"
#include <Arduino.h>
#include <EEPROM.h>
#include "debuglog.h"

Parameter::Parameter()
{
    initVars();
}
Parameter::~Parameter()
{

}

void Parameter::parameterChanged(bool save){
    checkParameter();

    if (DebugLog.getVerbosity() >= 1){
        DebugLog.println(String(F("Version = ")) + String(version));
        DebugLog.println(String(F("SSID = ")) + String(SSID));
        DebugLog.println(String(F("PSK = ")) + String(password));
        DebugLog.println(String(F("Hostname = ")) + String(hostName));
        DebugLog.println(String(F("Hostname (default) = ")) + String(hostNameDefault));
        DebugLog.println(String(F("red = ")) + String(red));
        DebugLog.println(String(F("green = ")) + String(green));
        DebugLog.println(String(F("blue = ")) + String(blue));
        DebugLog.println(String(F("brightness = ")) + String(brightness));
        DebugLog.println(String(F("date interval = ")) + String(dateInterval));
        DebugLog.println(String(F("showDate = ")) + String(showDate));
        DebugLog.println(String(F("showIP = ")) + String(showIP));
        DebugLog.println(String(F("ntpServer = ")) + String(ntpServer));
        DebugLog.println(String(F("timezone = ")) + String(timeZone));
        DebugLog.println(String(F("dispMode = ")) + String(dispMode));
        DebugLog.println(String(F("brightnessMode = ")) + String(brightnessMode));
        DebugLog.println(String(F("illuminationMin = ")) + String(illuminationMin));
        DebugLog.println(String(F("illuminationMax = ")) + String(illuminationMax));
        DebugLog.println(String(F("colorMode = ")) + String(colorMode));
        DebugLog.println(String(F("colorChangeInterval = ")) + String(colorChangeInterval));
        DebugLog.println(String(F("Display off = ")) + getDisplayTimeOff());
        DebugLog.println(String(F("Display on = ")) + getDisplayTimeOn() );
    }

    if (save) write();

    for(auto fn : aChangedCallback) fn();
}

void Parameter::registerChangedCallback(std::function<void()> callback){
    aChangedCallback.push_back(callback);
}

void Parameter::resetSettings(){
    // reset to defaults
    initVars();
    write();    
}

void Parameter::setHostNameDefault(const String& name) { strncpy(hostNameDefault, name.c_str(), sizeof(hostNameDefault)-1);}
String Parameter::getHostNameDefault() const { return String(hostNameDefault); }
void Parameter::setHostName(const String& name) { strncpy(hostName, name.c_str(), sizeof(hostName)-1);}
String Parameter::getHostName() const { String hn = hostName; return hn.isEmpty() ? String(hostNameDefault) : hn; }
void Parameter::setTimezone(const String& tz) { strncpy(timeZone, tz.c_str(), sizeof(timeZone)-1); }
String Parameter::getTimezone() const { return String(timeZone); }
void Parameter::setNtpServer(const String& server){ strncpy(ntpServer, server.c_str(), sizeof(ntpServer)-1); }
String Parameter::getNtpServer() const{ return String(ntpServer); }
void Parameter::setDisplayTimeOff(const String& time) {
    String split[2]; 
    if (StringEx::StringSplit(time, ':', split, 2) == 2){
//        split[0] = split[0].startsWith("0") ? split[0].substring(1) : split[0];
//        split[1] = split[1].startsWith("0") ? split[1].substring(1) : split[1];

        if (DebugLog.getVerbosity() >= 1) {
            DebugLog.print(time + String(":"));
            DebugLog.print(F("DispTimeOff = "));
            DebugLog.println(split[0] + String(F(":")) + split[1]);
        }

        dispOffHour = (uint8_t)split[0].toInt();
        dispOffMinute = (uint8_t)split[1].toInt();
    }
    else{
        dispOffHour = PARAM_DISP_OFF_HOUR_DEFAULT;
        dispOffMinute = PARAM_DISP_OFF_MINUTE_DEFAULT;
    }
}

String Parameter::getDisplayTimeOff() const { return String(dispOffHour<10 ? "0" : "") + String(dispOffHour) + String(F(":")) + String(dispOffMinute<10 ? "0" : "") + String(dispOffMinute); }
void Parameter::setDisplayTimeOn(const String& time) {
    String split[2]; 
    if (StringEx::StringSplit(time, ':', split, 2) == 2){
//        split[0] = split[0].startsWith("0") ? split[0].substring(1) : split[0];
//        split[1] = split[1].startsWith("0") ? split[1].substring(1) : split[1];
        if (DebugLog.getVerbosity() >= 1){
            DebugLog.print(time + String(":")); 
            DebugLog.print(F("DispTimeON = ")); 
            DebugLog.println(split[0] + String(F(":")) + split[1]);
        }

        dispOnHour = (uint8_t)split[0].toInt();
        dispOnMinute = (uint8_t)split[1].toInt();
    }
    else{
        dispOnHour = PARAM_DISP_ON_HOUR_DEFAULT;
        dispOnMinute = PARAM_DISP_ON_MINUTE_DEFAULT;
    }
}

String Parameter::getDisplayTimeOn() const { return String(dispOnHour<10 ? "0" : "") + String(dispOnHour) + String(F(":")) + String(dispOnMinute<10 ? "0" : "") + String(dispOnMinute); }

bool Parameter::getTestEnable() const{ return test_enable;}
void Parameter::setTestEnable(bool flag){ test_enable = flag; }

bool Parameter::read(){
    uint32_t size = sizeof(Parameter_t);
    if (DebugLog.getVerbosity() >= 1) DebugLog.println(String(F("parameter - read ")) + String(size) + String(F("bytes")));
    
    uint32_t i = 0;
    uint8_t csum = 0;
    EEPROM.begin(size);
    delay(100);

    auto p = (uint8_t*)this;
    *p = EEPROM.read(0);    // read unused value
    for (i=0; i < size; i++){
        *p = EEPROM.read(i);
        if (DebugLog.getVerbosity() >= 2){
            if (*p < 16) DebugLog.print(F("0"));
            DebugLog.print(String(*p, 16)+ String(F(" ")));
            if (0 == ((i+1)%16)) DebugLog.println("");
        }
        if (i < (size-sizeof(checksum))) csum ^= *p;
        p++;
    }
    EEPROM.end();

    if (DebugLog.getVerbosity() >= 2){
        DebugLog.println("");
        DebugLog.println(String(F("csum=")) + String(csum,16) + String(F(", Checksum=")) + String(checksum,16));
    }

    if (csum != checksum){  // if checksum is invalid -> set to default values
        if (DebugLog.getVerbosity() >= 1) DebugLog.println(F("Checksum different - loading default values"));
        initVars();
    }
   
    parameterChanged(false);

    return (csum == checksum);
}
bool Parameter::write(){
    uint32_t size = sizeof(Parameter_t);
    if (DebugLog.getVerbosity() >= 1) DebugLog.println(String(F("parameter - write ")) + String(size) + String(F("bytes")));
    uint32_t i=0;

    version = PARAM_VERSION;    // explicitely set version number

    EEPROM.begin(size);
    delay(100);

    checksum = 0;
    auto p = (uint8_t*)this;
    for (i=0; i < size; i++){
        EEPROM.write(i, *p);
        if (i < (size-sizeof(checksum))) checksum ^= *p;
        if (DebugLog.getVerbosity() >= 2){
            if (*p < 16) DebugLog.print(F("0"));
            DebugLog.print(String(*p, 16)+ String(F(" ")));
            if (0 == ((i+1)%16)) DebugLog.println("");
        }
        p++;
    }

    if (DebugLog.getVerbosity() >= 2){
        DebugLog.println("");
        DebugLog.println(String(F("checksum = ")) + String(checksum,16));
    }

    delay(200);
    EEPROM.commit();

    EEPROM.end();

    return true;
}

void Parameter::initVars(){
    version = PARAM_VERSION;
    memset(SSID, 0, sizeof(SSID));
    memset(password, 0, sizeof(password));
    red = PARAM_COLOR_RED;
    green = PARAM_COLOR_GREEN;
    blue = PARAM_COLOR_BLUE;
    brightness = PARAM_BRIGHTNESS_DEFAULT;
    showDate = PARAM_SHOWDATE_DEFAULT;
    showIP = PARAM_SHOWIP_DEFAULT;
    dateInterval = PARAM_DATE_INTERVAL_DEFAULT;
    dispOnHour = PARAM_DISP_ON_HOUR_DEFAULT;
    dispOnMinute = PARAM_DISP_ON_MINUTE_DEFAULT;
    dispOffHour = PARAM_DISP_ON_HOUR_DEFAULT;
    dispOffMinute = PARAM_DISP_ON_MINUTE_DEFAULT;
    dispMode = PARAM_DISP_MODE_DEFAULT;
    brightnessMode = PARAM_BRIGHTNESS_MODE_DEFAULT;
    illuminationMin = PARAM_BRIGHTNESS_AUTO_ILLUMINATION_MIN;
    illuminationMax = PARAM_BRIGHTNESS_AUTO_ILLUMINATION_MAX;
    colorMode = PARAM_COLOR_MODE_DEFAULT;
    colorChangeInterval = PARAM_COLOR_CHANGE_INTERVAL_DEFAULT;
    strncpy(ntpServer, PARAM_NTPSERVER_DEFAULT, sizeof(ntpServer)-1);
    strncpy(timeZone, PARAM_TIMEZONE_DEFAULT, sizeof(timeZone)-1);
    strncpy(hostName, "", sizeof(hostName)-1);

    checksum = calcChecksum();
    test_enable = false;
}

void Parameter::checkParameter(){
    dispOnHour = std::min(dispOnHour, (uint8_t)23);
    dispOnMinute = std::min(dispOnMinute, (uint8_t)59);
    dispOffHour = std::min(dispOffHour, (uint8_t)23);
    dispOffMinute = std::min(dispOffMinute, (uint8_t)59);
}

int8_t Parameter::calcChecksum(){
    uint32_t size = sizeof(*this) - sizeof(this->checksum);
    int8_t cs = 0;

    auto p = (uint8_t*)this;
    for (uint32_t i=0; i < size; i++){
        cs ^= *p;
        p++;
    }
    return cs;
}
#pragma once
#include "app-config.h"
#include <stringex.h>
#include <cstdint>
#include <vector>
#include <functional>

#define PARAM_VERSION 1
#define PARAM_COLOR_RED 128
#define PARAM_COLOR_GREEN 128
#define PARAM_COLOR_BLUE 128
#define PARAM_BRIGHTNESS_DEFAULT 50
#define PARAM_BRIGHTNESS_MODE_DEFAULT 0
#define PARAM_BRIGHTNESS_AUTO_ILLUMINATION_MIN 10
#define PARAM_BRIGHTNESS_AUTO_ILLUMINATION_MAX 1000
#define PARAM_BRIGHTNESS_AUTO_B_MIN 10
#define PARAM_BRIGHTNESS_AUTO_B_MAX 250
#define PARAM_SHOWDATE_DEFAULT false
#define PARAM_SHOWIP_DEFAULT true
#define PARAM_DATE_INTERVAL_DEFAULT 60
#define PARAM_NTPSERVER_DEFAULT NTP_SERVER_DEFAULT
#define PARAM_TIMEZONE_DEFAULT   NTP_TIMEZONE_DEFAULT
#define PARAM_TIMEZONE_CODE_DEFAULT NTP_TIMEZONE_CODE_DEFAULT
#define PARAM_HOSTNAME_DEFAULT  WORDCLOCK_HOSTNAME
#define PARAM_DISP_ON_HOUR_DEFAULT 0
#define PARAM_DISP_ON_MINUTE_DEFAULT 0
#define PARAM_DISP_OFF_HOUR_DEFAULT 0
#define PARAM_DISP_OFF_MINUTE_DEFAULT 0
#define PARAM_DISP_MODE_DEFAULT 0
#define PARAM_COLOR_MODE_DEFAULT 0
#define PARAM_COLOR_CHANGE_INTERVAL_DEFAULT 60

#pragma pack(push, 1)
using Parameter_t = struct Parameter_st{
    uint8_t version = PARAM_VERSION;     // version of parameter structure
    char    SSID[32];                    // SSID of WiFi
    char    password[32];                // Password of WiFi
    char    hostName[64];                // hostname
    char    ntpServer[64] = PARAM_NTPSERVER_DEFAULT;
    char    timeZone[64] = PARAM_TIMEZONE_DEFAULT;
    uint8_t red = PARAM_COLOR_RED;
    uint8_t green = PARAM_COLOR_GREEN;
    uint8_t blue = PARAM_COLOR_BLUE;
    uint8_t brightness = PARAM_BRIGHTNESS_DEFAULT;
    uint8_t dateInterval = PARAM_DATE_INTERVAL_DEFAULT;
    uint8_t dispOnHour = PARAM_DISP_ON_HOUR_DEFAULT;
    uint8_t dispOnMinute = PARAM_DISP_ON_MINUTE_DEFAULT;
    uint8_t dispOffHour = PARAM_DISP_OFF_HOUR_DEFAULT;
    uint8_t dispOffMinute = PARAM_DISP_OFF_MINUTE_DEFAULT;
    uint8_t dispMode = PARAM_DISP_MODE_DEFAULT;
    uint8_t brightnessMode = PARAM_BRIGHTNESS_MODE_DEFAULT;
    uint16_t illuminationMin = PARAM_BRIGHTNESS_AUTO_ILLUMINATION_MIN;
    uint16_t illuminationMax = PARAM_BRIGHTNESS_AUTO_ILLUMINATION_MAX;
    uint8_t colorMode = PARAM_COLOR_MODE_DEFAULT;
    uint8_t colorChangeInterval = PARAM_COLOR_CHANGE_INTERVAL_DEFAULT;
    bool    showDate = PARAM_SHOWDATE_DEFAULT;
    bool    showIP = PARAM_SHOWIP_DEFAULT;
    uint8_t checksum = 0;
};
#pragma pack(pop)

class Parameter : public Parameter_t
{
    std::vector<std::function<void()>> aChangedCallback;
    bool test_enable = false;
    char hostNameDefault[64];
public:
    enum { COLOR_MODE_SINGLE = 0, COLOR_MODE_RANDOM = 1};
    enum { BRIGHTNESS_MODE_FIXED = 0, BRIGHTNESS_MODE_AUTO = 1};

    Parameter();
    ~Parameter();
    bool read();
    bool write();
    void parameterChanged(bool save);
    void setHostNameDefault(const String& name);
    String getHostNameDefault() const;
    void setHostName(const String& name);
    String getHostName() const;
    void setTimezone(const String& tz);
    String getTimezone() const;
    void setNtpServer(const String& server);
    String getNtpServer() const;
    void setDisplayTimeOff(const String& time);
    String getDisplayTimeOff() const;
    void setDisplayTimeOn(const String& time);
    String getDisplayTimeOn() const;
    bool getTestEnable() const;
    void setTestEnable(bool flag);    
    void registerChangedCallback(std::function<void()> callback);
    void resetSettings();
private:
    void initVars();
    void checkParameter();
    int8_t calcChecksum();
};
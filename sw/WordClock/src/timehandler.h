////////////////////////////////////////////////////////////////////////////////////////////
// TimeHandler.h
// Class definition of time handler
////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <NTP.h>
#include <WiFiUdp.h>
#include <parameter.h>

class TimeHandler
{
public:
    TimeHandler(Parameter& param);
    virtual ~TimeHandler();
    void begin();
    void loop();
    void start();
    
    void setNtpServer(const String& name);
    String getNtpServer() const;
    void setUpdateInterval(uint32_t interval_ms);
    uint32_t getUpdateInterval() const;

    bool isNtpValid() const;
    
    uint32_t uptime_ms();
    uint32_t uptime_us();
    uint64_t uptime_ns();

    /**
     * @brief returns the DST state
     * 
     * @return int 1 if summertime, 0 no summertime
     */
    bool isDST();
	
    /**
     * @brief get the Unix epoch timestamp
     * 
     * @return time_t timestamp
     */
    time_t epoch();

    /**
     * @brief get the year
     * 
     * @return int year
     */
    int16_t year();

    /**
     * @brief get the month
     * 
     * @return int month, 1 = january 
     */
    int8_t month();

    /**
     * @brief get the day of a month
     * 
     * @return int day
     */
    int8_t day();

    /**
     * @brief get the day of a week
     * 
     * @return int day of the week, 0 = sunday
     */
    int8_t weekDay();

    /**
     * @brief get the hour of the day
     * 
     * @return int 
     */
    int8_t hours();

    /**
     * @brief get the minutes of the hour
     * 
     * @return int minutes
     */
    int8_t minutes();

    /**
     * @brief get the seconds of a minute
     * 
     * @return int seconds
     */
    int8_t seconds();    

    /**
     * @brief returns a formatted string
     * 
     * @param format for strftime
     * @return char* formated time string
     */
    char* formattedTime(const char *format);

    String formattedTime(const String& format);
    
private:

    // data member
private:
    Parameter& m_parameter;
    WiFiUDP m_ntpUdp;
    NTP m_ntp;
    String m_server;
    uint64_t m_local_time;
    bool m_autoDST;
    int16_t m_timezone_hour;
    int16_t m_timezone_minute;
    uint32_t m_update_interval_ms;
    bool m_ntp_available;
};
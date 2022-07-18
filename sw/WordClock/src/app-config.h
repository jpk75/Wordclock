#pragma once

#define WORDCLOCK_HOSTNAME "wordclock"
#define WORDCLOCK_VERSION "V0.1.0"
#define WORDCLOCK_COLS 11        // number of letters in a row
#define WORDCLOCK_ROWS 10        // number of rows

#define AP_TIMEOUT 240ul  // Timeout in seconds for AP / WLAN config
#define NTP_SERVER_DEFAULT "de.pool.ntp.org" // NTP server
#define NTP_TIMEZONE_DEFAULT  "Europe/Berlin"
#define NTP_TIMEZONE_CODE_DEFAULT  "CET-1CEST,M3.5.0/02,M10.5.0/03"
#define NTP_UPDATE_INTERVAL 3600000ul    // Update time every 1 hour (3600000ms)  

#define NEOPIXEL_PIN 12           // HW pin the LED stripe ist connected to
#define NEOPIXEL_NUMPIXELS  114  // number of LEDs

// RTC
#define USE_RTC 1
#define I2C_ADR_RTC 0x68            // I2C adress of the RTC  DS3231 (Chip on ZS-042 Board)

// Lightsensor
#define USE_LIGHTSENSOR 1
#define I2C_ADR_BH1750 0x23

// Features
#define CALC_SINELUT 0      // disable calulation of sine LUT
//#define CALC_SINELUT 1      // enable calulation of sine LUT

// MQTT
#define USE_MQTT 0
#define MQTT_SERVER "192.168.10.7"
#define MQTT_PORT   1883
#define MQTT_ALIVE_INTERVAL 300000      // MQTT alive message every 5 minutes 
#define MQTT_CLIENT WORDCLOCK_HOSTNAME
#define MQTT_GROUP  "wordclock"
#define MQTT_TOPIC  "${group}/${client}"

// Debugging
#define USE_DBGSTATE 0
#define USE_SERIAL 1
#define USE_TELNET 0

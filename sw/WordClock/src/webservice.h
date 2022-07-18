#pragma once
#include "app-config.h"   // include at top
#include <ESP8266WebServer.h>
#if USE_MQTT
    #include "mqtthandler.h"
#endif

#include "parameter.h"

class WebService
{
private:
    Parameter& parameter;
    ESP8266WebServer webServer;
#if USE_MQTT
    MqttHandler mqtt;
#endif
public:
    WebService(Parameter& param);
    ~WebService();
    void setup();
    void handle();
#if USE_MQTT
    MqttHandler* getMqttHandler();
#endif
    ESP8266WebServer* getWebServer();
    String getHostname() const;
    String getMacAddress() const;
    String getIPAddress() const;    
private:
    void setupHttpUpdate();
};
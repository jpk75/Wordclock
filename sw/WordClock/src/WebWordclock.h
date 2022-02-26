#pragma once
#include "wordclock-config.h"   // include at top
#include <ESP8266WebServer.h>
#include "parameter.h"
#include "webservice.h"

class WebWordclock
{
private:
    Parameter& parameter;
    ESP8266WebServer& webServer;
    static const String mimeHTML; 
    static const String mimeJSON; 
    static const String mimePlain;
    static const String mimeCSS;  
    static const String mimeJS;   
public:
    WebWordclock(Parameter& param, WebService& ws);
    ~WebWordclock();
    void setup();
    void handle();

private:
    void httpMainPage();
    void httpNotFound();
    void handleSubmit();
    void handleJs();
    void handleJson();
    void handleZones();
};
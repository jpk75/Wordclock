#include <Arduino.h>
#include "webservice.h"
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPUpdateServer.h>
#include "debuglog.h"

#define ABS(x,y) ((x)>(y)) ? ((x)-(y)) : ((y)-(x))

WebService::WebService(Parameter& param)
    : parameter(param)
    , webServer(80)
#if USE_MQTT
    , mqtt(param)
#endif
{

}

WebService::~WebService(){

}

void WebService::setup(){
    String hostName = parameter.getHostName();

    // configure MDNS service
    MDNS.begin(hostName);
    MDNS.addService("http", "tcp", 80);

    //configure OTA
    ArduinoOTA.setHostname(hostName.c_str());
    ArduinoOTA.onStart([]() {
        String type;
        DebugLog.stop();
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = F("sketch");
        } else { // U_FS
            type = F("filesystem");
        }

        // NOTE: if updating FS this would be the place to unmount FS using FS.end()
        DebugLog.println(F("Start updating ") + type);
    });
    ArduinoOTA.onEnd([]() {
        DebugLog.println(F("\nEnd"));
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        DebugLog.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        DebugLog.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            DebugLog.println(F("Auth Failed"));
        } else if (error == OTA_BEGIN_ERROR) {
            DebugLog.println(F("Begin Failed"));
        } else if (error == OTA_CONNECT_ERROR) {
            DebugLog.println(F("Connect Failed"));
        } else if (error == OTA_RECEIVE_ERROR) {
            DebugLog.println(F("Receive Failed"));
        } else if (error == OTA_END_ERROR) {
            DebugLog.println(F("End Failed"));
        }
    });

    ArduinoOTA.begin();

    // configure OTA web update server
    setupHttpUpdate();

    // setup MQTT
#if USE_MQTT
    mqtt.begin();
    mqtt.start();
#endif

}

void WebService::handle(){
    MDNS.update();
    ArduinoOTA.handle();
    webServer.handleClient();
#if USE_MQTT
    mqtt.loop();
#endif
}

#if USE_MQTT
MqttHandler* WebService::getMqttHandler(){
    return &mqtt;
}
#endif

ESP8266WebServer* WebService::getWebServer(){
    return &webServer;
}

String WebService::getHostname() const { return WiFi.hostname();}
String WebService::getMacAddress() const { return WiFi.macAddress(); }
String WebService::getIPAddress() const { return WiFi.localIP().toString();}

void WebService::setupHttpUpdate(){
    webServer.on("/update", HTTP_GET, 
    [&]() {
      webServer.sendHeader("Connection", "close");
      webServer.send(200, "text/html", "<form method='POST' action='/ota' enctype='multipart/form-data'><input type='file' name='update'><input type='submit' value='Update'></form>");
    });
    webServer.on("/ota", HTTP_POST, 
    [&]() {
      webServer.sendHeader("Connection", "close");
      webServer.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
      ESP.restart();
    },
    [&]() {
      HTTPUpload& upload = webServer.upload();
      if (upload.status == UPLOAD_FILE_START) {
        Serial.setDebugOutput(true);
        WiFiUDP::stopAll();
        DebugLog.printf("Update: %s\n", upload.filename.c_str());
        uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace)) { //start with max available size
          Update.printError(DebugLog);
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Update.printError(DebugLog);
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        if (Update.end(true)) { //true to set the size to the current progress
          DebugLog.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        } else {
          Update.printError(DebugLog);
        }
      }
      yield();
    });
}

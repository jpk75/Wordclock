#include <Arduino.h>
#include <LittleFS.h>
#include "timehandler.h"
#include "lighthandler.h"
#include "webwordclock.h"
#include "debuglog.h"

//#define TZ_WEB_SITE        "http://www.hs-help.net/hshelp/gira/v4_7/de/proj_tz.html"
#define TZ_WEB_SITE        "https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv"

const char mimeHTML[] PROGMEM = "text/html";
const char mimeJSON[] PROGMEM = "application/json";
const char mimePlain[] PROGMEM = "text/plain";
const char mimeCSS[] PROGMEM = "text/css";
const char mimeJS[] PROGMEM = "application/javascript";

// static methods + member initialization
const String WebWordclock::mimeHTML = mimeHTML;
const String WebWordclock::mimeJSON = mimeJSON;
const String WebWordclock::mimePlain= mimePlain;
const String WebWordclock::mimeCSS  = mimeCSS;
const String WebWordclock::mimeJS   = mimeJS;

//extern RTC_DS3231 rtc;
extern TimeHandler thTime;
extern LightHandler lightMeter;

WebWordclock::WebWordclock(Parameter& param, WebService& ws)
    : parameter(param)
    , webServer(*ws.getWebServer())
{
}

WebWordclock::~WebWordclock()
{

}

void WebWordclock::setup(){
    LittleFS.begin();

    webServer.onNotFound([&]{ this->httpNotFound(); });
    webServer.on(F("/"), [&]{ this->httpMainPage(); });
    webServer.on(F("/j.js"), HTTP_GET, [&]{ this->handleJs(); });  // handle javascript request
    webServer.on(F("/json"), [&]{ this->handleJson(); }); // send data in JSON format
    webServer.on(F("/setWC.php"), HTTP_GET, [&]{ this->handleSubmit(); });
    webServer.on(F("/readParameter"), HTTP_GET, [&]{ this->parameter.read(); });

    webServer.serveStatic("/zones", LittleFS, "/zones.json");
    webServer.begin();
}

void WebWordclock::handle(){
}

void WebWordclock::httpMainPage(){
    String strContent;

    // Display the HTML web page
    strContent += F("<!DOCTYPE html><html>\n");
    strContent += F("<head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n");
    strContent += F("<link rel=\"icon\" href=\"data:,\">\n");
    // CSS to style the on/off buttons
    // Feel free to change the background-color and font-size attributes to fit your preferences
    strContent += F("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n");
    strContent += F(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;\n");
    strContent += F("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}\n");
    strContent += F(".button2 {background-color: #77878A;}\n");
    strContent += F(".dimmed {color:dimgray;}</style>\n");
    strContent += F("<script src='j.js'></script>\n");
    strContent += F("</head>\n");

    // Web Page Heading
    strContent += F("<body><h1>Word Clock Konfiguration</h1>\n");
    strContent += F("<p>Date: <span class='dimmed' id='date'>1970-01-01</span><br>");
    strContent += F("Time: <span class='dimmed' id='time'>00:00:00</span><br>");
    strContent += F("Illumination Level: <span class='dimmed' id='illumination'>0</span>lx<br>");
    strContent += F("Tempertur: <span class='dimmed' id='temperature'>0</span>°C<br>");
    strContent += F("SSID: <span class='dimmed' id='ssid'></span><br>");
    strContent += F("Signal Strength: <span class='dimmed' id='rssi'></span>dB<br>");
    strContent += F("</p>\n"); 

    strContent += F("<form action=\"/setWC.php\">\n");

    // Convert color into hex settings
    char hex[8] = {};
    sprintf(hex,"#%02X%02X%02X",parameter.red,parameter.green,parameter.blue);

    strContent += F("<br><br><h2>Allgemein</h2><br>\n");
    strContent += F("<label for='showip'>IP Adresse beim Start anzeigen?</label>");
    strContent += F("<input type='checkbox' id='showip' name='showip' value='on'");
    if (parameter.showIP)
        strContent += F(" checked");
    strContent += F("><br><br>\n");
    strContent += F("<label for='showdate'>Datum anzeigen?</label>");
    strContent += F("<input type='checkbox' id='showdate' name='showdate' value='on'");
    if (parameter.showDate)
        strContent += F(" checked");
    strContent += F("><br><br>\n");
    strContent += F("<label for='dateinterval'>Datum Anzeigeintervall [<span class='dimmed' id='disp_dateinterval' name='disp_dateinterval'>");
    strContent += parameter.dateInterval;
    strContent += F("</span>s]<br></label>");
    strContent += F("<input type='range' id='dateinterval' name='dateinterval'  min='10' max='255' value='");
    strContent += parameter.dateInterval;
    strContent += F("' onchange=\"(function(){document.getElementById('disp_dateinterval').innerHTML = event.target.value;})()\"><br><br>\n");
    // strContent += F("<label for='debug_verbosity'>Verbosity</label>\n");
    // strContent += F("<input type='number' id='debug_verbosity' name='debug_verbosity' min='0' max='5' value='");
    // strContent += DebugLog.getVerbosity();
    // strContent += F("'><br><br>\n");

    strContent += F("<br><br><h2>Zeitzone &amp; NTP-Server</h2><br>\n");

    strContent += F("<label for='ntpserver'>NTP Server</label>\n");
    strContent += F("<input type='text' id='ntpserver' name='ntpserver' size='45' value='");
    strContent += parameter.ntpServer;
    strContent += F("'><br>\n");

    strContent += F("<br><label for='timezone'>Timezone</label>\n");
    strContent += F("<select id='timezone' name='timezone'>\n<option selected value='");
    strContent += parameter.timeZone;
    strContent += F("'>");
    strContent += parameter.timeZone;
    strContent += F("</option>\n");
    strContent += F("</select><br>\n");

//    strContent += F("<input type='text' id='timezone' name='timezone' size='45' value='");
//    strContent += parameter.timeZone;
//    strContent += F("'><br>\n");

    strContent += F("<br>Default: \n");
    strContent += F(PARAM_TIMEZONE_DEFAULT);
    strContent += F("<br>\n");

    strContent += F("<br><a href='");
    strContent += F(TZ_WEB_SITE) + String(F("'"));
    strContent += F(" target='_blank'>Liste mit Einstellung der Zeitzone</a><br>\n");

    // strContent += F("<label for='curdate'>Date</label>\n");
    // strContent += F("<input type='date' id='curdate' name='curdate' value=''><br>\n");
    // strContent += F("<label for='curtime'>Time</label>\n");
    // strContent += F("<input type='time' id='curtime' name='curtime' value=''><br>\n");

    strContent += F("<br><br><h2>Anzeige Einstellungen</h2><br>\n");
//    strContent += F("<fieldset><legend>Color Mode</legend>\n");
    strContent += F("<label for='col_uni'>Einzelne Farbe: </label>");
    strContent += F("<input type='radio' id='col_uni' name='colmode' value='0'");
    strContent += (Parameter::COLOR_MODE_SINGLE == parameter.colorMode) ? F(" checked") : F("");
    strContent += F("><br>\n");
    strContent += F("<label for='col_random'>Zuf&auml;llige Farbe: </label>");
    strContent += F("<input type='radio' id='col_random' name='colmode' value='1'");
    strContent += (Parameter::COLOR_MODE_RANDOM == parameter.colorMode) ? F(" checked") : F("");
    strContent += F("><br><br>\n");
    strContent += F("<label for='colchangeinterval'>Farbwechselintervall [<span class='dimmed' id='disp_colchangeinterval' name='disp_colchangeinterval'>");
    strContent += parameter.colorChangeInterval;
    strContent += F("</span>s]<br></label>");
    strContent += F("<input type='range' id='colchangeinterval' name='colchangeinterval'  min='0' max='255' value='");
    strContent += parameter.colorChangeInterval;
    strContent += F("' onchange=\"(function(){document.getElementById('disp_colchangeinterval').innerHTML = event.target.value;})()\"><br><br>\n");

//    strContent += F("</fieldset>\n");

    strContent += F("<label for='favcolor'>Farbe: </label>\n");
    strContent += F("<input type='color' id='favcolor' name='favcolor' value='");
    strContent += String(hex);
    strContent += F("'><br><br>\n");

    strContent += F("<label for='bright_fix'>Feste Helligkeit: </label>");
    strContent += F("<input type='radio' id='bright_fix' name='brightmode' value='0'");
    strContent += (Parameter::BRIGHTNESS_MODE_FIXED == parameter.brightnessMode) ? F(" checked") : F("");
    strContent += F("><br>\n");
    strContent += F("<label for='bright_auto'>Automatische Helligkeit: </label>");
    strContent += F("<input type='radio' id='bright_auto' name='brightmode' value='1'");
    strContent += (Parameter::BRIGHTNESS_MODE_AUTO == parameter.colorMode) ? F(" checked") : F("");
    strContent += F("><br><br>\n");

    strContent += F("<label for='brightness'>Helligkeit (1...255) [<span class='dimmed' id='disp_brightness' name='disp_brightness'>");
    strContent += parameter.brightness;
    strContent += F("</span>]<br></label>");
    strContent += F("<input type='range' id='brightness' name='brightness' min='1' max='255' value='");
    strContent += parameter.brightness;
    strContent += F("' onchange=\"(function(){document.getElementById('disp_brightness').innerHTML = event.target.value;})()\"><br>");
    strContent += F("<label for='illumination'>Illumination (1...65535) [<span class='dimmed' id='disp_illumination' name='disp_illumination'>");
    strContent += parameter.brightness;
    strContent += F("</span>]<br></label>");
    strContent += F("<input type='range' id='illumination' name='illumination' min='1' max='65535' multiple value='");
    strContent += parameter.illuminationMin + String(F(",")) + parameter.illuminationMax;
    strContent += F("' onchange=\"(function(){document.getElementById('disp_illumination').innerHTML = event.target.value;})()\">");
    strContent += F("<br><br>\n");

    strContent += F("<label for='time_on'>Schalte Display ein um </label>\n");
    strContent += F("<input type='time' id='time_on' name='time_on' value='");
    strContent += parameter.getDisplayTimeOn();
    strContent += F("'><br><br>\n");
    strContent += F("<label for='time_off'>Schalte Display aus um </label>\n");
    strContent += F("<input type='time' id='time_off' name='time_off' value='");
    strContent += parameter.getDisplayTimeOff();
    strContent += F("'><br><br>\n");

    strContent += F("<br><br><h2>Wifi Einstellungen</h2><br>\n");
    strContent += F("<label for='wifi_host'>Hostname</label>\n");
    strContent += F("<input type='text' id='wifi_host' name='wifi_host' size='45' value='");
    strContent += parameter.getHostName().c_str();
    strContent += F("'><br><br>\n");
    strContent += F("<label for='wifi_ssid'>SSID</label>\n");
    strContent += F("<input type='text' id='wifi_ssid' name='wifi_ssid' size='45' value='");
    strContent += parameter.SSID;
    strContent += F("'><br><br>\n");
    strContent += F("<label for='wifi_psk'>Passwort</label>\n");
    strContent += F("<input type='password' id='wifi_psk' name='wifi_psk' size='45' value='");
    strContent += parameter.password;
    strContent += F("'><br>\n");
    strContent += F("<input type='checkbox' id='wifi_showpassword' name='wifi_showpassword'>Passwort anzeigen<br><br>\n");
    
    strContent += F("<br><br><button name='task' id='test' value='test'>LED-Test</button>\n");
    strContent += F("<br><br><button name='task' id='reset' value='reset'>Auf Werkseinstellungen zur&uuml;cksetzen</button>\n");
    strContent += F("<br><br><button name='task' id='update' value='update'>Firmware update</button>\n");
    strContent += F("<br><br><button name='task' id='reboot' value='reboot'>Reboot</button>\n");
    strContent += F("<input type='submit'>\n");
    strContent += F("</form>\n");
    strContent += F("</body></html>\n");


    webServer.send(200, mimeHTML, strContent);
}

void WebWordclock::httpNotFound(){
    webServer.send(404, mimePlain, F("404: Not found"));
}

void WebWordclock::handleSubmit(){
    bool bWifiChanged = false;
    webServer.send(204, "");

    for (auto i=0; i < webServer.args(); i++)
    {
        DebugLog.println(webServer.argName(i) + String(" = ") + webServer.arg(i));
    }

    // take color
    if (webServer.hasArg(F("favcolor"))){
        String col = webServer.arg(F("favcolor"));
        if (col.startsWith("#") && col.length() == 7){
            String sRed = col.substring(1,3);
            String sGreen = col.substring(3,5);
            String sBlue = col.substring(5,7);
            DebugLog.println(String(F("Red = ")) + sRed + String(F(", Green = ")) + sGreen + String(F(", Blue = ")) + sBlue);
            parameter.red   = (uint8_t)strtol(sRed.c_str(), nullptr, 16);
            parameter.green = (uint8_t)strtol(sGreen.c_str(), nullptr, 16);
            parameter.blue  = (uint8_t)strtol(sBlue.c_str(), nullptr, 16);
        }
    }
    // take brightness
    if (webServer.hasArg(F("brightness"))){
        parameter.brightness = (uint8_t)webServer.arg(F("brightness")).toInt();
    }
    // take date interval
    if (webServer.hasArg(F("dateinterval"))){
        parameter.dateInterval = (uint8_t)webServer.arg(F("dateinterval")).toInt();
    }
    // take display on time
    if (webServer.hasArg(F("time_on"))){
        parameter.setDisplayTimeOn(webServer.arg(F("time_on")));
    }
    // take display off time
    if (webServer.hasArg(F("time_off"))){
        parameter.setDisplayTimeOff(webServer.arg(F("time_off")));
    }

    // take display color mode
    if (webServer.hasArg(F("colmode"))){
        parameter.colorMode = (uint8_t)webServer.arg(F("colmode")).toInt();
    }
    // take color change interval
    if (webServer.hasArg(F("colchangeinterval"))){
        parameter.colorChangeInterval = (uint8_t)webServer.arg(F("colchangeinterval")).toInt();
    }

    // take show ip
    parameter.showIP = false;
    if (webServer.hasArg(F("showip"))){
        parameter.showIP = (webServer.arg(F("showip"))=="on") ? true : false;
    }
    // take show date
    parameter.showDate = false;
    if (webServer.hasArg(F("showdate"))){
        parameter.showDate = (webServer.arg(F("showdate"))=="on") ? true : false;
    }
    // take NTP server
    if (webServer.hasArg(F("ntpserver"))){
        strncpy(parameter.ntpServer, webServer.arg(F("ntpserver")).c_str(), sizeof(parameter.ntpServer)-1);
    }
    // take timezone
    if (webServer.hasArg(F("timezone"))){
        strncpy(parameter.timeZone, webServer.arg(F("timezone")).c_str(), sizeof(parameter.timeZone)-1);
    }

    // take verbosity
    if (webServer.hasArg(F("debug_verbosity"))){
        DebugLog.setVerbosity(webServer.arg(F("debug_verbosity")).toInt());
    }


    // take hostname
    if (webServer.hasArg(F("wifi_host"))){
        parameter.setHostName(webServer.arg(F("wifi_host")));
    }

    // take Wifi SSID
    if (webServer.hasArg(F("wifi_ssid"))){
        String ssid = parameter.SSID;
        if (webServer.arg(F("wifi_ssid")) != ssid){
            strncpy(parameter.SSID, webServer.arg(F("wifi_ssid")).c_str(), sizeof(parameter.SSID)-1);
            bWifiChanged = true;
        }
    }

    // take Wifi PSK
    if (webServer.hasArg(F("wifi_psk"))){
        String psk = parameter.password;
        if (webServer.arg(F("wifi_psk")) != psk){
            strncpy(parameter.password, webServer.arg(F("wifi_psk")).c_str(), sizeof(parameter.password)-1);
            bWifiChanged = true;
        }
    }

    // handle tasks
    if (webServer.hasArg(F("task"))){
        if (String(F("reboot")) == webServer.arg(F("task"))) ESP.restart();
        else if (String(F("reset")) == webServer.arg(F("task"))){
            parameter.resetSettings();
            WiFi.disconnect(true);
            delay(1000);
            ESP.restart();
        }
        else if (String(F("test")) == webServer.arg(F("task"))){
            parameter.setTestEnable(parameter.getTestEnable() ? false : true); // enable/ disable test flag
        }
    }
    
    parameter.parameterChanged(true);

    if (bWifiChanged){
        // reconnect to changed WLAN
        DebugLog.print("Try to connect to WLAN '"); DebugLog.print(parameter.SSID); DebugLog.println("'");

        WiFi.disconnect();
        WiFi.setHostname(parameter.getHostName().c_str());
        WiFi.begin(parameter.SSID, parameter.password);
    }
}

void WebWordclock::handleJs() {
    // Output: a fetch API / JavaScript

    String message;
    message += F("const url ='json';\n"
                "const zones ='zones';\n"
                "function renew(){\n"
                " fetch(url)\n" // Call the fetch function passing the url of the API as a parameter
                " .then(response => {return response.json();})\n"
                " .then(jo => {\n"
                "   for (var i in jo)\n"
                "    {if (document.getElementById(i)) document.getElementById(i).innerHTML = jo[i];}\n"    
                " })\n"
                " .catch(function() {\n"            
                " });\n"
                "}\n"
                "\n"
                "function onLoad(){\n"
                " document.getElementById('wifi_showpassword').addEventListener('click', showPassword);\n"
                " document.getElementById('reset').addEventListener('click', function(){ setTimeout(reloadPage, 2000);});\n"
                " document.getElementById('update').addEventListener('click', function(){ \nwindow.open(window.location.href + 'update');\n });\n\n"
                " fetch(zones)\n"
                " .then(response => {return response.json();})\n"
                " .then(jo => {\n"
                "   var cmbZones = document.getElementById('timezone');\n"
                "   var zonesOptions = cmbZones.options;\n"
                "   var selOpt = \"Europe/Berlin\"\n"
                "   if (zonesOptions.length > 0){\n"
                "     selOpt = zonesOptions[0].value;\n"
                "   }\n"
                "   for(var i in jo){\n"
                "     if (i === selOpt) continue;\n"
                "     var optn = document.createElement('OPTION');\n"
                "     optn.value = i;\n"
                "     optn.text = i;\n"
                "     cmbZones.options.add(optn);\n"
                "   }\n"
                " })\n"
                "}\n"
                "function reloadPage(){\n"
                "  window.location.reload();\n"
                "}\n"
                "function showPassword() {\n"
                "  var x = document.getElementById('wifi_psk');\n"
                "  if (x.type === 'password') {\n"
                "     x.type = 'text';\n"
                "  } else {\n"
                "     x.type = 'password';\n"
                "  }\n"
                "}\n\n"
                "document.addEventListener('DOMContentLoaded', onLoad);\n"
                "document.addEventListener('DOMContentLoaded', renew, setInterval(renew, "
            );
    message += 1000;
    message += F("));");
                
    webServer.send(200, F("text/javascript"), message);
}

void WebWordclock::handleJson() {
    // Output: send data to browser as JSON
    time_t now = thTime.epoch();
    struct tm curTime = {};
    localtime_r(&now, &curTime);
    DateTime dt(curTime.tm_year+1900, curTime.tm_mon+1,curTime.tm_mday, curTime.tm_hour, curTime.tm_min, curTime.tm_sec);

    String message = "";
    message = (F("{"));
    message += (F("\"date\":"));
    message += String(F("\"")) + dt.timestamp(DateTime::TIMESTAMP_DATE) + String(F("\""));
    message += (F(",\"time\":"));
    message += String(F("\"")) + dt.timestamp(DateTime::TIMESTAMP_TIME) + String(F("\""));
    message += (F(",\"illumination\":"));
    message += String(lightMeter.readLightLevel());
    message += (F(",\"temperature\":"));
    message += String(0); //rtc.getTemperature());
    message += (F(",\"ssid\":"));
    message += String(F("\"")) + String(WiFi.SSID()) + String(F("\""));
    message += (F(",\"rssi\":"));
    message += String(WiFi.RSSI());
    message += (F("}")); // End of JSON
    webServer.send(200, F("application/json"), message);
}

void WebWordclock::handleZones(){

}
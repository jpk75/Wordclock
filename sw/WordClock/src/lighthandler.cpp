////////////////////////////////////////////////////////////////////////////////////////////
// TimeHandler.cpp
// Class definition of time handler
////////////////////////////////////////////////////////////////////////////////////////////
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <lighthandler.h>
#include <debuglog.h>

#define LH_CHECK_INTERVAL_MS 1000l
LightHandler::LightHandler(Parameter& param)
    : m_parameter(param)
    , m_lightMeter(I2C_ADR_BH1750)
    , m_bh1750Started(false)
    , m_lastUpdate(0)
{
    
}

LightHandler::~LightHandler(){

}

void LightHandler::begin(){
   DebugLog.print(F("Start light service "));
   if (initBH1750())
      DebugLog.println(F("... success"));
   else
      DebugLog.println(F("... failed (init BH1750)"));
 }

void LightHandler::loop(){
   if ((millis() - m_lastUpdate >= LH_CHECK_INTERVAL_MS) || m_lastUpdate == 0) {
      m_lastUpdate = millis();
      if (!m_bh1750Started) begin();
   }
}
void LightHandler::start(){}
bool LightHandler::initBH1750(){
#if USE_LIGHTSENSOR
   if (!m_bh1750Started){
      if (m_lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, I2C_ADR_BH1750)) {
         m_bh1750Started = true;
      }
      else {
         m_bh1750Started = false;
      }
   }
   return m_bh1750Started;
#else
   return false;
#endif   
}
float LightHandler::readLightLevel(){
#if USE_LIGHTSENSOR
   return m_bh1750Started ? m_lightMeter.readLightLevel() : -1.f;
#else
   return -1.f;
#endif
}


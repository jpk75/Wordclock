////////////////////////////////////////////////////////////////////////////////////////////
// TimeHandler.h
// Class definition of time handler
////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include "app-config.h"
#include <BH1750.h>
#include <parameter.h>
#if USE_LIGHTSENSOR
    #include "RTClib.h"
#endif

class LightHandler
{
public:
    LightHandler(Parameter& param);
    virtual ~LightHandler();
    void begin();
    void loop();
    void start();
    float readLightLevel();

private:
    bool initBH1750();
    
    // data member
private:
    Parameter& m_parameter;
    BH1750 m_lightMeter;
    bool m_bh1750Started;
    uint32_t m_lastUpdate;
};
#include <Arduino.h>
#include "debuglog.h"
#include "telnethandler.h"

#if defined(USE_SERIAL) && USE_SERIAL
#define DBGLOG_DEVICE Serial
#else
#define DBGLOG_DEVICE Telnet
#endif

TelnetHandler Telnet;

// Debugging
cDebugLog::cDebugLog(){
    dbgState = -1;
    verbosity = 0;
}

cDebugLog::~cDebugLog(){

}

void cDebugLog::init(){
#if USE_DBGSTATE
    timer1Start(500000, onTimerISR, false);
#endif

#if USE_TELNET
    Telnet.begin();
#endif
}

void cDebugLog::stop(){
#if USE_DBGSTATE
    timer1Stop();
#endif
}
void cDebugLog::handle(){
#if USE_TELNET
    Telnet.loop();
#endif
}
int cDebugLog::available() { return DBGLOG_DEVICE.available();}
int cDebugLog::peek() {return DBGLOG_DEVICE.peek();}
bool cDebugLog::hasPeekBufferAPI() const { return DBGLOG_DEVICE.hasPeekBufferAPI();}
const char* cDebugLog::peekBuffer() { return DBGLOG_DEVICE.peekBuffer();}
size_t cDebugLog::peekAvailable() { return DBGLOG_DEVICE.peekAvailable();}
void cDebugLog::peekConsume(size_t consume) { DBGLOG_DEVICE.peekConsume(consume);}
int cDebugLog::read() {return  DBGLOG_DEVICE.read();}
int cDebugLog::read(uint8_t* buffer, size_t size) { return DBGLOG_DEVICE.read(buffer, size); }
size_t cDebugLog::readBytes(char* buffer, size_t size) { return DBGLOG_DEVICE.readBytes(buffer, size);}
size_t cDebugLog::readBytes(uint8_t* buffer, size_t size) { return DBGLOG_DEVICE.readBytes(buffer, size);}
int cDebugLog::availableForWrite(void) { return DBGLOG_DEVICE.availableForWrite();}
void cDebugLog::flush(void) { DBGLOG_DEVICE.flush();}
size_t cDebugLog::write(uint8_t c){ return DBGLOG_DEVICE.write(c);}
size_t cDebugLog::write(const uint8_t *buffer, size_t size) { return DBGLOG_DEVICE.write(buffer, size);}

int32_t cDebugLog::getVerbosity(){ return verbosity;}
void cDebugLog::setVerbosity(int32_t level) { verbosity = level;}

void cDebugLog::onTimer(){
   char szMsg[20] = {};
   char szVal[10] = {};
   strcat(szMsg, "Debug State = ");
   strcat(szMsg, itoa(dbgState, szVal, 10));
   println(szMsg);
   ESP.wdtFeed();   // feed the watchdog
}

void IRAM_ATTR cDebugLog::onTimerISR(){
    DebugLog.onTimer();
}

void cDebugLog::timer1Start(uint32_t interval_ms, timercallback callback, bool single){
   timer1_attachInterrupt(callback);
   timer1_enable(TIM_DIV256, TIM_EDGE, (single) ? TIM_SINGLE : TIM_LOOP);
   timer1_write(interval_ms);
}
void cDebugLog::timer1Stop(){
   timer1_disable();
}

cDebugLog DebugLog;

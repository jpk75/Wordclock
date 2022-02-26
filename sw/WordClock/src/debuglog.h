#pragma once
#include "wordclock-config.h"
#include <Stream.h>

#if defined(USE_DBGSTATE) && USE_DBGSTATE
#define DBG_STATE(x) DebugLog.dbgState = (x); 
#else
#define DBG_STATE(x)
#endif

class cDebugLog : public Stream
{
public:
    int32_t dbgState;
    int32_t verbosity;
public:
    cDebugLog();
    virtual ~cDebugLog();
    void init();
    void stop();
    void handle();
    
    int available() override;
    int peek() override;
    bool hasPeekBufferAPI() const override;
    const char* peekBuffer() override;
    size_t peekAvailable() override;
    void peekConsume(size_t consume) override;
    int read() override;
    int read(uint8_t* buffer, size_t size) override;
    size_t readBytes(char* buffer, size_t size) override;
    size_t readBytes(uint8_t* buffer, size_t size) override;
    int availableForWrite(void) override;
    void flush(void) override;
    size_t write(uint8_t c) override;
    size_t write(const uint8_t *buffer, size_t size) override;

    int32_t getVerbosity();
    void setVerbosity(int32_t level);
private:
    void timer1Start(uint32_t interval_ms, timercallback callback, bool single);
    void timer1Stop();
    void onTimer();
    static void IRAM_ATTR onTimerISR();
};

extern cDebugLog DebugLog;
////////////////////////////////////////////////////////////////////////////////////////////
// TelnetHandler.h
// Class definition of telnet handler
////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>

class TelnetHandler : public Stream
{
    // data member
private:
    WiFiServer m_TelnetServer;
    WiFiClient m_TelnetClient;
    bool m_ConnectionEstablished;
   
public:
    TelnetHandler();
    virtual ~TelnetHandler();
    void begin();
    void loop();
    void start();
    
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
    using Print::write; // Import other write() methods to support things like write(0) properly

    static TelnetHandler& instance();
    
private:
};
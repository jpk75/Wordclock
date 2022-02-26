////////////////////////////////////////////////////////////////////////////////////////////
// TelnetHandler.cpp
// Class definition of telnet handler
////////////////////////////////////////////////////////////////////////////////////////////
#include <Arduino.h>
#include <telnethandler.h>

// static methods + member initialization
TelnetHandler& TelnetHandler::instance() { static TelnetHandler i; return i; }

TelnetHandler::TelnetHandler()
    : m_TelnetServer(23)
    , m_ConnectionEstablished(false)
{
}

TelnetHandler::~TelnetHandler(){
}

void TelnetHandler::begin(){
    m_TelnetServer.begin();
    m_TelnetServer.setNoDelay(true);
}

void TelnetHandler::loop(){
    // Cleanup disconnected session
    if (!m_TelnetClient.connected())
    {
        m_TelnetClient.stop();
    }

    // Check new client connections
    if (m_TelnetServer.hasClient())
    {
        m_ConnectionEstablished = false; // Set to false

        // m_stream.print("Checking telnet session "); m_stream.println(i+1);
        
        // find free socket
        if (!m_TelnetClient)
        {
            m_TelnetClient = m_TelnetServer.available(); 
                            
            m_TelnetClient.flush();  // clear input buffer, else you get strange characters
            m_TelnetClient.println("Welcome!");
            
            m_TelnetClient.print("Millis since start: ");
            m_TelnetClient.println(millis());
            
//            m_TelnetClient.print("Free Heap RAM: ");
//            m_TelnetClient.println(ESP.getFreeHeap());

            m_TelnetClient.println("----------------------------------------------------------------");
            
            m_ConnectionEstablished = true; 
            
        }
        else
        {
            // m_stream.println("Session is in use");
        }

    if (m_ConnectionEstablished == false)
    {
        m_TelnetServer.available().stop();
        // TelnetMsg("An other user cannot connect ... MAX_TELNET_CLIENTS limit is reached!");
    }
    }

    if (m_TelnetClient.connected())
    {
        if(m_TelnetClient.available())
        { 
            //get data from the telnet client
            while(m_TelnetClient.available())
            {
//                m_stream->write(m_TelnetClient.read());
            }
        }
    }
}

void TelnetHandler::start(){   
}

int TelnetHandler::available() {
    return m_TelnetClient.available();
}
int TelnetHandler::peek() {
    return m_TelnetClient.peek();
}
bool TelnetHandler::hasPeekBufferAPI() const {
    return m_TelnetClient.hasPeekBufferAPI();
}
const char* TelnetHandler::peekBuffer() {
    return m_TelnetClient.peekBuffer();
}
size_t TelnetHandler::peekAvailable() {
    return m_TelnetClient.peekAvailable();
}
void TelnetHandler::peekConsume(size_t consume) {
    m_TelnetClient.peekConsume(consume);
}

int TelnetHandler::read() {
    return m_TelnetClient.read();
}
int TelnetHandler::read(uint8_t* buffer, size_t size) {
    return m_TelnetClient.read(buffer, size);
}
size_t TelnetHandler::readBytes(char* buffer, size_t size) {
    return m_TelnetClient.readBytes(buffer, size);
}
size_t TelnetHandler::readBytes(uint8_t* buffer, size_t size) {
    return m_TelnetClient.readBytes(buffer, size);
}
int TelnetHandler::availableForWrite(void) {
    return m_TelnetClient.availableForWrite();
}
void TelnetHandler::flush(void) {
    m_TelnetClient.flush();
}

size_t TelnetHandler::write(uint8_t c){
    return m_TelnetClient.write(c);
}

size_t TelnetHandler::write(const uint8_t *buffer, size_t size){
    return m_TelnetClient.write(buffer, size);
}


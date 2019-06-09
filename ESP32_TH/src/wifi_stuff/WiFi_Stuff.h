#ifndef WiFiStuff_h
#define WiFiStuff_h

#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <memory>
#include <string>
#include <esp_wifi.h>
#include "SPIFFS.h"

class WiFiStuff
{
  public:
    WiFiStuff();

    String _sensorID;
    unsigned int _interval;
    int8_t _caloffset;

    bool  startPortal(char const *apName);


    //sets timeout before webserver loop ends and exits even if there has been no setup.
    //useful for devices that failed to connect at some point and got stuck in a webserver loop
    //in seconds setConfigPortalTimeout is a new name for setTimeout
    void          setConfigPortalTimeout(unsigned long seconds);
    void          setTimeout(unsigned long seconds);

    //sets timeout for which to attempt connecting, useful if you get a lot of failed connects
    void          setConnectTimeout(unsigned long seconds);

    void          setDebugOutput(bool debug);

  private:
    std::unique_ptr<DNSServer>        dnsServer;
    std::unique_ptr<WebServer>        wwwServer;

    void          setupPortal();
    const char*   _apName                 = "SENSOR";
    unsigned long _configPortalTimeout    = 0;
    unsigned long _connectTimeout         = 0;
    unsigned long _configPortalStart      = 0;
    void (*_apcallback)(WiFiStuff*)     = NULL;

    IPAddress     _sta_static_ip;
    IPAddress     _sta_static_gw;
    IPAddress     _sta_static_sn;

    String        prepConfigPage();
    void          handleRoot();
    void          handleData();
    void          handleWipe();
    void          handleConfig();
    void          handleConfigSave();
    void          handleNotFound();
    void          handleLog();
    bool          captivePortal();
    bool          configPortalHasTimeout();
    bool          setDateTime(String, String);

    // DNS server
    const byte    DNS_PORT = 53;

    //helpers
    int           getRSSIasQuality(int RSSI);
    bool          isIp(String str);
    String        toStringIp(IPAddress ip);

    bool         connect;
    bool         _debug = true;

    template <typename Generic>
    void          DEBUG_WM(Generic text);
};

#endif

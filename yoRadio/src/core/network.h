#ifndef network_h
#define network_h
#include <WiFi.h>

enum n_Status_e { CONNECTED, SOFT_AP, FAILED, SDREADY };

class MyNetwork {
  public:
    n_Status_e status;
    struct tm timeinfo;
    bool lostPlaying = false, beginReconnect = false;
  public:
    MyNetwork() {};
    void begin();
    /** Drain deferred Wi‑Fi lost/reconnect work (call from main loop). */
    void loop();
    void requestTimeSync(bool withTelnetOutput=false, uint8_t clientId=0);
    void requestWeatherSync();
    void setWifiParams();
    bool wifiBegin(bool silent=false);
  private:
    void raiseSoftAP();
    void _handleLost();
    void _handleReconnected();
    static void WiFiLostConnection(WiFiEvent_t event, WiFiEventInfo_t info);
    static void WiFiReconnected(WiFiEvent_t event, WiFiEventInfo_t info);
    volatile uint8_t _evtLost = 0;
    volatile uint8_t _evtGotIp = 0;
};

extern MyNetwork network;

extern __attribute__((weak)) void network_on_connect();

#endif

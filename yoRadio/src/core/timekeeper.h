#ifndef timekeeper_h
#define timekeeper_h
#pragma once

#include <stdint.h>

void _syncTask(void * pvParameters);
bool _getWeather();

/** Last OpenWeatherMap snapshot for DWIN / other UIs. */
struct WeatherData {
  bool valid = false;
  uint8_t icon = 9;    // 0…9 (01 clear … 50 mist, 9=unknown)
  int16_t temp_c = 0;  // °C, rounded
  int16_t feels_c = 0; // °C, rounded
  uint16_t press = 0;  // mmHg (OWM hPa / 1.333)
  uint16_t press_hpa = 0; // raw hPa from OWM
  uint16_t hum = 0;    // %
  uint16_t wind_ms = 0; // m/s, rounded
  char desc[64] = {0};
};

class TimeKeeper {
  public:
    volatile bool forceWeather;
    volatile bool forceTimeSync;
    volatile bool busy;
    char *weatherBuf;
    WeatherData weather;
  public:
    TimeKeeper();
    bool loop0();
    bool loop1();
    void timeTask();
    void weatherTask();
    void waitAndReturnPlayer(uint8_t time_s);
    void waitAndDo(uint8_t time_s, void (*callback)());
  private:
    uint32_t _returnPlayerTime, _doAfterTime;
    void (*_aftercallback)();
    void (*_watchdogcallback)();
    void _upRSSI();
    void _upSDPos();
    void _upClock();
    void _upScreensaver();
    void _returnPlayer();
    void _doAfterWait();
    void _doWatchDog();
    
};

extern TimeKeeper timekeeper;

#endif

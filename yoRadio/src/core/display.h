#pragma once

#include "common.h"

/** The sole display backend of this yoRadio fork. */
class Display {
 public:
  uint16_t currentPlItem = 1;
  uint16_t numOfNextStation = 0;
  void init(); void loop(); void _start(); bool ready() const;
  void resetQueue(); void putRequest(displayRequestType_e type, int payload = 0);
  displayMode_e mode() const; void mode(displayMode_e value);
  void flip() {} void invert() {} bool deepsleep(); void wakeup(); void setContrast();
  void lock(); void unlock(); uint16_t width() const { return 1024; } uint16_t height() const { return 600; }
};

extern Display display;

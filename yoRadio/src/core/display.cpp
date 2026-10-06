#include "display.h"
#include "../displays/dwin/dwin_display.h"
#include "netserver.h"
#include "timekeeper.h"

#ifndef DWIN_TASK_PRIORITY
  #define DWIN_TASK_PRIORITY 2
#endif
#ifndef DWIN_TASK_CORE_ID
  #define DWIN_TASK_CORE_ID 0
#endif

static void dwinTask(void *) {
  for (;;) {
    dwin.loop();
    timekeeper.loop0();
    netserver.loop();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

Display display;
void Display::init() {
  dwin.begin();
  xTaskCreatePinnedToCore(dwinTask, "DwinTask", DWIN_TASK_STACK_SIZE, nullptr,
                          DWIN_TASK_PRIORITY, nullptr, DWIN_TASK_CORE_ID);
}
void Display::loop() { dwin.loop(); }
void Display::_start() { dwin.start(); }
bool Display::ready() const { return dwin.ready(); }
void Display::resetQueue() { dwin.resetQueue(); }
void Display::putRequest(displayRequestType_e type, int payload) { dwin.putRequest({type, payload}); }
displayMode_e Display::mode() const { return dwin.mode(); }
void Display::mode(displayMode_e value) { dwin.setMode(value); }
bool Display::deepsleep() { dwin.sleep(); return true; }
void Display::wakeup() { dwin.wake(); }
void Display::setContrast() { dwin.setBrightness(); }
void Display::lock() { dwin.lock(); }
void Display::unlock() { dwin.unlock(); }

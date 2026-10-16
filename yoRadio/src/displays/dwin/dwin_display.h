#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include "../../core/common.h"

/** UART-sidecar for DMG10600C070 (DGUS II). Maps VP traffic to yoRadio APIs. */
class DwinDisplay {
 public:
  void begin();
  void loop();
  void start();
  bool ready() const { return _ready; }
  void resetQueue();
  /** Thread-safe: enqueues work for DwinTask (or handles inline on that task). */
  void putRequest(requestParams_t request);
  displayMode_e mode() const { return _mode; }
  void setMode(displayMode_e value);
  void sleep();
  void wake();
  void setBrightness();
  void lock() { _locked = true; }
  void unlock() { _locked = false; }
  /** Dropped UART TX packets since boot (silent overflow counter). */
  uint32_t txOverflowCount() const { return _txOverflow; }
  /** Dropped display requests since boot. */
  uint32_t reqOverflowCount() const { return _reqOverflow; }

 private:
  static constexpr uint8_t kTxDepth = 64;
  static constexpr uint8_t kTxFlushMax = 8;   // packets drained per loop()
  static constexpr uint8_t kPacketMax = 72;
  static constexpr uint8_t kRxMax = 96;
  static constexpr uint8_t kReqQueueDepth = 32;

  struct Packet {
    uint8_t data[kPacketMax];
    uint8_t len;
  };

  HardwareSerial _serial{2};
  Packet _tx[kTxDepth];
  uint8_t _txHead = 0;
  uint8_t _txTail = 0;
  portMUX_TYPE _txMux = portMUX_INITIALIZER_UNLOCKED;
  QueueHandle_t _reqQueue = nullptr;
  TaskHandle_t _ownerTask = nullptr;
  uint32_t _txOverflow = 0;
  uint32_t _reqOverflow = 0;
  uint8_t _rx[kRxMax];
  uint8_t _rxLen = 0;
  bool _ready = false;
  bool _locked = false;
  uint32_t _lastProbeMs = 0;
  displayMode_e _mode = PLAYER;
  int _rxPin = 4;
  int _txPin = 2;
  uint32_t _baud = 115200;
  char _wifiSsid[31] = {0};
  char _wifiPass[41] = {0};

  void flushTx();
  void drainRequests();
  void handleRequest(requestParams_t r);
  bool enqueue(const uint8_t *data, uint8_t len);
  void writeRaw(const uint8_t *data, uint8_t len);
  uint16_t probeOnce(uint32_t waitMs);
  bool tryLink(int rxPin, int txPin, uint32_t baud);
  /** Probe pin/baud combos; true only if panel replied. */
  bool probeLink();
  /** Periodic re-probe while `_ready` is still false. */
  void retryLinkIfNeeded();
  void pollRx();
  void frame(const uint8_t *data, uint8_t len);
  void writeWord(uint16_t vp, uint16_t value);
  /** One DGUS write for VU L+R at contiguous VP 0x50D0 / 0x50D2. */
  void writeVuPair(uint8_t left, uint8_t right);
  /** Ordinary Text Display — data at VP. */
  void writeText(uint16_t vp, const char *text);
  /** Text Display with fixed width (zero-padded), matches DGUS Text Length. */
  void writeTextFixed(uint16_t vp, const char *text, uint8_t width);
  /** DGUS Rolling/Scroll Text — widget VP in Tool, payload at VP+3 (as in src 67B4→67B7). */
  void writeScrollText(uint16_t widgetVp, const char *text);
  void page(uint16_t pageId);
  void refresh();
  void refreshStatus();
  void pushStation();
  void pushTitle();
  /** Write bass/middle/treble/balance to display VPs (Audio page). */
  void pushEq();
  /** Write telnet on/off checkbox VP. */
  void pushTelnet();
  /** Push OpenWeatherMap snapshot to weather VPs (icon 0…9 + numbers + desc). */
  void pushWeather();
  /** Push alarm list rows + editor (when on alarm pages). */
  void pushAlarms();
  /** Lightweight active/snooze banner for Player. */
  void pushAlarmStatus();
  /** Push editor fields for `_editAlarm`. */
  void pushAlarmEditor();
  /** Show conflict / hint on ALARM_ERR (UTF-8 → CP1251). */
  void showAlarmError(const char *msg);
  void openAlarmEditor(uint8_t id);
  static int8_t clampTone(int16_t v);
  static int8_t clampBalance(int16_t v);
  void refreshPlaylist();
  /** First station number shown in the 7-row window. */
  uint16_t playlistWindowBase() const;
  /** Move cursor by delta (±1 or ±7), wrap, rewrite only row texts. */
  void movePlaylist(int16_t delta);
  /** Play absolute station and return to Player page. */
  void playStation(uint16_t num);
  void updatePlayState();
  void syncPlayState();
  void ackCommand();
  bool mapLegacyPlayerKey(uint16_t key, uint16_t *commandOut);
  void command(uint16_t value);
  void value(uint16_t vp, uint16_t value);
  void text(uint16_t vp, const uint8_t *data, uint8_t len);
  void logDiagnosticHint(uint16_t vp, uint16_t raw);
  int8_t _lastPlayState = -1;
  int16_t _lastVuL = -1;
  int16_t _lastVuR = -1;
  /** Pending slider level; suppress VOLUME echo while finger is moving. */
  uint8_t _sliderVol = 0;
  uint32_t _sliderGuardUntil = 0;
  uint32_t _sliderLastSetMs = 0;
  char _lastStation[64] = {0};
  char _lastTitle[64] = {0};
  char _lastClock[12] = {0};
  char _lastDate[12] = {0};
  char _lastWeekday[24] = {0};
  char _lastIp[16] = {0};
  char _lastFormat[8] = {0};
  char _lastMode[8] = {0};
  char _lastSsid[33] = {0};
  int8_t _lastWday = -1;
  uint16_t _lastStationNum = 0;
  bool _metaPushed = false;
  uint8_t _editAlarm = 0;
  int8_t _lastAlarmActive = -1;
};

extern DwinDisplay dwin;

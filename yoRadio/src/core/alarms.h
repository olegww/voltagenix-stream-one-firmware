#pragma once
#include <Arduino.h>

/** Three programmable radio alarms (SPIFFS-backed). */
#define ALARM_COUNT 3
#define ALARM_SNOOZE_MIN 10
#define ALARMS_PATH "/data/alarms.bin"

/**
 * days bitmask follows struct tm::tm_wday:
 * bit0=Sun … bit6=Sat. Weekdays Mon–Fri = 0x3E, Weekend = 0x41, Daily = 0x7F.
 */
struct alarm_t {
  uint8_t enabled;   // 0/1
  uint8_t days;      // bitmask
  uint16_t startMin; // 0…1439
  uint16_t stopMin;  // 0…1439 (may wrap past midnight)
  uint16_t station;  // 1-based playlist index
  uint8_t volume;    // 0…254
};

class AlarmManager {
 public:
  void begin();
  void tick(const struct tm &now);
  bool load();
  bool save();

  const alarm_t &get(uint8_t id) const { return _alarms[id < ALARM_COUNT ? id : 0]; }
  alarm_t *editable(uint8_t id) { return id < ALARM_COUNT ? &_alarms[id] : nullptr; }

  /** Apply field change; if enabling/range conflicts → false and fill err (UTF-8). */
  bool setEnabled(uint8_t id, bool on, char *err, size_t errLen);
  bool setStart(uint8_t id, uint16_t minOfDay, char *err, size_t errLen);
  bool setStop(uint8_t id, uint16_t minOfDay, char *err, size_t errLen);
  bool setStation(uint8_t id, uint16_t station);
  bool setVolume(uint8_t id, uint8_t volume);
  bool setDays(uint8_t id, uint8_t daysMask, char *err, size_t errLen);
  bool setDaysPreset(uint8_t id, uint8_t preset, char *err, size_t errLen); // 0=daily 1=weekdays 2=weekend
  bool toggleDay(uint8_t id, uint8_t wday, char *err, size_t errLen);       // 0=Sun…6=Sat

  bool overlaps(uint8_t a, uint8_t b) const;
  bool wouldConflict(uint8_t id, const alarm_t &candidate, uint8_t *withId = nullptr) const;

  int8_t activeId() const { return _activeId; }
  int8_t snoozeFromId() const { return _snoozeFromId; }
  bool sessionActive() const { return _activeId >= 0; }
  bool snoozePending() const { return _snoozeUntilMs != 0; }
  void dismiss();
  void snooze();

  /** Short days label for list row, e.g. "Пн-Пт" / "СбВс" / "ежедн" / "Пн Ср". */
  static void formatDays(uint8_t days, char *out, size_t outLen);
  static void formatRange(uint16_t startMin, uint16_t stopMin, char *out, size_t outLen);
  static uint16_t clampMinOfDay(int32_t v);
  static void minToHm(uint16_t m, uint8_t &h, uint8_t &min);
  static uint16_t hmToMin(uint8_t h, uint8_t min);

 private:
  alarm_t _alarms[ALARM_COUNT];
  int8_t _activeId = -1;
  int8_t _snoozeFromId = -1;
  bool _firedThisMinute = false;
  int _lastCheckedMinute = -1;
  uint32_t _snoozeUntilMs = 0;
  uint8_t _snoozeVolume = 0;
  uint16_t _snoozeStation = 1;

  void _defaults();
  void _startSession(uint8_t id);
  void _stopSession();
  bool _inWindow(const alarm_t &a, uint16_t nowMin) const;
  bool _dayMatches(uint8_t days, int wday) const;
  bool _rangesOverlap(uint16_t a0, uint16_t a1, uint16_t b0, uint16_t b1) const;
};

extern AlarmManager alarms;

#include "options.h"
#include "alarms.h"
#include "config.h"
#include "player.h"
#include "display.h"
#include "network.h"
#include "telnet.h"
#include <SPIFFS.h>

AlarmManager alarms;

static const uint32_t kMagic = 0x414C4D31; // ALM1

void AlarmManager::_defaults() {
  for (uint8_t i = 0; i < ALARM_COUNT; i++) {
    _alarms[i].enabled = 0;
    _alarms[i].days = 0x3E; // Mon–Fri
    _alarms[i].startMin = (uint16_t)(7 * 60);     // 07:00
    _alarms[i].stopMin = (uint16_t)(7 * 60 + 30); // 07:30
    _alarms[i].station = 1;
    _alarms[i].volume = 80;
  }
}

void AlarmManager::begin() {
  _defaults();
  if (!load()) save();
  BOOTLOG("alarms:\t%d slots loaded", ALARM_COUNT);
}

bool AlarmManager::load() {
  if (!SPIFFS.exists(ALARMS_PATH)) return false;
  File f = SPIFFS.open(ALARMS_PATH, "r");
  if (!f) return false;
  uint32_t magic = 0;
  if (f.read((uint8_t *)&magic, sizeof(magic)) != sizeof(magic) || magic != kMagic) {
    f.close();
    return false;
  }
  size_t n = f.read((uint8_t *)_alarms, sizeof(_alarms));
  f.close();
  return n == sizeof(_alarms);
}

bool AlarmManager::save() {
  File f = SPIFFS.open(ALARMS_PATH, "w");
  if (!f) return false;
  f.write((const uint8_t *)&kMagic, sizeof(kMagic));
  f.write((const uint8_t *)_alarms, sizeof(_alarms));
  f.close();
  return true;
}

uint16_t AlarmManager::clampMinOfDay(int32_t v) {
  if (v < 0) return 0;
  if (v > 1439) return 1439;
  return (uint16_t)v;
}

void AlarmManager::minToHm(uint16_t m, uint8_t &h, uint8_t &min) {
  if (m > 1439) m = 1439;
  h = (uint8_t)(m / 60);
  min = (uint8_t)(m % 60);
}

uint16_t AlarmManager::hmToMin(uint8_t h, uint8_t min) {
  if (h > 23) h = 23;
  if (min > 59) min = 59;
  return (uint16_t)(h * 60 + min);
}

void AlarmManager::formatRange(uint16_t startMin, uint16_t stopMin, char *out, size_t outLen) {
  if (!out || outLen < 12) return;
  uint8_t sh, sm, eh, em;
  minToHm(startMin, sh, sm);
  minToHm(stopMin, eh, em);
  snprintf(out, outLen, "%02u:%02u-%02u:%02u", sh, sm, eh, em);
}

void AlarmManager::formatDays(uint8_t days, char *out, size_t outLen) {
  if (!out || outLen == 0) return;
  days &= 0x7F;
  if (days == 0) {
    strlcpy(out, "нет дней", outLen);
    return;
  }
  if (days == 0x7F) {
    strlcpy(out, "ежедн", outLen);
    return;
  }
  if (days == 0x3E) {
    strlcpy(out, "Пн-Пт", outLen);
    return;
  }
  if (days == 0x41) {
    strlcpy(out, "СбВс", outLen);
    return;
  }
  static const char *names[] = {"Вс", "Пн", "Вт", "Ср", "Чт", "Пт", "Сб"};
  out[0] = 0;
  for (uint8_t i = 0; i < 7; i++) {
    if (!(days & (1u << i))) continue;
    if (out[0]) strlcat(out, " ", outLen);
    strlcat(out, names[i], outLen);
  }
}

bool AlarmManager::_dayMatches(uint8_t days, int wday) const {
  if (wday < 0 || wday > 6) return false;
  return (days & (1u << wday)) != 0;
}

bool AlarmManager::_inWindow(const alarm_t &a, uint16_t nowMin) const {
  if (a.startMin == a.stopMin) return false;
  if (a.startMin < a.stopMin) {
    return nowMin >= a.startMin && nowMin < a.stopMin;
  }
  return nowMin >= a.startMin || nowMin < a.stopMin;
}

bool AlarmManager::_rangesOverlap(uint16_t a0, uint16_t a1, uint16_t b0, uint16_t b1) const {
  auto segs = [](uint16_t s, uint16_t e, uint16_t out[2][2], uint8_t &n) {
    n = 0;
    if (s == e) return;
    if (s < e) {
      out[n][0] = s;
      out[n][1] = e;
      n = 1;
    } else {
      out[0][0] = s;
      out[0][1] = 1440;
      out[1][0] = 0;
      out[1][1] = e;
      n = 2;
    }
  };
  uint16_t as[2][2], bs[2][2];
  uint8_t an = 0, bn = 0;
  segs(a0, a1, as, an);
  segs(b0, b1, bs, bn);
  for (uint8_t i = 0; i < an; i++) {
    for (uint8_t j = 0; j < bn; j++) {
      if (as[i][0] < bs[j][1] && bs[j][0] < as[i][1]) return true;
    }
  }
  return false;
}

bool AlarmManager::overlaps(uint8_t a, uint8_t b) const {
  if (a >= ALARM_COUNT || b >= ALARM_COUNT || a == b) return false;
  const alarm_t &A = _alarms[a];
  const alarm_t &B = _alarms[b];
  if (!(A.days & B.days)) return false;
  return _rangesOverlap(A.startMin, A.stopMin, B.startMin, B.stopMin);
}

bool AlarmManager::wouldConflict(uint8_t id, const alarm_t &candidate, uint8_t *withId) const {
  if (!candidate.enabled) return false;
  for (uint8_t i = 0; i < ALARM_COUNT; i++) {
    if (i == id || !_alarms[i].enabled) continue;
    if (!(candidate.days & _alarms[i].days)) continue;
    if (_rangesOverlap(candidate.startMin, candidate.stopMin, _alarms[i].startMin, _alarms[i].stopMin)) {
      if (withId) *withId = i;
      return true;
    }
  }
  return false;
}

static void conflictMsg(char *err, size_t errLen, uint8_t with) {
  if (err && errLen) snprintf(err, errLen, "Конфликт с №%u", (unsigned)(with + 1));
}

bool AlarmManager::setEnabled(uint8_t id, bool on, char *err, size_t errLen) {
  alarm_t *a = editable(id);
  if (!a) return false;
  alarm_t tmp = *a;
  tmp.enabled = on ? 1 : 0;
  uint8_t with = 0;
  if (on && wouldConflict(id, tmp, &with)) {
    conflictMsg(err, errLen, with);
    return false;
  }
  if (err && errLen) err[0] = 0;
  a->enabled = tmp.enabled;
  save();
  return true;
}

bool AlarmManager::setStart(uint8_t id, uint16_t minOfDay, char *err, size_t errLen) {
  alarm_t *a = editable(id);
  if (!a) return false;
  alarm_t tmp = *a;
  tmp.startMin = clampMinOfDay(minOfDay);
  uint8_t with = 0;
  if (tmp.enabled && wouldConflict(id, tmp, &with)) {
    conflictMsg(err, errLen, with);
    return false;
  }
  if (err && errLen) err[0] = 0;
  a->startMin = tmp.startMin;
  save();
  return true;
}

bool AlarmManager::setStop(uint8_t id, uint16_t minOfDay, char *err, size_t errLen) {
  alarm_t *a = editable(id);
  if (!a) return false;
  alarm_t tmp = *a;
  tmp.stopMin = clampMinOfDay(minOfDay);
  uint8_t with = 0;
  if (tmp.enabled && wouldConflict(id, tmp, &with)) {
    conflictMsg(err, errLen, with);
    return false;
  }
  if (err && errLen) err[0] = 0;
  a->stopMin = tmp.stopMin;
  save();
  return true;
}

bool AlarmManager::setStation(uint8_t id, uint16_t station) {
  alarm_t *a = editable(id);
  if (!a) return false;
  uint16_t len = config.playlistLength();
  if (station < 1) station = 1;
  if (len > 0 && station > len) station = len;
  a->station = station;
  save();
  return true;
}

bool AlarmManager::setVolume(uint8_t id, uint8_t volume) {
  alarm_t *a = editable(id);
  if (!a) return false;
  if (volume > 254) volume = 254;
  a->volume = volume;
  save();
  return true;
}

bool AlarmManager::setDays(uint8_t id, uint8_t daysMask, char *err, size_t errLen) {
  alarm_t *a = editable(id);
  if (!a) return false;
  alarm_t tmp = *a;
  tmp.days = daysMask & 0x7F;
  uint8_t with = 0;
  if (tmp.enabled && wouldConflict(id, tmp, &with)) {
    conflictMsg(err, errLen, with);
    return false;
  }
  if (err && errLen) err[0] = 0;
  a->days = tmp.days;
  save();
  return true;
}

bool AlarmManager::setDaysPreset(uint8_t id, uint8_t preset, char *err, size_t errLen) {
  uint8_t mask = 0x7F;
  if (preset == 1) mask = 0x3E;
  else if (preset == 2) mask = 0x41;
  return setDays(id, mask, err, errLen);
}

bool AlarmManager::toggleDay(uint8_t id, uint8_t wday, char *err, size_t errLen) {
  alarm_t *a = editable(id);
  if (!a || wday > 6) return false;
  return setDays(id, (uint8_t)(a->days ^ (1u << wday)), err, errLen);
}

void AlarmManager::_startSession(uint8_t id) {
  if (id >= ALARM_COUNT) return;
  const alarm_t &a = _alarms[id];
  _activeId = (int8_t)id;
  _snoozeUntilMs = 0;
  _snoozeFromId = -1;
  player.setVol(a.volume);
  config.setVolume(a.volume);
  player.sendCommand({PR_PLAY, a.station});
  display.putRequest(PSTART);
  display.putRequest(NEWMODE, PLAYER);
  Serial.printf("[ALARM] start #%u station=%u vol=%u\n", id + 1, a.station, a.volume);
}

void AlarmManager::_stopSession() {
  if (_activeId < 0) return;
  Serial.printf("[ALARM] stop #%d\n", _activeId + 1);
  _activeId = -1;
  player.sendCommand({PR_STOP, 0});
  display.putRequest(PSTOP);
}

void AlarmManager::dismiss() {
  if (_activeId < 0 && _snoozeUntilMs == 0) return;
  _snoozeUntilMs = 0;
  _snoozeFromId = -1;
  _stopSession();
}

void AlarmManager::snooze() {
  if (_activeId < 0 && _snoozeUntilMs == 0) return;
  if (_activeId >= 0) {
    _snoozeFromId = _activeId;
    _snoozeStation = _alarms[_activeId].station;
    _snoozeVolume = _alarms[_activeId].volume;
    _stopSession();
  }
  _snoozeUntilMs = millis() + (uint32_t)ALARM_SNOOZE_MIN * 60UL * 1000UL;
  Serial.printf("[ALARM] snooze %u min\n", ALARM_SNOOZE_MIN);
}

void AlarmManager::tick(const struct tm &now) {
  if (now.tm_year < 100) return;

  uint16_t nowMin = (uint16_t)(now.tm_hour * 60 + now.tm_min);
  int minuteKey = now.tm_yday * 1440 + nowMin;
  if (minuteKey != _lastCheckedMinute) {
    _lastCheckedMinute = minuteKey;
    _firedThisMinute = false;
  }

  if (_snoozeUntilMs != 0 && (int32_t)(millis() - _snoozeUntilMs) >= 0) {
    _snoozeUntilMs = 0;
    int8_t id = _snoozeFromId >= 0 ? _snoozeFromId : 0;
    _snoozeFromId = -1;
    player.setVol(_snoozeVolume);
    config.setVolume(_snoozeVolume);
    player.sendCommand({PR_PLAY, _snoozeStation});
    display.putRequest(PSTART);
    display.putRequest(NEWMODE, PLAYER);
    _activeId = id;
    Serial.println("[ALARM] snooze ended -> play");
  }

  if (_activeId >= 0 && _snoozeUntilMs == 0) {
    const alarm_t &a = _alarms[_activeId];
    // After snooze resume, still respect original stop window when possible.
    if (!a.enabled || !_dayMatches(a.days, now.tm_wday) || !_inWindow(a, nowMin)) {
      // If resumed after snooze past stop — still allow until user dismisses if
      // we are past window; stop only when stop time reached for normal sessions.
      if (_inWindow(a, nowMin) == false) _stopSession();
    }
  }

  if (_firedThisMinute || _activeId >= 0 || _snoozeUntilMs != 0) return;

  for (uint8_t i = 0; i < ALARM_COUNT; i++) {
    const alarm_t &a = _alarms[i];
    if (!a.enabled) continue;
    if (!_dayMatches(a.days, now.tm_wday)) continue;
    if (nowMin == a.startMin && _inWindow(a, nowMin)) {
      _firedThisMinute = true;
      _startSession(i);
      break;
    }
  }
}

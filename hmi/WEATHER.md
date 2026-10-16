# Weather (OpenWeatherMap)

**English** · [Русский](WEATHER.ru.md)

Configure in the web UI: **`/settings.html` → Weather** — enable weather, lat/lon, API key → Apply.

The firmware talks to OpenWeatherMap over **HTTPS**. The API key is **not shown** when settings are loaded in the browser: if a key is already stored, the field stays empty and a separate flag means “key is set”. Saving an empty key field does **not** erase the stored key.

## Weather icon (VP `5310`)

| DGUS field | Value |
|---|---|
| VP | `5310` |
| Icon file | your `*_weather.icl` |
| Minimum | 0 → Icon ID 0 |
| Maximum | 9 → Icon ID 9 |

If Maximum = 0, the icon never changes.

| Icon ID | OWM code | Meaning |
|---:|---|---|
| 0 | `01` | clear |
| 1 | `02` | few clouds |
| 2 | `03` | clouds |
| 3 | `04` | overcast |
| 4 | `09` | shower |
| 5 | `10` | rain |
| 6 | `11` | thunderstorm |
| 7 | `13` | snow |
| 8 | `50` | mist |
| 9 | — | no data |

## Numbers

Type: Data Variable Int, Decimal digits = **0**. Firmware writes integers (no ×10 scaling).

| VP | Meaning | Example |
|---:|---|---|
| `5312` | temperature °C | `12` |
| `5314` | feels-like °C | `12` |
| `5316` | pressure mmHg | `741` |
| `5318` | humidity % | `79` |
| `531A` | wind m/s | `5` |
| `531C` | pressure hPa (as on the OWM site) | `1008` |

Use `531C` (hPa) to compare with the OpenWeatherMap website; `5316` is mmHg.

## Description

| VP | Text Length | Coding |
|---:|---:|---|
| `5620` | 48 | 8bit |

## Serial check

```
##WEATHER###: ..., temp:12.3 C, pressure:741mmHg (988hPa), ..., wind:5.0 m/s, icon=5
[DWIN] weather icon=5 temp=12 feels=12 press=741mm/988hPa hum=79 wind=5
```

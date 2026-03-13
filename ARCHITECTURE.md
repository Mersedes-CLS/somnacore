# SomnaCore — Architecture Reference

Фитнес-трекер для тренажёрного зала: ESP32 + VL53L0X лазерный датчик расстояния. Считает повторения, определяет вес по калибровке, отправляет данные на бэкенд, отображает в Telegram Mini App.

## Дерево файлов

```
SomnaCore/
├── platformio.ini                # PlatformIO: esp32dev, ArduinoJson v7
├── CLAUDE.md                     # Инструкции для Claude Code (HW, регистры, I2C debug)
├── ARCHITECTURE.md               # Этот файл
├── README.md                     # Краткое описание проекта
│
├── src/                          # === ESP32 Firmware ===
│   ├── main.cpp                  # Точка входа: init + main loop (50ms)
│   ├── config.h                  # Все константы (пины, тайминги, пороги, WiFi, URL)
│   ├── hal/
│   │   └── i2c_bus.h/.cpp        # I2C init, bus recovery (bit-bang SCL), scan
│   ├── drivers/
│   │   └── vl53l0x.h/.cpp        # VL53L0X: 11-step register init, single-shot ranging
│   ├── processing/
│   │   ├── filter.h/.cpp         # Медианный фильтр (3 сэмпла, bounds 30–2000mm)
│   │   └── rep_detector.h/.cpp   # Детектор повторений (baseline drift + delta)
│   ├── app/
│   │   ├── session.h/.cpp        # State machine: IDLE → IN_SET → REST
│   │   └── set_record.h          # Struct: reps, ROM, duration, timestamp
│   ├── calib/
│   │   └── calibrator.h/.cpp     # Serial + remote калибровка, distToWeightKg()
│   └── net/
│       ├── wifi_manager.h/.cpp   # WiFi connect/reconnect
│       ├── api_client.h/.cpp     # POST /api/set (отправка подхода)
│       ├── calib_client.h/.cpp   # POST /api/calib/live, GET /api/calib/table
│       └── web_server.h/.cpp     # Локальный HTTP: /distance, /status
│
├── server/                       # === Node.js Backend ===
│   ├── index.js                  # Express app, route mounting
│   ├── db.js                     # PostgreSQL pool + schema migrations
│   ├── bot.js                    # Telegram bot (grammy, /start → Web App)
│   ├── package.json              # express, pg, grammy, dotenv
│   ├── BACKEND.md                # Документация API
│   ├── routes/
│   │   ├── sets.js               # POST /api/set, GET /api/sets
│   │   ├── machines.js           # CRUD /api/machines
│   │   ├── stats.js              # GET /api/today, /api/stats
│   │   └── calibration.js        # /api/calib/* (5 эндпоинтов)
│   └── public/
│       ├── index.html            # Telegram Mini App HTML
│       ├── style.css             # Стили
│       └── app.js                # JS: 4 вкладки (today, history, stats, calib)
│
├── data/                         # LittleFS (ESP32 local web UI)
│   └── index.html                # Локальная страница /
│
├── docs/
│   └── data_flow.md              # Диаграммы потоков данных
│
├── step_A_i2c_scan/              # Debug: I2C scan → 0x29
├── step_B_read_id/               # Debug: Model ID → 0xEE
├── step_C_write_read/            # Debug: Write/read reg 0x80
├── step_D_minimal_init/          # Debug: Full 11-step init
├── step_E_single_shot/           # Debug: Single-shot ranging loop
│
└── calib_src/                    # Альтернативный скетч только для калибровки
```

## Обзор системы

```
┌──────────────────────────┐
│       Telegram App       │
│  (Mini App in WebView)   │
│  today|history|stats|cal │
└──────────┬───────────────┘
           │ HTTPS polling
           ▼
┌──────────────────────────┐      ┌────────────────────┐
│    Node.js Backend       │      │    PostgreSQL       │
│    (Express + grammy)    │◄────►│  machines, sets,    │
│    Railway deployment    │      │  calibrations       │
└──────────┬───────────────┘      └────────────────────┘
           │ HTTPS
           ▼
┌──────────────────────────┐
│       ESP32 MCU          │
│  VL53L0X → filter →     │
│  rep detect → session →  │
│  POST /api/set           │
└──────────────────────────┘
```

## ESP32 Firmware

### Main Loop (50ms цикл)

```
main.cpp::loop()
  │
  ├─ webServer.handleClient()        # Локальный HTTP
  ├─ calibrator.tick()               # Serial ввод + live mode
  │
  ├─ vl53l0x.readDistance()          # Single-shot ranging
  │   └─ filter.update(raw)         # Медиана 3-х, bounds [30..2000]
  │
  ├─ Если ошибка: errorCount++      # После 3-х ошибок → sensor.reset()
  │
  ├─ calibPushLive() каждые 2с      # POST /api/calib/live (distance + weight)
  │
  ├─ repDetector.update(filtered)    # Детекция повторения
  │   ├─ Вход: delta > 30% baseline
  │   ├─ Пик: track max delta
  │   └─ Выход: delta < 10% baseline, ≥300ms → feedRep()
  │
  ├─ session.feedDistance(dist)      # Ring buffer [0..63] во время IN_SET
  │
  └─ session.tick()                  # 30с без репов → closeSet()
      └─ closeSet()
          ├─ calibrator.distToWeightKg(buf) → вес
          └─ net::sendSet() → POST /api/set
```

### State Machine (Session)

```
              feedRep()               30s timeout
  ┌──────┐ ──────────► ┌────────┐ ──────────────► ┌──────┐
  │ IDLE │              │ IN_SET │                  │ REST │
  └──────┘ ◄─────────── └────────┘ ◄──────────────  └──────┘
              reset()                  feedRep()
```

- **IDLE** → первый rep → IN_SET
- **IN_SET** → таймаут 30с без репов → closeSet() → REST
- **REST** → новый rep → IN_SET

При closeSet():
1. Собирает distBuf (ring buffer, до 64 значений)
2. `distToWeightKg()` — определяет вес по калибровке
3. `sendSet()` — POST на бэкенд
4. Сохраняет SetRecord в историю (до 32)

### Модули Firmware

| Модуль | Файлы | Назначение |
|--------|-------|------------|
| HAL | `hal/i2c_bus` | I2C init, recovery (bit-bang 16 SCL), scan |
| Driver | `drivers/vl53l0x` | Прямое управление VL53L0X по регистрам, 11-step init |
| Filter | `processing/filter` | Медианный фильтр 3-х, bounds rejection |
| RepDetect | `processing/rep_detector` | Baseline drift + delta thresholds |
| Session | `app/session` | State machine IDLE/IN_SET/REST, closeSet |
| Calibrator | `calib/calibrator` | Serial wizard + remote, distToWeightKg() |
| WiFi | `net/wifi_manager` | Connect, reconnect |
| API | `net/api_client` | POST /api/set |
| CalibClient | `net/calib_client` | POST /api/calib/live, GET /api/calib/table |
| WebServer | `net/web_server` | Локальный HTTP (/distance, /status) |

### Ключевые константы (config.h)

| Константа | Значение | Описание |
|-----------|----------|----------|
| LOOP_DELAY_MS | 50 | Период main loop |
| FILTER_SIZE | 3 | Размер медианного фильтра |
| DIST_MIN / DIST_MAX | 30 / 2000 | Границы валидного расстояния (мм) |
| REP_ENTER_PCT | 0.30 | Порог входа в реп (30% от baseline) |
| REP_EXIT_PCT | 0.10 | Порог выхода из репа (10% от baseline) |
| REP_MIN_MS | 300 | Мин. длительность репа |
| SET_TIMEOUT_MS | 30000 | Таймаут без репов → закрытие подхода |
| MAX_SETS | 32 | Макс. подходов в памяти |
| DIST_BUF_SIZE | 64 | Размер ring buffer расстояний |
| CALIB_LIVE_PUSH_MS | 2000 | Интервал пуша live данных |
| MAX_SENSOR_ERRORS | 3 | Ошибок до reset сенсора |

### Калибровка (distToWeightKg)

Алгоритм определения веса по массиву расстояний:
1. Сортирует буфер расстояний
2. Outlier rejection: отбрасывает значения за пределами ±halfGap от границ калибровки
3. Вычисляет медиану оставшихся
4. Midpoint boundary lookup: находит ближайшую калибровочную точку
5. Confidence check: предупреждение если медиана ±30% от midpoint

Калибровочная таблица: 17 позиций (5кг, 10кг, ..., 85кг), каждая — 25 сэмплов → min/max/median/jitter.

## Backend (Node.js)

### Стек
- Express, PostgreSQL (pg pool), grammy (Telegram bot), dotenv
- Деплой: Railway (DATABASE_URL + SSL)

### Схема БД

```sql
machines (
  id TEXT PRIMARY KEY,         -- задаётся ESP32 (e.g. "machine_01")
  name TEXT NOT NULL,
  muscle_group TEXT,
  location TEXT
)

sets (
  id SERIAL PRIMARY KEY,
  machine_id TEXT REFERENCES machines(id),
  reps INTEGER NOT NULL,
  rom_mm INTEGER,
  duration_sec INTEGER,
  weight_kg INTEGER,
  timestamp TIMESTAMPTZ DEFAULT NOW()
)

calibrations (
  machine_id TEXT NOT NULL,
  position INTEGER NOT NULL,    -- номер позиции (0..16)
  weight_kg INTEGER NOT NULL,
  distance_mm INTEGER NOT NULL,
  dist_min INTEGER,
  dist_max INTEGER,
  jitter INTEGER,
  calibrated_at TIMESTAMPTZ DEFAULT NOW(),
  UNIQUE(machine_id, position)
)
```

### In-Memory State

```javascript
// calibration.js — ephemeral, не в БД
liveData = Map<machine_id, {distance_mm, weight_kg, updatedAt}>
// Стухает через 5с (online: false)
```

### API Endpoints

| Метод | Путь | Источник | Назначение |
|-------|------|----------|------------|
| POST | /api/set | ESP32 | Запись подхода |
| GET | /api/sets | Mini App | Список подходов (фильтр по machine_id) |
| POST | /api/machines | Mini App | Создать тренажёр |
| GET | /api/machines | Mini App | Список тренажёров |
| GET | /api/machines/:id | Mini App | Тренажёр + его подходы |
| PUT | /api/machines/:id | Mini App | Обновить тренажёр |
| DELETE | /api/machines/:id | Mini App | Удалить (SET NULL на sets) |
| GET | /api/today | Mini App | Сегодняшняя сводка + подходы |
| GET | /api/stats | Mini App | Агрегация: total, per-day (30д), per-machine |
| POST | /api/calib/live | ESP32 | Пуш live distance + weight (in-memory) |
| GET | /api/calib/live | Mini App | Чтение live данных |
| POST | /api/calib/result | ESP32 | Сохранение калибровочной точки (upsert) |
| GET | /api/calib/table | Оба | Полная таблица калибровки |
| DELETE | /api/calib/table | Mini App | Сброс калибровки для машины |

### Telegram Bot (bot.js)

- grammy, long-polling
- `/start` → ссылка на Web App (если WEBAPP_URL задан)

## Mini App (Telegram WebView)

### Вкладки

| Вкладка | Файл | Polling | Содержание |
|---------|------|---------|------------|
| Today | app.js | 5с | Сводка дня + список подходов |
| History | app.js | — | Тренажёры → подходы per machine |
| Stats | app.js | — | Total sets, per-day (30д таблица), per-machine |
| Calib | app.js | 1с | Live weight display + таблица калибровки |

### Поток калибровки

```
ESP32                           Backend                        Mini App
  │                                │                              │
  ├──POST /calib/live──────────►   │                              │
  │  (distance + weight, 2с)       │   ◄──GET /calib/live──────── │
  │                                │      (polling 1с)            │
  │                                │                              │
  │  [Serial калибровка]           │                              │
  ├──POST /calib/result────────►   │                              │
  │  (position, weight, stats)     │   ◄──GET /calib/table─────── │
  │                                │                              │
  │  [Startup]                     │                              │
  ├──GET /calib/table──────────►   │                              │
  │  (загрузка таблицы)            │                              │
```

Калибровка выполняется через Serial CLI на ESP32 (команды: s=start, ENTER=measure, r=repeat, p=print, l=live, x=reset). Результаты автоматически отправляются на бэкенд. Mini App отображает live вес и таблицу калибровки, но не управляет процессом.

## Debug-скетчи (step_A – step_E)

Изолированные тестовые скетчи для пошаговой отладки VL53L0X. Каждый загружается отдельно.

| Step | Папка | Что проверяет | Критерий прохождения |
|------|-------|---------------|---------------------|
| A | `step_A_i2c_scan/` | I2C bus scan | Адрес 0x29 найден |
| B | `step_B_read_id/` | ID регистры VL53L0X | 0xC0 = 0xEE |
| C | `step_C_write_read/` | Запись/чтение reg 0x80 | Read-back совпадает |
| D | `step_D_minimal_init/` | Полная 11-step init | VHV + Phase cal OK |
| E | `step_E_single_shot/` | Single-shot ranging loop | Расстояние 50–2000 мм |

## Особенности и edge cases

- **VL53L0X rev 1.15** — Adafruit библиотека не работает, используется прямой доступ к регистрам
- **Wire.begin()** без аргументов — с явными пинами вызывает boot loop
- **XSHUT на GPIO16** — предотвращает floating low (standby)
- **I2C bus recovery** — bit-bang 16 SCL циклов + STOP при зависании SDA
- **Sensor reset** — после 3-х ошибок: XSHUT reset + bus recovery + re-init
- **Outlier rejection** — в distToWeightKg отбрасываются выбросы ±halfGap
- **Baseline drift** — rep detector адаптирует baseline: decay 0.98 + adapt 0.02

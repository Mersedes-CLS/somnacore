# LAZER Backend

Бэкенд для умного тренажёра LAZER — принимает данные о подходах от ESP32, хранит в SQLite, отдаёт статистику.

**Стек:** Node.js, Express, better-sqlite3 (WAL mode)

## Структура файлов

```
server/
├── index.js            # Точка входа, Express app, подключение роутов
├── db.js               # Инициализация SQLite, миграции, экспорт db
├── package.json         # Зависимости (express, better-sqlite3)
├── lazer.db             # SQLite база данных
└── routes/
    ├── machines.js      # CRUD для тренажёров
    ├── sets.js          # Запись и получение подходов
    └── stats.js         # Агрегированная статистика
```

## Схема БД

### machines

| Поле         | Тип  | Описание                        |
|--------------|------|---------------------------------|
| id           | TEXT | PK, задаётся клиентом (ESP32)  |
| name         | TEXT | Название тренажёра (NOT NULL)   |
| muscle_group | TEXT | Группа мышц (nullable)         |
| location     | TEXT | Расположение в зале (nullable) |

### sets

| Поле         | Тип      | Описание                              |
|--------------|----------|---------------------------------------|
| id           | INTEGER  | PK, AUTOINCREMENT                     |
| machine_id   | TEXT     | FK → machines(id), nullable           |
| reps         | INTEGER  | Количество повторений (NOT NULL)      |
| rom_mm       | INTEGER  | Амплитуда движения в мм (nullable)    |
| duration_sec | INTEGER  | Длительность подхода в сек (nullable) |
| weight_kg    | INTEGER  | Вес в кг (nullable, пока не реализовано на ESP32) |
| timestamp    | DATETIME | DEFAULT CURRENT_TIMESTAMP             |

## API Reference

Все эндпоинты под префиксом `/api`. Формат: JSON.

### Health

```
GET /api/health
→ { "status": "ok" }
```

### Sets (подходы)

```
POST /api/set
Body: { "machine_id": "machine_01", "reps": 12, "rom_mm": 450, "duration_sec": 35 }
→ { "id": 1, "ok": true }
```

- `reps` — обязательное (number)
- `machine_id`, `rom_mm`, `duration_sec`, `weight_kg` — опциональные

```
GET /api/sets
GET /api/sets?machine_id=machine_01
→ [
    {
      "id": 1,
      "machine_id": "machine_01",
      "machine_name": "Верхняя тяга",
      "reps": 12,
      "rom_mm": 450,
      "duration_sec": 35,
      "weight_kg": null,
      "timestamp": "2026-03-09 12:00:00"
    }
  ]
```

### Machines (тренажёры)

```
POST /api/machines
Body: { "id": "machine_01", "name": "Верхняя тяга", "muscle_group": "спина", "location": "зал 1" }
→ { "id": "machine_01", "ok": true }
```

- `id` и `name` — обязательные
- 409 если машина с таким id уже есть

```
GET /api/machines
→ [{ "id": "machine_01", "name": "Верхняя тяга", "muscle_group": "спина", "location": "зал 1" }]
```

```
GET /api/machines/:id
→ { "id": "machine_01", "name": "...", "muscle_group": "...", "location": "...", "sets": [...] }
```

```
DELETE /api/machines/:id
→ { "ok": true }
```

- При удалении machine_id у связанных sets обнуляется (SET NULL)

### Stats (статистика)

```
GET /api/stats
→ {
    "total_sets": 42,
    "per_day": [
      { "day": "2026-03-09", "sets": 5, "total_reps": 60 }
    ],
    "per_machine": [
      { "machine_id": "machine_01", "machine_name": "Верхняя тяга", "sets": 10, "total_reps": 120 }
    ]
  }
```

- `per_day` — последние 30 дней
- `per_machine` — все тренажёры, сортировка по количеству подходов

## ESP32 интеграция

Прошивка ESP32 отправляет данные на бэкенд автоматически после каждого подхода.

**Конфигурация** (`src/config.h`):
- `MACHINE_ID` — идентификатор тренажёра (`"machine_01"`)
- `BACKEND_HOST` — IP компьютера с сервером (`"192.168.0.101"`)
- `BACKEND_PORT` — порт сервера (`3000`)

**Клиент** (`src/net/api_client.cpp`):
- Функция `net::sendSet(reps, romMm, durationMs)` — POST на `/api/set`
- Проверяет WiFi перед отправкой, логирует результат в Serial
- JSON body: `{"machine_id":"machine_01","reps":12,"rom_mm":450,"duration_sec":35}`

## Запуск

```bash
cd server
npm install
npm start          # → http://0.0.0.0:3000
```

Сервер слушает на `0.0.0.0:3000` — доступен из локальной сети (ESP32 подключается по WiFi).

## Реализовано

- [x] Express сервер с JSON API
- [x] SQLite с WAL mode и foreign keys
- [x] Таблица machines с CRUD
- [x] Таблица sets с записью и фильтрацией по machine_id
- [x] Агрегированная статистика (total, per-day, per-machine)
- [x] ESP32 HTTP клиент — POST подхода после детекции
- [x] machine_id привязка (ESP32 → сервер)
- [x] Health endpoint

## TODO / Roadmap

- [ ] **NFC / пользователи** — таблица users, привязка подходов к пользователю через NFC-браслет
- [ ] **weight_kg от датчика** — второй VL53L0X определяет позицию штифта → вес в кг
- [ ] **Авторизация** — токены или API-ключи для ESP32 и мобильного приложения
- [ ] **Фронтенд / мобильное приложение** — UI со статистикой, историей, графиками
- [ ] **Калибровка веса** — эндпоинт для сохранения маппинга расстояние → вес для каждого тренажёра
- [ ] **Обновление тренажёра** — PUT /api/machines/:id
- [ ] **Пагинация** — для /api/sets при большом объёме данных

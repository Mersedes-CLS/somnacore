# LAZER — Карта проекта

Умный трекер для блочных тренажёров в зале.
Датчик на тренажёре → считает повторения → данные на телефоне.

---

## Архитектура (3 слоя)

```
[ESP32 + VL53L0X]  ──HTTP POST──▶  [Сервер Railway]  ◀──Mini App──  [Telegram]
    датчик                          Express + PostgreSQL               UI

Калибровка (двусторонняя связь через сервер):
ESP32 ──push distance──▶ Backend ◀──poll distance── Mini App
ESP32 ◀──poll command─── Backend ◀──send command─── Mini App
ESP32 ──post result────▶ Backend ──poll result────▶ Mini App
```

---

## Что ГОТОВО

### Железо (ESP32 + датчик)
- [x] VL53L0X работает, меряет расстояние в мм
- [x] Детекция повторений (rep detection) по движению стека
- [x] WiFi подключение к хотспоту телефона
- [x] HTTP POST на сервер после каждого подхода
- [x] Удалённая калибровка через бэкенд (push distance, poll commands, post results)
- [x] Загрузка таблицы калибровки с сервера при старте
- [x] Автоопределение веса (weight_kg) в подходе по калибровке
- Конфиг: `src/config.h` (WiFi, backend URL, пороги датчика)

### Сервер (Express + PostgreSQL, Railway)
- [x] POST /api/set — приём данных от платы (теперь с weight_kg)
- [x] GET /api/sets — список подходов
- [x] GET /api/machines — список тренажёров
- [x] GET /api/stats — общая статистика
- [x] GET /api/today — подходы за сегодня
- [x] /api/calib/* — 8 эндпоинтов калибровки (live, command, result, table)
- [x] База данных: таблицы sets, machines, calibrations, calib_state
- Код: `server/`

### Telegram Mini App
- [x] 4 вкладки: Сегодня / История / Статистика / Калибровка
- [x] Калибровка: wizard на 17 позиций (5-85кг), живое расстояние, измерение, jitter-warnings
- [x] Отображение веса (кг) в карточках подходов
- [x] Авто-обновление каждые 5 сек
- [x] Telegram бот с кнопкой Web App
- Код: `server/public/`, `server/bot.js`

---

## Что НЕ ГОТОВО (идеи на будущее)

### Ближайшее
- [x] ~~Определение веса по позиции штифта~~ — ГОТОВО (калибровка через Mini App)
- [ ] Привязка machine_id к конкретному тренажёру
- [ ] NFC-браслет для идентификации пользователя

### Потом
- [ ] Несколько тренажёров (несколько плат)
- [ ] Мобильное приложение (или расширить Mini App)
- [ ] История по пользователям
- [ ] Графики прогресса (вес × повторения по неделям)
- [ ] Автономная работа платы (буфер данных при потере WiFi)

### Далёкое будущее
- [x] ~~Деплой сервера в облако~~ — ГОТОВО (Railway)
- [ ] Оснащение зала (много тренажёров, один сервер)
- [ ] Бизнес: подписка для залов

---

## Как всё запускать

1. Телефон раздаёт WiFi (`Pepega`)
2. Ноут подключён к тому же WiFi
3. `cd server && npm start` — сервер на порту 3000
4. `C:\Tri\ngrok.exe http 3000` — HTTPS туннель
5. Плата в повербанк → подключается → шлёт данные
6. Telegram → бот → /start → Mini App

### Важные настройки
- WiFi: `TP-Link_1018` / `46657763` (в `src/config.h`)
- Backend: `https://somnacore-production.up.railway.app` (в `src/config.h`)
- Bot token: в `server/.env`

---

## Структура файлов

```
LAZER/
├── src/                       # Прошивка ESP32
│   ├── config.h               # ВСЕ настройки (WiFi, backend URL, пороги)
│   ├── main.cpp               # Точка входа
│   ├── drivers/vl53l0x.h/.cpp # Драйвер датчика (прямые регистры)
│   ├── hal/i2c_bus.h/.cpp     # I2C инициализация и восстановление
│   ├── processing/            # Фильтр + детекция повторений
│   ├── app/session.h/.cpp     # Логика подходов (с weight_kg через калибровку)
│   ├── calib/calibrator.h/.cpp # Калибровка (Serial + удалённая через бэкенд)
│   └── net/                   # WiFi, HTTP клиент, калибровочный клиент
│       ├── wifi_manager.h/.cpp
│       ├── web_server.h/.cpp
│       ├── api_client.h/.cpp  # POST /api/set (с weight_kg)
│       └── calib_client.h/.cpp # HTTP клиент для /api/calib/*
├── server/                    # Бэкенд (Express + PostgreSQL)
│   ├── index.js               # Express сервер
│   ├── db.js                  # PostgreSQL (sets, machines, calibrations, calib_state)
│   ├── bot.js                 # Telegram бот
│   ├── routes/
│   │   ├── sets.js            # /api/set, /api/sets
│   │   ├── machines.js        # /api/machines
│   │   ├── stats.js           # /api/today, /api/stats
│   │   └── calibration.js     # /api/calib/* (8 эндпоинтов)
│   ├── public/                # Telegram Mini App
│   │   ├── index.html         # 4 вкладки
│   │   ├── app.js             # Логика UI + wizard калибровки
│   │   └── style.css          # Стили (Telegram theme vars)
│   └── .env                   # Секреты (не в git)
├── step_A-E/                  # Отладочные скетчи
├── PROJECT_MAP.md             # ← ТЫ ЗДЕСЬ
└── CLAUDE.md                  # Инструкции для Claude
```

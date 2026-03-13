---
description: Задачи связанные с калибровкой (все 3 слоя)
argument-hint: <что сделать> (пример: remote-калибровка через Mini App)
---

Калибровка: $ARGUMENTS

Контекст:
- Firmware: src/calib/calibrator.h/.cpp
- Backend: server/routes/calibration.js
- Mini App: server/public/app.js (calib section)
- Таблица: 17 позиций (5-85кг), 25 сэмплов
- Live: POST каждые 2с → Map → polling 1с
- Serial: s=start, ENTER=measure, r=repeat, p=print, l=live, x=reset

Если меняешь формат — обнови ВСЕ три слоя. Сначала покажи план.

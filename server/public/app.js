// Telegram Web App init
if (window.Telegram && Telegram.WebApp) {
  Telegram.WebApp.ready();
  Telegram.WebApp.expand();
}

// Tabs
document.querySelectorAll(".tab").forEach((btn) => {
  btn.addEventListener("click", () => {
    document.querySelectorAll(".tab").forEach((t) => t.classList.remove("active"));
    document.querySelectorAll(".tab-content").forEach((c) => c.classList.remove("active"));
    btn.classList.add("active");
    document.getElementById(btn.dataset.tab).classList.add("active");

    if (btn.dataset.tab === "today") loadToday();
    if (btn.dataset.tab === "history") loadMachines();
    if (btn.dataset.tab === "stats") loadStats();
    if (btn.dataset.tab === "calib") loadCalib();
  });
});

// --- Today ---
let todayTimer = null;

async function loadToday() {
  try {
    const res = await fetch("/api/today");
    const data = await res.json();
    renderToday(data);
  } catch (e) {
    document.getElementById("today-summary").innerHTML =
      '<div class="empty">Не удалось загрузить данные</div>';
  }
  clearInterval(todayTimer);
  todayTimer = setInterval(loadToday, 5000);
}

function renderToday(data) {
  const { summary, sets } = data;
  document.getElementById("today-summary").innerHTML = `
    <div class="summary-row">
      <div>
        <div class="big-number">${summary.sets}</div>
        <div class="label">подходов</div>
      </div>
      <div>
        <div class="big-number">${summary.total_reps}</div>
        <div class="label">повторений</div>
      </div>
    </div>
  `;

  if (sets.length === 0) {
    document.getElementById("today-sets").innerHTML =
      '<div class="empty">Пока нет данных за сегодня</div>';
    return;
  }

  document.getElementById("today-sets").innerHTML = sets
    .map((s) => {
      const time = new Date(s.timestamp).toLocaleTimeString("ru-RU", {
        hour: "2-digit",
        minute: "2-digit",
      });
      return `
      <div class="card">
        <div class="card-title">${s.machine_name || s.machine_id}</div>
        <div class="card-details">
          <span>${s.reps} повт.</span>
          ${s.weight_kg ? `<span>${s.weight_kg} кг</span>` : ""}
          ${s.rom_mm ? `<span>ROM ${s.rom_mm} мм</span>` : ""}
          <span>${time}</span>
        </div>
      </div>`;
    })
    .join("");
}

// --- History ---
async function loadMachines() {
  document.getElementById("machine-sets").innerHTML = "";
  try {
    const res = await fetch("/api/machines");
    const machines = await res.json();
    if (machines.length === 0) {
      document.getElementById("machine-list").innerHTML =
        '<div class="empty">Нет тренажёров</div>';
      return;
    }
    document.getElementById("machine-list").innerHTML = machines
      .map(
        (m) =>
          `<button class="machine-btn" data-id="${m.id}">${m.name || m.id}</button>`
      )
      .join("");

    document.querySelectorAll(".machine-btn").forEach((btn) => {
      btn.addEventListener("click", () => loadMachineSets(btn.dataset.id));
    });
  } catch (e) {
    document.getElementById("machine-list").innerHTML =
      '<div class="empty">Ошибка загрузки</div>';
  }
}

async function loadMachineSets(machineId) {
  document.getElementById("machine-list").style.display = "none";
  try {
    const res = await fetch(`/api/sets?machine_id=${encodeURIComponent(machineId)}`);
    const sets = await res.json();
    let html = `<button class="back-btn" id="back-to-machines">\u2190 Назад</button>`;
    if (sets.length === 0) {
      html += '<div class="empty">Нет подходов</div>';
    } else {
      html += sets
        .map((s) => {
          const dt = new Date(s.timestamp).toLocaleString("ru-RU", {
            day: "2-digit",
            month: "2-digit",
            hour: "2-digit",
            minute: "2-digit",
          });
          return `
          <div class="card">
            <div class="card-title">${s.reps} повт.</div>
            <div class="card-details">
              ${s.weight_kg ? `<span>${s.weight_kg} кг</span>` : ""}
              ${s.rom_mm ? `<span>ROM ${s.rom_mm} мм</span>` : ""}
              <span>${dt}</span>
            </div>
          </div>`;
        })
        .join("");
    }
    document.getElementById("machine-sets").innerHTML = html;
    document.getElementById("back-to-machines").addEventListener("click", () => {
      document.getElementById("machine-list").style.display = "";
      document.getElementById("machine-sets").innerHTML = "";
    });
  } catch (e) {
    document.getElementById("machine-sets").innerHTML =
      '<div class="empty">Ошибка загрузки</div>';
  }
}

// --- Stats ---
async function loadStats() {
  try {
    const res = await fetch("/api/stats");
    const data = await res.json();
    renderStats(data);
  } catch (e) {
    document.getElementById("stats-total").innerHTML =
      '<div class="empty">Ошибка загрузки</div>';
  }
}

function renderStats(data) {
  document.getElementById("stats-total").innerHTML = `
    <div class="summary">
      <div class="big-number">${data.total_sets}</div>
      <div class="label">всего подходов</div>
    </div>
  `;

  if (data.per_day.length > 0) {
    document.getElementById("stats-per-day").innerHTML = `
      <div class="section-title">По дням</div>
      <table>
        <tr><th>Дата</th><th>Подходы</th><th>Повторения</th></tr>
        ${data.per_day
          .map(
            (d) => `<tr><td>${d.day}</td><td>${d.sets}</td><td>${d.total_reps || 0}</td></tr>`
          )
          .join("")}
      </table>
    `;
  }

  if (data.per_machine.length > 0) {
    document.getElementById("stats-per-machine").innerHTML = `
      <div class="section-title">По тренажёрам</div>
      <table>
        <tr><th>Тренажёр</th><th>Подходы</th><th>Повторения</th></tr>
        ${data.per_machine
          .map(
            (m) =>
              `<tr><td>${m.machine_name || m.machine_id}</td><td>${m.sets}</td><td>${m.total_reps || 0}</td></tr>`
          )
          .join("")}
      </table>
    `;
  }
}

// --- Calibration ---
let calibMachineId = null;
let calibLiveTimer = null;
let calibResultTimer = null;
let calibWizardPos = 0;
let calibWizardActive = false;
const CALIB_NUM_POS = 17;
const CALIB_FIRST_KG = 5;
const CALIB_STEP_KG = 5;
function calibPosKg(i) { return CALIB_FIRST_KG + i * CALIB_STEP_KG; }

function stopCalibTimers() {
  clearInterval(calibLiveTimer);
  clearInterval(calibResultTimer);
  calibLiveTimer = null;
  calibResultTimer = null;
}

async function loadCalib() {
  stopCalibTimers();
  calibWizardActive = false;
  const selectEl = document.getElementById("calib-machine-select");
  const contentEl = document.getElementById("calib-content");
  try {
    const res = await fetch("/api/machines");
    const machines = await res.json();
    if (machines.length === 0) {
      selectEl.innerHTML = '<div class="empty">Нет тренажёров</div>';
      contentEl.innerHTML = "";
      return;
    }
    selectEl.innerHTML = machines
      .map(
        (m) =>
          `<button class="machine-btn calib-machine-btn" data-id="${m.id}">${m.name || m.id}</button>`
      )
      .join("");
    contentEl.innerHTML = "";
    document.querySelectorAll(".calib-machine-btn").forEach((btn) => {
      btn.addEventListener("click", () => {
        calibMachineId = btn.dataset.id;
        selectEl.style.display = "none";
        loadCalibTable();
      });
    });
  } catch (e) {
    selectEl.innerHTML = '<div class="empty">Ошибка загрузки</div>';
  }
}

async function loadCalibTable() {
  const contentEl = document.getElementById("calib-content");
  try {
    const res = await fetch(`/api/calib/table?machine_id=${encodeURIComponent(calibMachineId)}`);
    const table = await res.json();

    let html = `<button class="back-btn" id="calib-back">\u2190 Назад</button>`;
    html += `<div class="section-title">Калибровка: ${calibMachineId}</div>`;

    if (table.length > 0) {
      html += `<table><tr><th>#</th><th>кг</th><th>мм</th><th>jitter</th></tr>`;
      for (const row of table) {
        const warn = row.jitter > 30 ? ' class="calib-warn"' : "";
        html += `<tr${warn}><td>${row.position + 1}</td><td>${row.weight_kg}</td><td>${row.distance_mm}</td><td>${row.jitter}</td></tr>`;
      }
      html += `</table>`;
    } else {
      html += `<div class="empty">Нет данных калибровки</div>`;
    }

    html += `<div class="calib-actions">`;
    html += `<button class="calib-btn calib-btn-primary" id="calib-start-wizard">Калибровать</button>`;
    if (table.length > 0) {
      html += `<button class="calib-btn calib-btn-danger" id="calib-reset">Сбросить</button>`;
    }
    html += `</div>`;

    contentEl.innerHTML = html;
    document.getElementById("calib-back").addEventListener("click", () => {
      stopCalibTimers();
      calibWizardActive = false;
      document.getElementById("calib-machine-select").style.display = "";
      contentEl.innerHTML = "";
    });
    document.getElementById("calib-start-wizard").addEventListener("click", startCalibWizard);
    const resetBtn = document.getElementById("calib-reset");
    if (resetBtn) {
      resetBtn.addEventListener("click", async () => {
        if (!confirm("Сбросить все данные калибровки?")) return;
        await fetch(`/api/calib/table?machine_id=${encodeURIComponent(calibMachineId)}`, { method: "DELETE" });
        loadCalibTable();
      });
    }
  } catch (e) {
    contentEl.innerHTML = '<div class="empty">Ошибка загрузки</div>';
  }
}

function startCalibWizard() {
  calibWizardPos = 0;
  calibWizardActive = true;
  renderWizardStep();
}

function renderWizardStep() {
  const contentEl = document.getElementById("calib-content");
  const kg = calibPosKg(calibWizardPos);
  const progress = Math.round(((calibWizardPos) / CALIB_NUM_POS) * 100);

  let html = `<div class="calib-wizard">`;
  html += `<div class="calib-progress"><div class="calib-progress-bar" style="width:${progress}%"></div></div>`;
  html += `<div class="calib-step-info">${calibWizardPos + 1} / ${CALIB_NUM_POS} &mdash; ${kg} кг</div>`;
  html += `<div class="calib-instruction">Установите штифт на позицию ${calibWizardPos + 1} (${kg} кг) и держите неподвижно</div>`;
  html += `<div class="calib-live" id="calib-live-display">Датчик: ---</div>`;
  html += `<div id="calib-measure-result"></div>`;
  html += `<div class="calib-actions">`;
  html += `<button class="calib-btn calib-btn-primary" id="calib-measure-btn">Измерить</button>`;
  html += `<button class="calib-btn" id="calib-skip-btn">Пропустить</button>`;
  html += `<button class="calib-btn calib-btn-danger" id="calib-cancel-btn">Отмена</button>`;
  html += `</div></div>`;
  contentEl.innerHTML = html;

  document.getElementById("calib-measure-btn").addEventListener("click", doCalibMeasure);
  document.getElementById("calib-skip-btn").addEventListener("click", () => {
    calibWizardPos++;
    if (calibWizardPos >= CALIB_NUM_POS) finishCalibWizard();
    else renderWizardStep();
  });
  document.getElementById("calib-cancel-btn").addEventListener("click", () => {
    stopCalibTimers();
    calibWizardActive = false;
    loadCalibTable();
  });

  // Start live distance polling
  stopCalibTimers();
  pollLiveDistance();
  calibLiveTimer = setInterval(pollLiveDistance, 500);
}

async function pollLiveDistance() {
  try {
    const res = await fetch(`/api/calib/live?machine_id=${encodeURIComponent(calibMachineId)}`);
    const data = await res.json();
    const el = document.getElementById("calib-live-display");
    if (!el) return;
    if (data.stale || data.distance_mm == null) {
      el.textContent = "Датчик: оффлайн";
      el.classList.add("calib-offline");
    } else {
      el.textContent = `Датчик: ${data.distance_mm} мм`;
      el.classList.remove("calib-offline");
    }
  } catch (e) { /* ignore */ }
}

async function doCalibMeasure() {
  const measureBtn = document.getElementById("calib-measure-btn");
  const resultEl = document.getElementById("calib-measure-result");
  measureBtn.disabled = true;
  measureBtn.textContent = "Измеряем...";
  resultEl.innerHTML = '<div class="calib-measuring">Ожидание результата от ESP32...</div>';

  // Send measure command
  await fetch("/api/calib/command", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ machine_id: calibMachineId, command: "measure", position: calibWizardPos }),
  });

  // Poll for result
  let attempts = 0;
  clearInterval(calibResultTimer);
  calibResultTimer = setInterval(async () => {
    attempts++;
    try {
      const res = await fetch(`/api/calib/result?machine_id=${encodeURIComponent(calibMachineId)}`);
      const data = await res.json();
      if (data.result && data.result.position === calibWizardPos) {
        clearInterval(calibResultTimer);
        calibResultTimer = null;
        showMeasureResult(data.result);
      } else if (attempts > 30) {
        clearInterval(calibResultTimer);
        calibResultTimer = null;
        resultEl.innerHTML = '<div class="calib-warn-text">Таймаут ожидания. ESP32 не ответил.</div>';
        measureBtn.disabled = false;
        measureBtn.textContent = "Измерить";
      }
    } catch (e) {
      clearInterval(calibResultTimer);
      calibResultTimer = null;
      resultEl.innerHTML = '<div class="calib-warn-text">Ошибка сети</div>';
      measureBtn.disabled = false;
      measureBtn.textContent = "Измерить";
    }
  }, 500);
}

function showMeasureResult(result) {
  const resultEl = document.getElementById("calib-measure-result");
  const measureBtn = document.getElementById("calib-measure-btn");

  let warn = "";
  if (result.jitter > 30) warn += `<div class="calib-warn-text">Jitter высокий: ${result.jitter} мм</div>`;

  resultEl.innerHTML = `
    <div class="calib-result-card">
      <div><strong>${result.weight_kg} кг</strong> &mdash; ${result.distance_mm} мм</div>
      <div class="calib-result-details">min: ${result.dist_min} / max: ${result.dist_max} / jitter: ${result.jitter}</div>
      ${warn}
    </div>
  `;

  measureBtn.textContent = "Повторить";
  measureBtn.disabled = false;

  // Add Next button
  const actionsEl = measureBtn.parentElement;
  if (!document.getElementById("calib-next-btn")) {
    const nextBtn = document.createElement("button");
    nextBtn.id = "calib-next-btn";
    nextBtn.className = "calib-btn calib-btn-primary";
    nextBtn.textContent = "Далее";
    nextBtn.addEventListener("click", () => {
      calibWizardPos++;
      if (calibWizardPos >= CALIB_NUM_POS) finishCalibWizard();
      else renderWizardStep();
    });
    actionsEl.insertBefore(nextBtn, measureBtn.nextSibling);
  }
}

function finishCalibWizard() {
  stopCalibTimers();
  calibWizardActive = false;
  const contentEl = document.getElementById("calib-content");
  contentEl.innerHTML = `
    <div class="calib-complete">
      <div class="big-number">OK</div>
      <div class="label">Калибровка завершена</div>
    </div>
  `;
  setTimeout(loadCalibTable, 2000);
}

// Initial load
loadToday();

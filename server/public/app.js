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
let calibTableTimer = null;
let calibLiveTimer = null;

function stopCalibTimers() {
  clearInterval(calibTableTimer);
  calibTableTimer = null;
  clearInterval(calibLiveTimer);
  calibLiveTimer = null;
}

async function loadCalib() {
  stopCalibTimers();
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
    html += `<div id="calib-live-weight" class="calib-live-weight">Датчик: загрузка...</div>`;

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
    if (table.length > 0) {
      html += `<button class="calib-btn calib-btn-danger" id="calib-reset">Сбросить</button>`;
    }
    html += `</div>`;

    contentEl.innerHTML = html;
    document.getElementById("calib-back").addEventListener("click", () => {
      stopCalibTimers();
      document.getElementById("calib-machine-select").style.display = "";
      contentEl.innerHTML = "";
    });
    const resetBtn = document.getElementById("calib-reset");
    if (resetBtn) {
      resetBtn.addEventListener("click", async () => {
        if (!confirm("Сбросить все данные калибровки?")) return;
        await fetch(`/api/calib/table?machine_id=${encodeURIComponent(calibMachineId)}`, { method: "DELETE" });
        loadCalibTable();
      });
    }

    // Auto-refresh table every 5s to see new points from Serial calibration
    clearInterval(calibTableTimer);
    calibTableTimer = setInterval(loadCalibTable, 5000);

    // Start live weight polling
    pollCalibLive();
    clearInterval(calibLiveTimer);
    calibLiveTimer = setInterval(pollCalibLive, 2000);
  } catch (e) {
    contentEl.innerHTML = '<div class="empty">Ошибка загрузки</div>';
  }
}

async function pollCalibLive() {
  const el = document.getElementById("calib-live-weight");
  if (!el || !calibMachineId) return;
  try {
    const res = await fetch(`/api/calib/live?machine_id=${encodeURIComponent(calibMachineId)}`);
    const d = await res.json();
    if (!d.online) {
      el.textContent = "Датчик: оффлайн";
      el.className = "calib-live-weight offline";
    } else if (d.weight_kg > 0) {
      el.innerHTML = `Сейчас: <strong>${d.weight_kg} кг</strong> <span class="calib-live-dist">(${d.distance_mm} мм)</span>`;
      el.className = "calib-live-weight online";
    } else {
      el.innerHTML = `Датчик: ${d.distance_mm} мм <span class="calib-live-dist">(вес не определён)</span>`;
      el.className = "calib-live-weight online";
    }
  } catch {
    if (el) {
      el.textContent = "Датчик: ошибка связи";
      el.className = "calib-live-weight offline";
    }
  }
}

// Initial load
loadToday();

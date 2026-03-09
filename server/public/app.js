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

// Initial load
loadToday();

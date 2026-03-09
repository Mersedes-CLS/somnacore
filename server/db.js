const Database = require("better-sqlite3");
const path = require("path");

const db = new Database(path.join(__dirname, "lazer.db"));
db.pragma("journal_mode = WAL");
db.pragma("foreign_keys = ON");

// --- Migrations ---

db.exec(`
  CREATE TABLE IF NOT EXISTS machines (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    muscle_group TEXT,
    location TEXT
  )
`);

db.exec(`
  CREATE TABLE IF NOT EXISTS sets (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    machine_id TEXT REFERENCES machines(id),
    reps INTEGER NOT NULL,
    rom_mm INTEGER,
    duration_sec INTEGER,
    weight_kg INTEGER,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
  )
`);

// Add machine_id column if migrating from old schema
const cols = db.pragma("table_info(sets)").map((c) => c.name);
if (!cols.includes("machine_id")) {
  db.exec("ALTER TABLE sets ADD COLUMN machine_id TEXT REFERENCES machines(id)");
}

module.exports = db;

const Database = require("better-sqlite3");
const path = require("path");

const db = new Database(path.join(__dirname, "lazer.db"));
db.pragma("journal_mode = WAL");

db.exec(`
  CREATE TABLE IF NOT EXISTS sets (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    reps INTEGER NOT NULL,
    rom_mm INTEGER,
    duration_sec INTEGER,
    weight_kg INTEGER,
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP
  )
`);

const insertSet = db.prepare(
  "INSERT INTO sets (reps, rom_mm, duration_sec, weight_kg) VALUES (?, ?, ?, ?)"
);

const getAllSets = db.prepare(
  "SELECT id, reps, rom_mm, duration_sec, weight_kg, timestamp FROM sets ORDER BY id DESC"
);

module.exports = { insertSet, getAllSets };

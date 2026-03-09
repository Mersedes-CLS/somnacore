const { Pool } = require("pg");

const pool = new Pool({
  connectionString: process.env.DATABASE_URL,
  ssl: process.env.NODE_ENV === "production" ? { rejectUnauthorized: false } : false,
});

async function migrate() {
  await pool.query(`
    CREATE TABLE IF NOT EXISTS machines (
      id TEXT PRIMARY KEY,
      name TEXT NOT NULL,
      muscle_group TEXT,
      location TEXT
    )
  `);

  await pool.query(`
    CREATE TABLE IF NOT EXISTS sets (
      id SERIAL PRIMARY KEY,
      machine_id TEXT REFERENCES machines(id),
      reps INTEGER NOT NULL,
      rom_mm INTEGER,
      duration_sec INTEGER,
      weight_kg INTEGER,
      timestamp TIMESTAMPTZ DEFAULT NOW()
    )
  `);
}

module.exports = { pool, migrate };

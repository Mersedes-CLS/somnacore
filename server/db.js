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

  await pool.query(`
    CREATE TABLE IF NOT EXISTS calibrations (
      machine_id TEXT NOT NULL,
      position INTEGER NOT NULL,
      weight_kg INTEGER NOT NULL,
      distance_mm INTEGER NOT NULL,
      dist_min INTEGER,
      dist_max INTEGER,
      jitter INTEGER,
      calibrated_at TIMESTAMPTZ DEFAULT NOW(),
      UNIQUE(machine_id, position)
    )
  `);

}

module.exports = { pool, migrate };

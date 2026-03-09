const { Router } = require("express");
const { pool } = require("../db");

const router = Router();

router.get("/today", async (_req, res) => {
  const summary = await pool.query(
    "SELECT COUNT(*) AS sets, COALESCE(SUM(reps), 0) AS total_reps FROM sets WHERE timestamp::date = CURRENT_DATE"
  );
  const sets = await pool.query(
    `SELECT s.*, m.name AS machine_name FROM sets s
     LEFT JOIN machines m ON s.machine_id = m.id
     WHERE s.timestamp::date = CURRENT_DATE
     ORDER BY s.id DESC`
  );
  res.json({
    summary: summary.rows[0],
    sets: sets.rows,
  });
});

router.get("/stats", async (_req, res) => {
  const totalSets = await pool.query("SELECT COUNT(*) AS count FROM sets");
  const perDay = await pool.query(
    `SELECT timestamp::date AS day, COUNT(*) AS sets, SUM(reps) AS total_reps
     FROM sets GROUP BY timestamp::date ORDER BY day DESC LIMIT 30`
  );
  const perMachine = await pool.query(
    `SELECT s.machine_id, m.name AS machine_name,
            COUNT(*) AS sets, SUM(s.reps) AS total_reps
     FROM sets s LEFT JOIN machines m ON s.machine_id = m.id
     GROUP BY s.machine_id, m.name ORDER BY sets DESC`
  );
  res.json({
    total_sets: parseInt(totalSets.rows[0].count),
    per_day: perDay.rows,
    per_machine: perMachine.rows,
  });
});

module.exports = router;

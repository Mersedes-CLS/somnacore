const { Router } = require("express");
const db = require("../db");

const router = Router();

const totalSets = db.prepare("SELECT COUNT(*) AS count FROM sets");

const perDay = db.prepare(`
  SELECT DATE(timestamp) AS day, COUNT(*) AS sets, SUM(reps) AS total_reps
  FROM sets
  GROUP BY DATE(timestamp)
  ORDER BY day DESC
  LIMIT 30
`);

const perMachine = db.prepare(`
  SELECT s.machine_id, m.name AS machine_name,
         COUNT(*) AS sets, SUM(s.reps) AS total_reps
  FROM sets s
  LEFT JOIN machines m ON s.machine_id = m.id
  GROUP BY s.machine_id
  ORDER BY sets DESC
`);

router.get("/stats", (_req, res) => {
  res.json({
    total_sets: totalSets.get().count,
    per_day: perDay.all(),
    per_machine: perMachine.all(),
  });
});

module.exports = router;

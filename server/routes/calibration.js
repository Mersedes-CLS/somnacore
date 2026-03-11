const { Router } = require("express");
const { pool } = require("../db");

const router = Router();

// In-memory live distance+weight store (ephemeral, no DB)
const liveData = new Map(); // machine_id -> {distance_mm, weight_kg, updatedAt}

// ESP32 pushes live distance + weight
router.post("/calib/live", (req, res) => {
  const { machine_id, distance_mm, weight_kg } = req.body;
  if (!machine_id) return res.status(400).json({ error: "machine_id required" });
  liveData.set(machine_id, {
    distance_mm: distance_mm || 0,
    weight_kg: weight_kg != null ? weight_kg : -1,
    updatedAt: Date.now()
  });
  res.json({ ok: true });
});

// Mini App reads live distance + weight
router.get("/calib/live", (req, res) => {
  const { machine_id } = req.query;
  if (!machine_id) return res.status(400).json({ error: "machine_id required" });
  const entry = liveData.get(machine_id);
  if (!entry) return res.json({ online: false });
  const stale = Date.now() - entry.updatedAt > 5000;
  res.json({
    online: !stale,
    distance_mm: entry.distance_mm,
    weight_kg: entry.weight_kg,
    age_ms: Date.now() - entry.updatedAt
  });
});

// ESP32 posts measurement result (Serial calibration saves here)
router.post("/calib/result", async (req, res) => {
  const { machine_id, position, weight_kg, distance_mm, dist_min, dist_max, jitter } = req.body;
  if (!machine_id || position == null) return res.status(400).json({ error: "machine_id and position required" });

  await pool.query(
    `INSERT INTO calibrations (machine_id, position, weight_kg, distance_mm, dist_min, dist_max, jitter, calibrated_at)
     VALUES ($1, $2, $3, $4, $5, $6, $7, NOW())
     ON CONFLICT (machine_id, position) DO UPDATE
       SET weight_kg = $3, distance_mm = $4, dist_min = $5, dist_max = $6, jitter = $7, calibrated_at = NOW()`,
    [machine_id, position, weight_kg, distance_mm, dist_min, dist_max, jitter]
  );
  res.json({ ok: true });
});

// Full calibration table for a machine
router.get("/calib/table", async (req, res) => {
  const { machine_id } = req.query;
  if (!machine_id) return res.status(400).json({ error: "machine_id required" });
  const r = await pool.query(
    `SELECT position, weight_kg, distance_mm, dist_min, dist_max, jitter, calibrated_at
     FROM calibrations WHERE machine_id = $1 ORDER BY position`,
    [machine_id]
  );
  res.json(r.rows);
});

// Reset calibration for a machine
router.delete("/calib/table", async (req, res) => {
  const { machine_id } = req.query;
  if (!machine_id) return res.status(400).json({ error: "machine_id required" });
  await pool.query(`DELETE FROM calibrations WHERE machine_id = $1`, [machine_id]);
  res.json({ ok: true });
});

module.exports = router;

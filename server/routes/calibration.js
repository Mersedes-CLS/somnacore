const { Router } = require("express");
const { pool } = require("../db");

const router = Router();

// ESP32 pushes live distance
router.post("/calib/live", async (req, res) => {
  const { machine_id, distance_mm } = req.body;
  if (!machine_id) return res.status(400).json({ error: "machine_id required" });
  await pool.query(
    `INSERT INTO calib_state (machine_id, live_distance_mm, distance_updated_at)
     VALUES ($1, $2, NOW())
     ON CONFLICT (machine_id) DO UPDATE
       SET live_distance_mm = $2, distance_updated_at = NOW()`,
    [machine_id, distance_mm ?? null]
  );
  res.json({ ok: true });
});

// Mini App reads live distance
router.get("/calib/live", async (req, res) => {
  const { machine_id } = req.query;
  if (!machine_id) return res.status(400).json({ error: "machine_id required" });
  const r = await pool.query(
    `SELECT live_distance_mm, distance_updated_at FROM calib_state WHERE machine_id = $1`,
    [machine_id]
  );
  if (r.rows.length === 0) return res.json({ distance_mm: null, stale: true });
  const row = r.rows[0];
  const age = Date.now() - new Date(row.distance_updated_at).getTime();
  res.json({ distance_mm: row.live_distance_mm, age_ms: age, stale: age > 10000 });
});

// Mini App sends command to ESP32
router.post("/calib/command", async (req, res) => {
  const { machine_id, command, position } = req.body;
  if (!machine_id || !command) return res.status(400).json({ error: "machine_id and command required" });
  await pool.query(
    `INSERT INTO calib_state (machine_id, pending_command, command_position)
     VALUES ($1, $2, $3)
     ON CONFLICT (machine_id) DO UPDATE
       SET pending_command = $2, command_position = $3`,
    [machine_id, command, position ?? null]
  );
  res.json({ ok: true });
});

// ESP32 polls & consumes command
router.get("/calib/command", async (req, res) => {
  const { machine_id } = req.query;
  if (!machine_id) return res.status(400).json({ error: "machine_id required" });
  // Read first, then clear — RETURNING gives post-update values (NULL)
  const r = await pool.query(
    `SELECT pending_command AS command, command_position AS position
     FROM calib_state WHERE machine_id = $1 AND pending_command IS NOT NULL`,
    [machine_id]
  );
  if (r.rows.length === 0) return res.json({ command: null });
  await pool.query(
    `UPDATE calib_state SET pending_command = NULL, command_position = NULL WHERE machine_id = $1`,
    [machine_id]
  );
  res.json(r.rows[0]);
});

// ESP32 posts measurement result
router.post("/calib/result", async (req, res) => {
  const { machine_id, position, weight_kg, distance_mm, dist_min, dist_max, jitter } = req.body;
  if (!machine_id || position == null) return res.status(400).json({ error: "machine_id and position required" });

  // Save to calibrations table
  await pool.query(
    `INSERT INTO calibrations (machine_id, position, weight_kg, distance_mm, dist_min, dist_max, jitter, calibrated_at)
     VALUES ($1, $2, $3, $4, $5, $6, $7, NOW())
     ON CONFLICT (machine_id, position) DO UPDATE
       SET weight_kg = $3, distance_mm = $4, dist_min = $5, dist_max = $6, jitter = $7, calibrated_at = NOW()`,
    [machine_id, position, weight_kg, distance_mm, dist_min, dist_max, jitter]
  );

  // Also store in calib_state for Mini App polling
  const result = { position, weight_kg, distance_mm, dist_min, dist_max, jitter };
  await pool.query(
    `INSERT INTO calib_state (machine_id, measure_result, result_updated_at)
     VALUES ($1, $2, NOW())
     ON CONFLICT (machine_id) DO UPDATE
       SET measure_result = $2, result_updated_at = NOW()`,
    [machine_id, JSON.stringify(result)]
  );
  res.json({ ok: true });
});

// Mini App polls for measurement result
router.get("/calib/result", async (req, res) => {
  const { machine_id } = req.query;
  if (!machine_id) return res.status(400).json({ error: "machine_id required" });
  const r = await pool.query(
    `SELECT measure_result, result_updated_at FROM calib_state WHERE machine_id = $1`,
    [machine_id]
  );
  if (r.rows.length === 0 || !r.rows[0].measure_result) return res.json({ result: null });
  res.json({ result: r.rows[0].measure_result, updated_at: r.rows[0].result_updated_at });
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
  await pool.query(
    `UPDATE calib_state SET measure_result = NULL, result_updated_at = NULL WHERE machine_id = $1`,
    [machine_id]
  );
  res.json({ ok: true });
});

module.exports = router;

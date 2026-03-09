const { Router } = require("express");
const { pool } = require("../db");

const router = Router();

router.post("/set", async (req, res) => {
  const { machine_id, reps, rom_mm, duration_sec, weight_kg } = req.body;
  if (!reps || typeof reps !== "number") {
    return res.status(400).json({ error: "reps is required (number)" });
  }
  const result = await pool.query(
    "INSERT INTO sets (machine_id, reps, rom_mm, duration_sec, weight_kg) VALUES ($1, $2, $3, $4, $5) RETURNING id",
    [machine_id ?? null, reps, rom_mm ?? null, duration_sec ?? null, weight_kg ?? null]
  );
  res.json({ id: result.rows[0].id, ok: true });
});

router.get("/sets", async (req, res) => {
  const { machine_id } = req.query;
  if (machine_id) {
    const result = await pool.query(
      `SELECT s.id, s.machine_id, m.name AS machine_name,
              s.reps, s.rom_mm, s.duration_sec, s.weight_kg, s.timestamp
       FROM sets s LEFT JOIN machines m ON s.machine_id = m.id
       WHERE s.machine_id = $1 ORDER BY s.id DESC`,
      [machine_id]
    );
    res.json(result.rows);
  } else {
    const result = await pool.query(
      `SELECT s.id, s.machine_id, m.name AS machine_name,
              s.reps, s.rom_mm, s.duration_sec, s.weight_kg, s.timestamp
       FROM sets s LEFT JOIN machines m ON s.machine_id = m.id
       ORDER BY s.id DESC`
    );
    res.json(result.rows);
  }
});

module.exports = router;

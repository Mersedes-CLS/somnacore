const { Router } = require("express");
const { pool } = require("../db");

const router = Router();

router.get("/machines", async (_req, res) => {
  const result = await pool.query("SELECT id, name, muscle_group, location FROM machines ORDER BY id");
  res.json(result.rows);
});

router.post("/machines", async (req, res) => {
  const { id, name, muscle_group, location } = req.body;
  if (!id || !name) {
    return res.status(400).json({ error: "id and name are required" });
  }
  try {
    await pool.query(
      "INSERT INTO machines (id, name, muscle_group, location) VALUES ($1, $2, $3, $4)",
      [id, name, muscle_group ?? null, location ?? null]
    );
    res.json({ id, ok: true });
  } catch (err) {
    if (err.code === "23505") {
      return res.status(409).json({ error: "machine already exists" });
    }
    throw err;
  }
});

router.put("/machines/:id", async (req, res) => {
  const { name, muscle_group, location } = req.body;
  const result = await pool.query(
    "UPDATE machines SET name = COALESCE($1, name), muscle_group = COALESCE($2, muscle_group), location = COALESCE($3, location) WHERE id = $4 RETURNING *",
    [name ?? null, muscle_group ?? null, location ?? null, req.params.id]
  );
  if (result.rows.length === 0) {
    return res.status(404).json({ error: "machine not found" });
  }
  res.json({ ...result.rows[0], ok: true });
});

router.get("/machines/:id", async (req, res) => {
  const machine = await pool.query(
    "SELECT id, name, muscle_group, location FROM machines WHERE id = $1",
    [req.params.id]
  );
  if (machine.rows.length === 0) {
    return res.status(404).json({ error: "machine not found" });
  }
  const sets = await pool.query(
    "SELECT id, reps, rom_mm, duration_sec, weight_kg, timestamp FROM sets WHERE machine_id = $1 ORDER BY id DESC",
    [req.params.id]
  );
  res.json({ ...machine.rows[0], sets: sets.rows });
});

router.delete("/machines/:id", async (req, res) => {
  const machine = await pool.query("SELECT id FROM machines WHERE id = $1", [req.params.id]);
  if (machine.rows.length === 0) {
    return res.status(404).json({ error: "machine not found" });
  }
  await pool.query("UPDATE sets SET machine_id = NULL WHERE machine_id = $1", [req.params.id]);
  await pool.query("DELETE FROM machines WHERE id = $1", [req.params.id]);
  res.json({ ok: true });
});

module.exports = router;

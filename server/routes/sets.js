const { Router } = require("express");
const db = require("../db");

const router = Router();

const insertSet = db.prepare(
  "INSERT INTO sets (machine_id, reps, rom_mm, duration_sec, weight_kg) VALUES (?, ?, ?, ?, ?)"
);

const getAllSets = db.prepare(`
  SELECT s.id, s.machine_id, m.name AS machine_name,
         s.reps, s.rom_mm, s.duration_sec, s.weight_kg, s.timestamp
  FROM sets s
  LEFT JOIN machines m ON s.machine_id = m.id
  ORDER BY s.id DESC
`);

const getSetsByMachine = db.prepare(`
  SELECT s.id, s.machine_id, m.name AS machine_name,
         s.reps, s.rom_mm, s.duration_sec, s.weight_kg, s.timestamp
  FROM sets s
  LEFT JOIN machines m ON s.machine_id = m.id
  WHERE s.machine_id = ?
  ORDER BY s.id DESC
`);

router.post("/set", (req, res) => {
  const { machine_id, reps, rom_mm, duration_sec, weight_kg } = req.body;
  if (!reps || typeof reps !== "number") {
    return res.status(400).json({ error: "reps is required (number)" });
  }
  const info = insertSet.run(
    machine_id ?? null,
    reps,
    rom_mm ?? null,
    duration_sec ?? null,
    weight_kg ?? null
  );
  res.json({ id: Number(info.lastInsertRowid), ok: true });
});

router.get("/sets", (req, res) => {
  const { machine_id } = req.query;
  if (machine_id) {
    res.json(getSetsByMachine.all(machine_id));
  } else {
    res.json(getAllSets.all());
  }
});

module.exports = router;

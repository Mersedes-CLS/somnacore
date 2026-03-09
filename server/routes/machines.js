const { Router } = require("express");
const db = require("../db");

const router = Router();

const insertMachine = db.prepare(
  "INSERT INTO machines (id, name, muscle_group, location) VALUES (?, ?, ?, ?)"
);

const getAllMachines = db.prepare(
  "SELECT id, name, muscle_group, location FROM machines ORDER BY id"
);

const getMachineById = db.prepare(
  "SELECT id, name, muscle_group, location FROM machines WHERE id = ?"
);

const getSetsByMachine = db.prepare(
  "SELECT id, reps, rom_mm, duration_sec, weight_kg, timestamp FROM sets WHERE machine_id = ? ORDER BY id DESC"
);

const unlinkSets = db.prepare("UPDATE sets SET machine_id = NULL WHERE machine_id = ?");
const deleteMachine = db.prepare("DELETE FROM machines WHERE id = ?");

router.get("/machines", (_req, res) => {
  res.json(getAllMachines.all());
});

router.post("/machines", (req, res) => {
  const { id, name, muscle_group, location } = req.body;
  if (!id || !name) {
    return res.status(400).json({ error: "id and name are required" });
  }
  try {
    insertMachine.run(id, name, muscle_group ?? null, location ?? null);
    res.json({ id, ok: true });
  } catch (err) {
    if (err.code === "SQLITE_CONSTRAINT_PRIMARYKEY") {
      return res.status(409).json({ error: "machine already exists" });
    }
    throw err;
  }
});

router.get("/machines/:id", (req, res) => {
  const machine = getMachineById.get(req.params.id);
  if (!machine) {
    return res.status(404).json({ error: "machine not found" });
  }
  const sets = getSetsByMachine.all(req.params.id);
  res.json({ ...machine, sets });
});

router.delete("/machines/:id", (req, res) => {
  const machine = getMachineById.get(req.params.id);
  if (!machine) {
    return res.status(404).json({ error: "machine not found" });
  }
  unlinkSets.run(req.params.id);
  deleteMachine.run(req.params.id);
  res.json({ ok: true });
});

module.exports = router;

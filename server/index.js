const express = require("express");
const { insertSet, getAllSets } = require("./db");

const app = express();
const PORT = 3000;

app.use(express.json());

app.get("/health", (_req, res) => {
  res.json({ status: "ok" });
});

app.post("/set", (req, res) => {
  const { reps, rom_mm, duration_sec, weight_kg } = req.body;
  if (!reps || typeof reps !== "number") {
    return res.status(400).json({ error: "reps is required (number)" });
  }
  const info = insertSet.run(
    reps,
    rom_mm ?? null,
    duration_sec ?? null,
    weight_kg ?? null
  );
  res.json({ id: info.lastInsertRowid, ok: true });
});

app.get("/sets", (_req, res) => {
  res.json(getAllSets.all());
});

app.listen(PORT, "0.0.0.0", () => {
  console.log(`LAZER server listening on http://0.0.0.0:${PORT}`);
});

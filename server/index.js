const express = require("express");
const setsRouter = require("./routes/sets");
const machinesRouter = require("./routes/machines");
const statsRouter = require("./routes/stats");

const app = express();
const PORT = 3000;

app.use(express.json());

app.get("/api/health", (_req, res) => {
  res.json({ status: "ok" });
});

app.use("/api", setsRouter);
app.use("/api", machinesRouter);
app.use("/api", statsRouter);

app.listen(PORT, "0.0.0.0", () => {
  console.log(`LAZER server listening on http://0.0.0.0:${PORT}`);
});

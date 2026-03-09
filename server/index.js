require("dotenv").config();
const express = require("express");
const path = require("path");
const { migrate } = require("./db");
const setsRouter = require("./routes/sets");
const machinesRouter = require("./routes/machines");
const statsRouter = require("./routes/stats");

const app = express();
const PORT = process.env.PORT || 3000;

app.use(express.json());

app.get("/api/health", (_req, res) => {
  res.json({ status: "ok" });
});

app.use("/api", setsRouter);
app.use("/api", machinesRouter);
app.use("/api", statsRouter);

app.use(express.static(path.join(__dirname, "public")));

async function start() {
  await migrate();
  console.log("DB migrated");

  app.listen(PORT, "0.0.0.0", () => {
    console.log(`LAZER server listening on http://0.0.0.0:${PORT}`);
    require("./bot").startBot();
  });
}

start().catch((err) => {
  console.error("Failed to start:", err);
  process.exit(1);
});

const { Bot } = require("grammy");

function startBot() {
  const token = process.env.BOT_TOKEN;
  const webappUrl = process.env.WEBAPP_URL;

  if (!token) {
    console.warn("BOT_TOKEN not set — Telegram bot disabled");
    return;
  }

  if (!webappUrl) {
    console.warn("WEBAPP_URL not set — bot will start but /start won't have web app link");
  }

  const bot = new Bot(token);

  bot.command("start", async (ctx) => {
    if (webappUrl) {
      await ctx.reply("Открой трекер тренировок:", {
        reply_markup: {
          inline_keyboard: [
            [{ text: "Открыть LAZER", web_app: { url: webappUrl } }],
          ],
        },
      });
    } else {
      await ctx.reply("LAZER бот работает, но WEBAPP_URL не настроен.");
    }
  });

  bot.start();
  console.log("Telegram bot started (long-polling)");
}

module.exports = { startBot };

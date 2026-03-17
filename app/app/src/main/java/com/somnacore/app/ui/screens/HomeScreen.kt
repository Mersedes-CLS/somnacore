package com.somnacore.app.ui.screens

import androidx.compose.animation.core.*
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.somnacore.app.ui.components.*
import com.somnacore.app.ui.theme.*

@Composable
fun HomeScreen() {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(BgPrimary)
            .verticalScroll(rememberScrollState())
            .padding(bottom = 100.dp)
    ) {
        Spacer(Modifier.height(8.dp))

        // Greeting row
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 20.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column {
                Text(
                    text = "Привет \uD83D\uDC4B",
                    fontSize = 13.sp,
                    color = TextSecondary
                )
                Text(
                    text = "Вова",
                    fontSize = 26.sp,
                    fontWeight = FontWeight.ExtraBold,
                    color = TextPrimary,
                    letterSpacing = (-0.8).sp
                )
            }
            // Streak mini
            val infiniteTransition = rememberInfiniteTransition(label = "streak")
            val streakScale by infiniteTransition.animateFloat(
                initialValue = 1f,
                targetValue = 1.15f,
                animationSpec = infiniteRepeatable(
                    animation = tween(1500, easing = EaseInOut),
                    repeatMode = RepeatMode.Reverse
                ),
                label = "streakScale"
            )
            Text(
                text = "\uD83D\uDD25 7",
                fontSize = 16.sp,
                fontWeight = FontWeight.ExtraBold,
                color = Orange,
                modifier = Modifier
                    .clip(RoundedCornerShape(RadiusSm))
                    .background(OrangeGlow)
                    .padding(horizontal = 12.dp, vertical = 6.dp)
            )
        }

        Spacer(Modifier.height(14.dp))

        // Power hero card
        PowerHeroCard()

        Spacer(Modifier.height(4.dp))

        // Streak strip
        StreakStrip()

        Spacer(Modifier.height(4.dp))

        // Rank bar
        RankBar()

        Spacer(Modifier.height(14.dp))

        // XP bar
        XpBar()

        Spacer(Modifier.height(14.dp))

        // Quick metrics
        SectionHeader(title = "Сегодня", action = "Подробнее")
        Spacer(Modifier.height(10.dp))

        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 20.dp),
            horizontalArrangement = Arrangement.spacedBy(7.dp)
        ) {
            MetricCard(
                icon = "⚡",
                value = "342",
                label = "ккал",
                iconBg = Brush.linearGradient(listOf(Color(0xFF059669), Green)),
                modifier = Modifier.weight(1f)
            )
            MetricCard(
                icon = "\uD83C\uDFCB\uFE0F",
                value = "4",
                label = "подхода",
                iconBg = Brush.linearGradient(listOf(Accent, Color(0xFFA855F7))),
                modifier = Modifier.weight(1f)
            )
            MetricCard(
                icon = "⏱",
                value = "47",
                label = "минут",
                iconBg = Brush.linearGradient(listOf(Pink, Color(0xFFF472B6))),
                modifier = Modifier.weight(1f)
            )
        }

        Spacer(Modifier.height(14.dp))

        // Today's exercises
        SectionHeader(title = "Упражнения")
        Spacer(Modifier.height(10.dp))

        SetCard(
            icon = "\uD83E\uDDBE",
            iconBg = Brush.linearGradient(listOf(Accent, Color(0xFFA855F7))),
            name = "Жим лёжа",
            detail = "4 × 10 · 3 мин назад",
            weight = "80 кг",
            calories = "48 ккал",
            isPR = true
        )
        Spacer(Modifier.height(6.dp))
        SetCard(
            icon = "\uD83E\uDDB5",
            iconBg = Brush.linearGradient(listOf(Color(0xFF059669), Green)),
            name = "Присед",
            detail = "3 × 12 · 18 мин назад",
            weight = "100 кг",
            calories = "62 ккал"
        )
        Spacer(Modifier.height(6.dp))
        SetCard(
            icon = "\uD83D\uDCAA",
            iconBg = Brush.linearGradient(listOf(Pink, Color(0xFFF472B6))),
            name = "Тяга верхнего блока",
            detail = "3 × 10 · 32 мин назад",
            weight = "55 кг",
            calories = "38 ккал"
        )

        Spacer(Modifier.height(14.dp))

        // Motivation
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 20.dp)
                .clip(RoundedCornerShape(RadiusSm))
                .background(BgCard)
                .border(1.dp, Border, RoundedCornerShape(RadiusSm))
                .padding(10.dp, 10.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(10.dp)
        ) {
            Text(text = "\uD83D\uDCAC", fontSize = 20.sp)
            Text(
                text = "Ты на 12% сильнее, чем неделю назад. Так держать!",
                fontSize = 12.sp,
                fontWeight = FontWeight.Medium,
                color = TextSecondary,
                lineHeight = 17.sp
            )
        }

        Spacer(Modifier.height(14.dp))

        // Leaderboard
        LeaderboardCard()
    }
}

@Composable
private fun PowerHeroCard() {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 20.dp)
            .clip(RoundedCornerShape(RadiusXl))
            .background(BgCard)
            .border(1.dp, Border, RoundedCornerShape(RadiusXl))
            .padding(24.dp, 20.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(24.dp)
    ) {
        // Ring with score
        Box(contentAlignment = Alignment.Center) {
            ProgressRing(progress = 0.75f, size = 120.dp)
            Column(horizontalAlignment = Alignment.CenterHorizontally) {
                Text(
                    text = "1,247",
                    fontSize = 32.sp,
                    fontWeight = FontWeight.Black,
                    color = TextPrimary,
                    letterSpacing = (-1.5).sp
                )
                Text(
                    text = "POWER",
                    fontSize = 9.sp,
                    fontWeight = FontWeight.Bold,
                    color = AccentLight,
                    letterSpacing = 2.sp
                )
            }
        }

        // Stats
        Column {
            Text(
                text = "↑ +23",
                fontSize = 12.sp,
                fontWeight = FontWeight.Bold,
                color = Green,
                modifier = Modifier
                    .clip(RoundedCornerShape(6.dp))
                    .background(GreenGlow)
                    .padding(horizontal = 8.dp, vertical = 3.dp)
            )
            Spacer(Modifier.height(12.dp))
            Row(horizontalArrangement = Arrangement.spacedBy(14.dp)) {
                Column {
                    Text("Объём", fontSize = 11.sp, color = TextSecondary)
                    Text(
                        text = "4,230",
                        fontSize = 20.sp,
                        fontWeight = FontWeight.ExtraBold,
                        color = TextPrimary,
                        letterSpacing = (-0.5).sp
                    )
                }
                Column {
                    Text("Подходы", fontSize = 11.sp, color = TextSecondary)
                    Text(
                        text = "12",
                        fontSize = 20.sp,
                        fontWeight = FontWeight.ExtraBold,
                        color = TextPrimary,
                        letterSpacing = (-0.5).sp
                    )
                }
            }
        }
    }
}

@Composable
private fun StreakStrip() {
    val days = listOf("Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс")
    val done = listOf(true, true, true, true, true, true, false)
    val today = 6 // Sunday (0-indexed)

    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 20.dp, vertical = 4.dp),
        horizontalArrangement = Arrangement.spacedBy(4.dp),
        verticalAlignment = Alignment.CenterVertically
    ) {
        days.forEachIndexed { index, day ->
            val isDone = done[index]
            val isToday = index == today

            Box(
                modifier = Modifier
                    .weight(1f)
                    .height(22.dp)
                    .clip(RoundedCornerShape(6.dp))
                    .then(
                        if (isDone) {
                            Modifier.background(
                                Brush.linearGradient(listOf(Orange, Color(0xFFF97316)))
                            )
                        } else if (isToday) {
                            Modifier
                                .background(OrangeGlow)
                                .border(2.dp, Orange, RoundedCornerShape(6.dp))
                        } else {
                            Modifier.background(BgElevated)
                        }
                    ),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = day,
                    fontSize = 8.sp,
                    fontWeight = FontWeight.Bold,
                    color = if (isDone) Color.White
                    else if (isToday) Orange
                    else TextMuted
                )
            }
        }
        Spacer(Modifier.width(6.dp))
        Text(
            text = "6 дн.",
            fontSize = 10.sp,
            color = TextMuted,
            fontWeight = FontWeight.Medium
        )
    }
}

@Composable
private fun RankBar() {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 20.dp)
            .clip(RoundedCornerShape(RadiusSm))
            .background(BgCard)
            .border(1.dp, Border, RoundedCornerShape(RadiusSm))
            .clickable { }
            .padding(10.dp, 10.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(8.dp)
    ) {
        Text(
            text = "#4",
            fontSize = 16.sp,
            fontWeight = FontWeight.Black,
            color = AccentLight
        )
        Text(
            text = "в рейтинге зала",
            fontSize = 12.sp,
            color = TextSecondary,
            fontWeight = FontWeight.Medium
        )
        Spacer(Modifier.weight(1f))
        Text(
            text = "до #3 — 82 очка",
            fontSize = 11.sp,
            color = TextMuted,
            fontWeight = FontWeight.Medium
        )
        Text(
            text = "›",
            fontSize = 12.sp,
            fontWeight = FontWeight.Bold,
            color = AccentLight
        )
    }
}

@Composable
private fun XpBar() {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 20.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(10.dp)
    ) {
        // Level badge
        Box(
            modifier = Modifier
                .size(34.dp)
                .clip(RoundedCornerShape(RadiusSm))
                .background(Brush.linearGradient(listOf(Accent, Pink))),
            contentAlignment = Alignment.Center
        ) {
            Text(
                text = "12",
                fontSize = 13.sp,
                fontWeight = FontWeight.Black,
                color = Color.White
            )
        }

        Column(modifier = Modifier.weight(1f)) {
            Text(
                text = "1,230 / 2,000 XP",
                fontSize = 10.sp,
                fontWeight = FontWeight.SemiBold,
                color = TextSecondary
            )
            Spacer(Modifier.height(4.dp))
            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .height(5.dp)
                    .clip(RoundedCornerShape(3.dp))
                    .background(BgElevated)
            ) {
                Box(
                    modifier = Modifier
                        .fillMaxWidth(0.65f)
                        .fillMaxHeight()
                        .clip(RoundedCornerShape(3.dp))
                        .background(Brush.horizontalGradient(listOf(Accent, Pink)))
                )
            }
        }

        Text(
            text = "LVL 13",
            fontSize = 9.sp,
            fontWeight = FontWeight.Bold,
            color = TextMuted
        )
    }
}

@Composable
private fun LeaderboardCard() {
    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 20.dp)
            .clip(RoundedCornerShape(RadiusLg))
            .background(BgCard)
            .border(1.dp, Border, RoundedCornerShape(RadiusLg))
    ) {
        // Header
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(12.dp, 12.dp, 12.dp, 8.dp),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Text(
                text = "Рейтинг зала",
                fontSize = 13.sp,
                fontWeight = FontWeight.Bold,
                color = TextPrimary
            )
            Text(
                text = "Топ 10",
                fontSize = 10.sp,
                fontWeight = FontWeight.Bold,
                color = AccentLight,
                modifier = Modifier
                    .clip(RoundedCornerShape(6.dp))
                    .background(Color(0x1A_7C3AED))
                    .padding(horizontal = 8.dp, vertical = 3.dp)
            )
        }

        val leaders = listOf(
            Triple("1", "Дмитрий К.", "2,847"),
            Triple("2", "Анна С.", "2,391"),
            Triple("3", "Максим Р.", "1,955"),
            Triple("4", "Вова", "1,247"),
        )

        leaders.forEachIndexed { index, (rank, name, score) ->
            val isMe = rank == "4"
            val rankColor = when (rank) {
                "1" -> Gold
                "2" -> Color(0xFF94A3B8)
                "3" -> Color(0xFFCD7F32)
                else -> TextMuted
            }
            val avatarColors = when (index) {
                0 -> listOf(Color(0xFF059669), Green)
                1 -> listOf(Pink, Color(0xFFF472B6))
                2 -> listOf(Color(0xFF0891B2), Cyan)
                else -> listOf(Accent, Color(0xFFA855F7))
            }

            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .then(
                        if (isMe) Modifier.background(Color(0x0F_7C3AED))
                        else Modifier
                    )
                    .padding(horizontal = 14.dp, vertical = 8.dp),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(10.dp)
            ) {
                Text(
                    text = rank,
                    fontSize = 12.sp,
                    fontWeight = FontWeight.ExtraBold,
                    color = rankColor,
                    modifier = Modifier.width(22.dp)
                )
                Box(
                    modifier = Modifier
                        .size(28.dp)
                        .clip(RoundedCornerShape(8.dp))
                        .background(Brush.linearGradient(avatarColors)),
                    contentAlignment = Alignment.Center
                ) {
                    Text(
                        text = name.first().toString(),
                        fontSize = 12.sp,
                        fontWeight = FontWeight.Bold,
                        color = Color.White
                    )
                }
                Text(
                    text = name,
                    fontSize = 12.sp,
                    fontWeight = FontWeight.SemiBold,
                    color = TextPrimary,
                    modifier = Modifier.weight(1f)
                )
                Text(
                    text = score,
                    fontSize = 12.sp,
                    fontWeight = FontWeight.ExtraBold,
                    color = AccentLight
                )
            }
        }
    }
}

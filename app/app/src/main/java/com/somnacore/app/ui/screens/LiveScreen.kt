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
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.somnacore.app.ui.components.*
import com.somnacore.app.ui.theme.*

@Composable
fun LiveScreen() {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(BgPrimary)
            .verticalScroll(rememberScrollState())
            .padding(bottom = 100.dp)
    ) {
        Spacer(Modifier.height(8.dp))

        // Live status
        Column(
            modifier = Modifier.fillMaxWidth(),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Row(
                modifier = Modifier
                    .clip(RoundedCornerShape(20.dp))
                    .background(GreenGlow)
                    .border(1.dp, Color(0x40_10B981), RoundedCornerShape(20.dp))
                    .padding(horizontal = 12.dp, vertical = 4.dp),
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(6.dp)
            ) {
                PulsingDot(Green, 6.dp)
                Text(
                    text = "Тренировка идёт",
                    fontSize = 11.sp,
                    fontWeight = FontWeight.SemiBold,
                    color = Green
                )
            }

            Spacer(Modifier.height(12.dp))

            Text(
                text = "ТРЕНАЖЁР",
                fontSize = 11.sp,
                fontWeight = FontWeight.Bold,
                color = AccentLight,
                letterSpacing = 2.sp
            )

            Text(
                text = "Жим лёжа",
                fontSize = 24.sp,
                fontWeight = FontWeight.ExtraBold,
                color = TextPrimary,
                letterSpacing = (-0.8).sp
            )
        }

        Spacer(Modifier.height(24.dp))

        // Big ring with reps
        Box(
            modifier = Modifier.fillMaxWidth(),
            contentAlignment = Alignment.Center
        ) {
            Box(contentAlignment = Alignment.Center) {
                ProgressRing(
                    progress = 0.67f,
                    size = 200.dp,
                    strokeWidth = 7.dp
                )
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    Text(
                        text = "8",
                        fontSize = 64.sp,
                        fontWeight = FontWeight.Black,
                        color = TextPrimary,
                        letterSpacing = (-3).sp
                    )
                    Text(
                        text = "повторений",
                        fontSize = 12.sp,
                        color = TextSecondary,
                        fontWeight = FontWeight.Medium
                    )
                    Text(
                        text = "цель: 12",
                        fontSize = 10.sp,
                        color = TextMuted
                    )
                }
            }
        }

        Spacer(Modifier.height(24.dp))

        // Stats numbers row
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 20.dp),
            verticalAlignment = Alignment.CenterVertically
        ) {
            LiveNum("80", "кг", Modifier.weight(1f))
            Box(
                modifier = Modifier
                    .width(1.dp)
                    .height(28.dp)
                    .background(Border)
            )
            LiveNum("48", "ккал", Modifier.weight(1f))
            Box(
                modifier = Modifier
                    .width(1.dp)
                    .height(28.dp)
                    .background(Border)
            )
            LiveNum("2:34", "время", Modifier.weight(1f))
        }

        Spacer(Modifier.height(16.dp))

        // Comparison
        Box(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 20.dp)
                .clip(RoundedCornerShape(RadiusSm))
                .background(BgCard)
                .border(1.dp, Border, RoundedCornerShape(RadiusSm))
                .padding(8.dp, 8.dp),
            contentAlignment = Alignment.Center
        ) {
            Text(
                text = "Прошлый раз: 75 кг × 10. Сейчас +5 кг \uD83D\uDCAA",
                fontSize = 11.sp,
                color = TextSecondary,
                fontWeight = FontWeight.Medium,
                textAlign = TextAlign.Center
            )
        }

        Spacer(Modifier.height(16.dp))

        // Timeline
        SectionHeader(title = "Подходы")
        Spacer(Modifier.height(10.dp))

        TimelineItem(num = "4", text = "80 кг × 8", cal = "14 ккал", isActive = true)
        TimelineRest("отдых 1:45")
        TimelineItem(num = "3", text = "80 кг × 10", cal = "16 ккал")
        TimelineRest("отдых 2:00")
        TimelineItem(num = "2", text = "75 кг × 10", cal = "15 ккал")
        TimelineRest("отдых 1:30")
        TimelineItem(num = "1", text = "70 кг × 12", cal = "18 ккал")

        Spacer(Modifier.height(16.dp))

        // PR card
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 20.dp)
                .clip(RoundedCornerShape(RadiusMd))
                .background(
                    Brush.linearGradient(
                        listOf(Color(0x1A_FBBF24), Color(0x0A_F97316))
                    )
                )
                .border(1.dp, Color(0x33_FBBF24), RoundedCornerShape(RadiusMd))
                .padding(12.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(10.dp)
        ) {
            Text(text = "\uD83C\uDFC6", fontSize = 22.sp)
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text = "Новый рекорд!",
                    fontSize = 12.sp,
                    fontWeight = FontWeight.Bold,
                    color = Gold
                )
                Text(
                    text = "Жим лёжа — максимальный вес",
                    fontSize = 10.sp,
                    color = TextSecondary
                )
            }
            Text(
                text = "80 кг",
                fontSize = 16.sp,
                fontWeight = FontWeight.ExtraBold,
                color = TextPrimary
            )
        }
    }
}

@Composable
private fun LiveNum(value: String, unit: String, modifier: Modifier = Modifier) {
    Column(
        modifier = modifier,
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Text(
            text = value,
            fontSize = 28.sp,
            fontWeight = FontWeight.Black,
            color = TextPrimary,
            letterSpacing = (-1).sp
        )
        Text(
            text = unit,
            fontSize = 10.sp,
            fontWeight = FontWeight.SemiBold,
            color = TextSecondary
        )
    }
}

@Composable
private fun TimelineItem(
    num: String,
    text: String,
    cal: String,
    isActive: Boolean = false
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 20.dp, vertical = 8.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(10.dp)
    ) {
        Box(
            modifier = Modifier
                .size(20.dp)
                .clip(CircleShape)
                .background(if (isActive) Accent else BgElevated),
            contentAlignment = Alignment.Center
        ) {
            Text(
                text = num,
                fontSize = 10.sp,
                fontWeight = FontWeight.ExtraBold,
                color = if (isActive) Color.White else TextMuted
            )
        }
        Text(
            text = text,
            fontSize = 13.sp,
            fontWeight = FontWeight.SemiBold,
            color = TextPrimary,
            modifier = Modifier.weight(1f)
        )
        Text(
            text = cal,
            fontSize = 11.sp,
            fontWeight = FontWeight.Bold,
            color = Orange
        )
    }
}

@Composable
private fun TimelineRest(text: String) {
    Text(
        text = "⏸ $text",
        fontSize = 10.sp,
        color = TextMuted,
        fontWeight = FontWeight.Medium,
        modifier = Modifier.padding(start = 50.dp, top = 2.dp, bottom = 2.dp)
    )
}

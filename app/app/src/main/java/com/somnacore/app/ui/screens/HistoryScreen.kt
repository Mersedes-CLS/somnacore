package com.somnacore.app.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.horizontalScroll
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
fun HistoryScreen() {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(BgPrimary)
            .verticalScroll(rememberScrollState())
            .padding(bottom = 100.dp)
    ) {
        Spacer(Modifier.height(8.dp))

        Text(
            text = "История",
            fontSize = 26.sp,
            fontWeight = FontWeight.ExtraBold,
            color = TextPrimary,
            letterSpacing = (-0.8).sp,
            modifier = Modifier.padding(horizontal = 20.dp)
        )

        Spacer(Modifier.height(10.dp))

        // Calendar strip
        CalendarStrip()

        Spacer(Modifier.height(10.dp))

        // Day summary
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 20.dp)
                .clip(RoundedCornerShape(RadiusLg))
                .background(BgCard)
                .border(1.dp, Border, RoundedCornerShape(RadiusLg))
                .padding(14.dp),
            horizontalArrangement = Arrangement.SpaceAround
        ) {
            DaySummaryItem("4", "подхода")
            DaySummaryItem("342", "ккал")
            DaySummaryItem("47", "минут")
            DaySummaryItem("3,280", "объём")
        }

        Spacer(Modifier.height(14.dp))

        SectionHeader(title = "Упражнения")
        Spacer(Modifier.height(10.dp))

        HistoryCard(
            icon = "\uD83E\uDDBE",
            name = "Жим лёжа",
            sets = "4 подхода",
            time = "12:34",
            calories = "48",
            isPR = true
        )
        Spacer(Modifier.height(6.dp))
        HistoryCard(
            icon = "\uD83E\uDDB5",
            name = "Присед",
            sets = "3 подхода",
            time = "12:18",
            calories = "62"
        )
        Spacer(Modifier.height(6.dp))
        HistoryCard(
            icon = "\uD83D\uDCAA",
            name = "Тяга верхнего блока",
            sets = "3 подхода",
            time = "11:52",
            calories = "38"
        )
        Spacer(Modifier.height(6.dp))
        HistoryCard(
            icon = "\uD83E\uDDBE",
            name = "Разгибание на трицепс",
            sets = "3 подхода",
            time = "11:30",
            calories = "28"
        )
    }
}

@Composable
private fun CalendarStrip() {
    val days = listOf(
        "Пн" to "10", "Вт" to "11", "Ср" to "12", "Чт" to "13",
        "Пт" to "14", "Сб" to "15", "Вс" to "16"
    )
    val activeIndex = 4 // Friday
    val hasDot = listOf(true, false, true, true, true, true, false)

    Row(
        modifier = Modifier
            .fillMaxWidth()
            .horizontalScroll(rememberScrollState())
            .padding(horizontal = 20.dp),
        horizontalArrangement = Arrangement.spacedBy(5.dp)
    ) {
        days.forEachIndexed { index, (dayName, dayNum) ->
            val isActive = index == activeIndex

            Column(
                modifier = Modifier
                    .width(40.dp)
                    .clip(RoundedCornerShape(RadiusSm))
                    .then(
                        if (isActive) {
                            Modifier
                                .background(Accent)
                                .border(1.dp, Accent, RoundedCornerShape(RadiusSm))
                        } else {
                            Modifier
                                .background(BgCard)
                                .border(1.dp, Border, RoundedCornerShape(RadiusSm))
                        }
                    )
                    .clickable { }
                    .padding(vertical = 7.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Text(
                    text = dayName.uppercase(),
                    fontSize = 8.sp,
                    fontWeight = FontWeight.Bold,
                    color = if (isActive) Color.White else TextMuted,
                    letterSpacing = 0.5.sp
                )
                Text(
                    text = dayNum,
                    fontSize = 15.sp,
                    fontWeight = FontWeight.Bold,
                    color = if (isActive) Color.White else TextPrimary
                )
                Spacer(Modifier.height(3.dp))
                Box(
                    modifier = Modifier
                        .size(4.dp)
                        .clip(CircleShape)
                        .background(
                            if (hasDot[index]) {
                                if (isActive) Color.White else Accent
                            } else Color.Transparent
                        )
                )
            }
        }
    }
}

@Composable
private fun DaySummaryItem(value: String, label: String) {
    Column(horizontalAlignment = Alignment.CenterHorizontally) {
        Text(
            text = value,
            fontSize = 18.sp,
            fontWeight = FontWeight.ExtraBold,
            color = TextPrimary,
            letterSpacing = (-0.5).sp
        )
        Text(
            text = label.uppercase(),
            fontSize = 9.sp,
            fontWeight = FontWeight.SemiBold,
            color = TextSecondary,
            letterSpacing = 0.3.sp
        )
    }
}

@Composable
private fun HistoryCard(
    icon: String,
    name: String,
    sets: String,
    time: String,
    calories: String,
    isPR: Boolean = false
) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 20.dp)
            .clip(RoundedCornerShape(RadiusMd))
            .background(BgCard)
            .border(1.dp, Border, RoundedCornerShape(RadiusMd))
            .clickable { }
            .padding(11.dp),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(11.dp)
    ) {
        Box(
            modifier = Modifier
                .size(38.dp)
                .clip(RoundedCornerShape(RadiusSm))
                .background(
                    Brush.linearGradient(
                        listOf(Color(0x1F_7C3AED), Color(0x0F_EC4899))
                    )
                ),
            contentAlignment = Alignment.Center
        ) {
            Text(text = icon, fontSize = 17.sp)
        }
        Column(modifier = Modifier.weight(1f)) {
            Row(verticalAlignment = Alignment.CenterVertically) {
                Text(
                    text = name,
                    fontSize = 13.sp,
                    fontWeight = FontWeight.SemiBold,
                    color = TextPrimary
                )
                if (isPR) {
                    Spacer(Modifier.width(5.dp))
                    Box(
                        modifier = Modifier
                            .clip(RoundedCornerShape(5.dp))
                            .background(Brush.linearGradient(listOf(Gold, Color(0xFFF97316))))
                            .padding(horizontal = 5.dp, vertical = 2.dp)
                    ) {
                        Text("PR", fontSize = 8.sp, fontWeight = FontWeight.ExtraBold, color = Color.Black)
                    }
                }
            }
            Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                Text(text = sets, fontSize = 11.sp, color = TextSecondary)
                Text(text = time, fontSize = 11.sp, color = TextSecondary)
            }
        }
        Column(horizontalAlignment = Alignment.End) {
            Text(
                text = calories,
                fontSize = 13.sp,
                fontWeight = FontWeight.ExtraBold,
                color = Orange
            )
            Text(
                text = "ккал",
                fontSize = 9.sp,
                color = TextSecondary
            )
        }
    }
}

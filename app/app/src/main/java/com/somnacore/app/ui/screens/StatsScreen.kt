package com.somnacore.app.ui.screens

import androidx.compose.animation.core.*
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
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
fun StatsScreen() {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(BgPrimary)
            .verticalScroll(rememberScrollState())
            .padding(bottom = 100.dp)
    ) {
        Spacer(Modifier.height(8.dp))

        Text(
            text = "Статистика",
            fontSize = 26.sp,
            fontWeight = FontWeight.ExtraBold,
            color = TextPrimary,
            letterSpacing = (-0.8).sp,
            modifier = Modifier.padding(horizontal = 20.dp)
        )

        Spacer(Modifier.height(6.dp))

        // Period buttons
        PeriodSelector()

        Spacer(Modifier.height(12.dp))

        // Volume chart
        ChartCard(
            title = "ОБЩИЙ ОБЪЁМ",
            value = "24,530 кг",
            change = "+12%",
            isUp = true,
            barHeights = listOf(0.4f, 0.6f, 0.35f, 0.7f, 0.55f, 0.8f, 0.65f)
        )

        Spacer(Modifier.height(7.dp))

        // Calories chart
        ChartCard(
            title = "КАЛОРИИ",
            value = "2,140",
            change = "+8%",
            isUp = true,
            barHeights = listOf(0.5f, 0.3f, 0.6f, 0.45f, 0.7f, 0.6f, 0.9f)
        )

        Spacer(Modifier.height(14.dp))

        // Stats grid
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 20.dp),
            horizontalArrangement = Arrangement.spacedBy(7.dp)
        ) {
            StatGridCard(
                icon = "\uD83C\uDFCB\uFE0F",
                value = "28",
                label = "Тренировок",
                change = "+4",
                isUp = true,
                modifier = Modifier.weight(1f)
            )
            StatGridCard(
                icon = "⚡",
                value = "8,420",
                label = "Калорий",
                change = "+12%",
                isUp = true,
                modifier = Modifier.weight(1f)
            )
        }
        Spacer(Modifier.height(7.dp))
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 20.dp),
            horizontalArrangement = Arrangement.spacedBy(7.dp)
        ) {
            StatGridCard(
                icon = "\uD83C\uDFC6",
                value = "7",
                label = "Рекордов",
                change = "+2",
                isUp = true,
                modifier = Modifier.weight(1f)
            )
            StatGridCard(
                icon = "\uD83D\uDD25",
                value = "14",
                label = "Серия дней",
                change = "-",
                isUp = true,
                modifier = Modifier.weight(1f)
            )
        }
    }
}

@Composable
private fun PeriodSelector() {
    var selected by remember { mutableIntStateOf(1) }
    val periods = listOf("День", "Неделя", "Месяц", "Год")

    Row(
        modifier = Modifier.padding(horizontal = 20.dp, vertical = 6.dp),
        horizontalArrangement = Arrangement.spacedBy(5.dp)
    ) {
        periods.forEachIndexed { index, period ->
            val isActive = index == selected
            Text(
                text = period,
                fontSize = 11.sp,
                fontWeight = FontWeight.Bold,
                color = if (isActive) Color.White else TextMuted,
                modifier = Modifier
                    .clip(RoundedCornerShape(RadiusSm))
                    .then(
                        if (isActive) {
                            Modifier.background(Accent)
                        } else {
                            Modifier
                                .background(Color.Transparent)
                                .border(1.dp, Border, RoundedCornerShape(RadiusSm))
                        }
                    )
                    .clickable { selected = index }
                    .padding(horizontal = 12.dp, vertical = 6.dp)
            )
        }
    }
}

@Composable
private fun ChartCard(
    title: String,
    value: String,
    change: String,
    isUp: Boolean,
    barHeights: List<Float>
) {
    val days = listOf("Пн", "Вт", "Ср", "Чт", "Пт", "Сб", "Вс")

    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(horizontal = 20.dp)
            .clip(RoundedCornerShape(RadiusLg))
            .background(BgCard)
            .border(1.dp, Border, RoundedCornerShape(RadiusLg))
            .padding(16.dp)
    ) {
        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.Top
        ) {
            Column {
                Text(
                    text = title,
                    fontSize = 11.sp,
                    fontWeight = FontWeight.SemiBold,
                    color = TextSecondary,
                    letterSpacing = 0.5.sp
                )
                Text(
                    text = value,
                    fontSize = 22.sp,
                    fontWeight = FontWeight.ExtraBold,
                    color = TextPrimary,
                    letterSpacing = (-0.5).sp
                )
            }
            Text(
                text = change,
                fontSize = 10.sp,
                fontWeight = FontWeight.Bold,
                color = if (isUp) Green else Red,
                modifier = Modifier
                    .clip(RoundedCornerShape(6.dp))
                    .background(if (isUp) GreenGlow else RedGlow)
                    .padding(horizontal = 8.dp, vertical = 3.dp)
            )
        }

        Spacer(Modifier.height(12.dp))

        // Bar chart
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .height(110.dp),
            horizontalArrangement = Arrangement.spacedBy(4.dp),
            verticalAlignment = Alignment.Bottom
        ) {
            barHeights.forEachIndexed { index, height ->
                val isToday = index == barHeights.lastIndex

                Column(
                    modifier = Modifier.weight(1f),
                    horizontalAlignment = Alignment.CenterHorizontally,
                    verticalArrangement = Arrangement.Bottom
                ) {
                    val animatedHeight by animateFloatAsState(
                        targetValue = height,
                        animationSpec = tween(1000, easing = FastOutSlowInEasing),
                        label = "bar$index"
                    )

                    Box(
                        modifier = Modifier
                            .fillMaxWidth(0.7f)
                            .fillMaxHeight(animatedHeight)
                            .clip(RoundedCornerShape(topStart = 5.dp, topEnd = 5.dp))
                            .background(
                                Brush.verticalGradient(
                                    listOf(Color(0xFFA855F7), Accent)
                                )
                            )
                    )

                    Spacer(Modifier.height(4.dp))

                    Text(
                        text = days[index],
                        fontSize = 9.sp,
                        fontWeight = FontWeight.SemiBold,
                        color = TextMuted
                    )
                }
            }
        }
    }
}

@Composable
private fun StatGridCard(
    icon: String,
    value: String,
    label: String,
    change: String,
    isUp: Boolean,
    modifier: Modifier = Modifier
) {
    Column(
        modifier = modifier
            .clip(RoundedCornerShape(RadiusMd))
            .background(BgCard)
            .border(1.dp, Border, RoundedCornerShape(RadiusMd))
            .clickable { }
            .padding(14.dp, 12.dp)
    ) {
        Text(text = icon, fontSize = 15.sp)
        Spacer(Modifier.height(6.dp))
        Text(
            text = value,
            fontSize = 22.sp,
            fontWeight = FontWeight.ExtraBold,
            color = TextPrimary,
            letterSpacing = (-0.5).sp
        )
        Text(
            text = label,
            fontSize = 10.sp,
            color = TextSecondary,
            fontWeight = FontWeight.Medium
        )
        Spacer(Modifier.height(5.dp))
        Text(
            text = change,
            fontSize = 9.sp,
            fontWeight = FontWeight.ExtraBold,
            color = if (isUp) Green else Red,
            modifier = Modifier
                .clip(RoundedCornerShape(5.dp))
                .background(if (isUp) GreenGlow else RedGlow)
                .padding(horizontal = 6.dp, vertical = 2.dp)
        )
    }
}

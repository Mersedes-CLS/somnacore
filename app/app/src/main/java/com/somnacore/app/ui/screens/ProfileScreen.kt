package com.somnacore.app.ui.screens

import androidx.compose.animation.core.*
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
import androidx.compose.ui.draw.scale
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.somnacore.app.ui.components.*
import com.somnacore.app.ui.theme.*

@Composable
fun ProfileScreen() {
    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(BgPrimary)
            .verticalScroll(rememberScrollState())
            .padding(bottom = 100.dp)
    ) {
        Spacer(Modifier.height(12.dp))

        // Profile header
        Column(
            modifier = Modifier.fillMaxWidth(),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            // Avatar
            Box(contentAlignment = Alignment.BottomEnd) {
                val infiniteTransition = rememberInfiniteTransition(label = "avatar")
                val avatarScale by infiniteTransition.animateFloat(
                    initialValue = 1f,
                    targetValue = 1.05f,
                    animationSpec = infiniteRepeatable(
                        animation = tween(2000, easing = EaseInOut),
                        repeatMode = RepeatMode.Reverse
                    ),
                    label = "avatarScale"
                )

                Box(
                    modifier = Modifier
                        .size(80.dp)
                        .scale(avatarScale)
                        .clip(CircleShape)
                        .background(Brush.linearGradient(listOf(Accent, Pink))),
                    contentAlignment = Alignment.Center
                ) {
                    Text(text = "\uD83D\uDE0E", fontSize = 32.sp)
                }

                // Level badge
                Box(
                    modifier = Modifier
                        .size(26.dp)
                        .clip(RoundedCornerShape(7.dp))
                        .background(Brush.linearGradient(listOf(Gold, Color(0xFFF97316))))
                        .border(2.dp, BgPrimary, RoundedCornerShape(7.dp)),
                    contentAlignment = Alignment.Center
                ) {
                    Text(
                        text = "12",
                        fontSize = 10.sp,
                        fontWeight = FontWeight.Black,
                        color = Color.Black
                    )
                }
            }

            Spacer(Modifier.height(10.dp))

            Text(
                text = "Вова",
                fontSize = 24.sp,
                fontWeight = FontWeight.ExtraBold,
                color = TextPrimary,
                letterSpacing = (-0.8).sp
            )
            Text(
                text = "Участник с марта 2026",
                fontSize = 11.sp,
                color = TextSecondary
            )

            Spacer(Modifier.height(16.dp))

            // Stats row
            Row(
                horizontalArrangement = Arrangement.spacedBy(24.dp)
            ) {
                ProfileStat("128", "Тренировок")
                ProfileStat("42,840", "Калорий")
                ProfileStat("14", "Рекордов")
            }
        }

        Spacer(Modifier.height(14.dp))

        // NFC band card
        Row(
            modifier = Modifier
                .fillMaxWidth()
                .padding(horizontal = 20.dp)
                .clip(RoundedCornerShape(RadiusLg))
                .background(
                    Brush.linearGradient(
                        listOf(Color(0x14_7C3AED), Color(0x0A_EC4899))
                    )
                )
                .border(1.dp, BorderAccent, RoundedCornerShape(RadiusLg))
                .clickable { }
                .padding(14.dp, 14.dp),
            verticalAlignment = Alignment.CenterVertically,
            horizontalArrangement = Arrangement.spacedBy(12.dp)
        ) {
            Box(
                modifier = Modifier
                    .size(40.dp)
                    .clip(RoundedCornerShape(RadiusSm))
                    .background(Brush.linearGradient(listOf(Accent, Pink))),
                contentAlignment = Alignment.Center
            ) {
                Text(text = "📡", fontSize = 18.sp)
            }
            Column(modifier = Modifier.weight(1f)) {
                Text(
                    text = "SomnaBand",
                    fontSize = 13.sp,
                    fontWeight = FontWeight.Bold,
                    color = TextPrimary
                )
                Text(
                    text = "SB-2024-0847",
                    fontSize = 10.sp,
                    color = TextSecondary
                )
            }
            Row(
                verticalAlignment = Alignment.CenterVertically,
                horizontalArrangement = Arrangement.spacedBy(4.dp)
            ) {
                PulsingDot(Green, 5.dp)
                Text(
                    text = "Online",
                    fontSize = 10.sp,
                    fontWeight = FontWeight.Bold,
                    color = Green
                )
            }
        }

        Spacer(Modifier.height(14.dp))

        // Achievements
        SectionHeader(title = "Достижения", action = "Все")
        Spacer(Modifier.height(10.dp))

        Row(
            modifier = Modifier
                .fillMaxWidth()
                .horizontalScroll(rememberScrollState())
                .padding(horizontal = 20.dp),
            horizontalArrangement = Arrangement.spacedBy(6.dp)
        ) {
            AchievementChip("\uD83D\uDD25", "7 дней", unlocked = true)
            AchievementChip("⚡", "1000 ккал", unlocked = true)
            AchievementChip("\uD83C\uDFC6", "Первый PR", unlocked = true)
            AchievementChip("\uD83D\uDCAA", "Силач", unlocked = true)
            AchievementChip("\uD83C\uDF1F", "Марафон", unlocked = false)
            AchievementChip("\uD83D\uDC8E", "Легенда", unlocked = false)
        }

        Spacer(Modifier.height(14.dp))

        // Settings
        SettingsSection("Приложение") {
            SettingsItem("🔔", "Уведомления", trailing = "Вкл")
            SettingsItem("🎨", "Тема", trailing = "Тёмная")
            SettingsItem("🌍", "Язык", trailing = "Русский")
        }

        SettingsSection("Аккаунт") {
            SettingsItem("👤", "Профиль")
            SettingsItem("🔒", "Безопасность")
            SettingsItem("📊", "Экспорт данных")
        }

        SettingsSection("Поддержка") {
            SettingsItem("❓", "Помощь")
            SettingsItem("📝", "Обратная связь")
            SettingsItem("ℹ\uFE0F", "О приложении", trailing = "v1.0")
        }
    }
}

@Composable
private fun ProfileStat(value: String, label: String) {
    Column(horizontalAlignment = Alignment.CenterHorizontally) {
        Text(
            text = value,
            fontSize = 18.sp,
            fontWeight = FontWeight.ExtraBold,
            color = TextPrimary
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
private fun AchievementChip(icon: String, name: String, unlocked: Boolean) {
    Column(
        modifier = Modifier
            .width(72.dp)
            .clip(RoundedCornerShape(RadiusMd))
            .then(
                if (unlocked) {
                    Modifier
                        .background(
                            Brush.linearGradient(
                                listOf(Color(0x0F_FBBF24), Color.Transparent)
                            )
                        )
                        .border(1.dp, Color(0x4D_FBBF24), RoundedCornerShape(RadiusMd))
                } else {
                    Modifier
                        .background(BgCard)
                        .border(1.dp, Border, RoundedCornerShape(RadiusMd))
                }
            )
            .padding(12.dp, 8.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Text(
            text = icon,
            fontSize = 24.sp,
            modifier = if (!unlocked) Modifier.then(
                Modifier // opacity handled via alpha
            ) else Modifier
        )
        Spacer(Modifier.height(4.dp))
        Text(
            text = name.uppercase(),
            fontSize = 8.sp,
            fontWeight = FontWeight.Bold,
            color = if (unlocked) TextSecondary else TextMuted,
            letterSpacing = 0.3.sp
        )
    }
}

@Composable
private fun SettingsSection(title: String, content: @Composable ColumnScope.() -> Unit) {
    Column(modifier = Modifier.padding(horizontal = 20.dp)) {
        Spacer(Modifier.height(10.dp))
        Text(
            text = title.uppercase(),
            fontSize = 9.sp,
            fontWeight = FontWeight.ExtraBold,
            color = TextMuted,
            letterSpacing = 1.5.sp
        )
        Spacer(Modifier.height(6.dp))
        content()
    }
}

@Composable
private fun SettingsItem(icon: String, text: String, trailing: String? = null) {
    Row(
        modifier = Modifier
            .fillMaxWidth()
            .clickable { }
            .padding(vertical = 11.dp)
            .then(Modifier.drawBehind { }),
        verticalAlignment = Alignment.CenterVertically,
        horizontalArrangement = Arrangement.spacedBy(10.dp)
    ) {
        Text(text = icon, fontSize = 15.sp, modifier = Modifier.width(18.dp))
        Text(
            text = text,
            fontSize = 13.sp,
            fontWeight = FontWeight.Medium,
            color = TextPrimary,
            modifier = Modifier.weight(1f)
        )
        if (trailing != null) {
            Text(
                text = trailing,
                fontSize = 12.sp,
                color = TextSecondary
            )
        }
        Text(
            text = "›",
            fontSize = 12.sp,
            color = TextMuted
        )
    }
    // Divider
    Box(
        modifier = Modifier
            .fillMaxWidth()
            .height(1.dp)
            .background(Border)
    )
}

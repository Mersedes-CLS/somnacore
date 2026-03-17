package com.somnacore.app.ui.screens

import androidx.compose.animation.core.*
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Text
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.SpanStyle
import androidx.compose.ui.text.buildAnnotatedString
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.text.withStyle
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.somnacore.app.ui.components.*
import com.somnacore.app.ui.theme.*

@Composable
fun OnboardingScreen(onStart: () -> Unit = {}) {
    val infiniteTransition = rememberInfiniteTransition(label = "float")
    val offsetY by infiniteTransition.animateFloat(
        initialValue = 0f,
        targetValue = -5f,
        animationSpec = infiniteRepeatable(
            animation = tween(2000, easing = EaseInOut),
            repeatMode = RepeatMode.Reverse
        ),
        label = "floatY"
    )

    Column(
        modifier = Modifier
            .fillMaxSize()
            .background(BgPrimary)
            .padding(horizontal = 30.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        Text(
            text = "\uD83D\uDCAA",
            fontSize = 80.sp,
            modifier = Modifier.offset(y = offsetY.dp)
        )

        Spacer(Modifier.height(24.dp))

        Text(
            text = buildAnnotatedString {
                append("Тренируйся.\n")
                withStyle(SpanStyle(color = AccentLight)) {
                    append("Мы считаем.")
                }
            },
            fontSize = 28.sp,
            fontWeight = FontWeight.Black,
            color = TextPrimary,
            textAlign = TextAlign.Center,
            letterSpacing = (-1).sp,
            lineHeight = 32.sp
        )

        Spacer(Modifier.height(12.dp))

        Text(
            text = "Приложи браслет. Тренируйся.\nВсё запишется само.",
            fontSize = 14.sp,
            color = TextSecondary,
            textAlign = TextAlign.Center,
            lineHeight = 21.sp
        )

        Spacer(Modifier.height(32.dp))

        GradientButton(text = "Начать", onClick = onStart)

        Spacer(Modifier.height(14.dp))

        Row {
            Text(
                text = "Уже есть аккаунт? ",
                fontSize = 13.sp,
                color = TextMuted
            )
            Text(
                text = "Войти",
                fontSize = 13.sp,
                fontWeight = FontWeight.SemiBold,
                color = AccentLight,
                modifier = Modifier.clickable { }
            )
        }

        Spacer(Modifier.height(28.dp))

        // Dots indicator
        Row(horizontalArrangement = Arrangement.spacedBy(6.dp)) {
            Box(
                modifier = Modifier
                    .width(20.dp)
                    .height(6.dp)
                    .clip(RoundedCornerShape(3.dp))
                    .background(Accent)
            )
            Box(
                modifier = Modifier
                    .size(6.dp)
                    .clip(CircleShape)
                    .background(TextMuted)
            )
            Box(
                modifier = Modifier
                    .size(6.dp)
                    .clip(CircleShape)
                    .background(TextMuted)
            )
        }
    }
}

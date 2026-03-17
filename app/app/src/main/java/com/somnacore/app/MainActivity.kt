package com.somnacore.app

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.animation.*
import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.somnacore.app.ui.screens.*
import com.somnacore.app.ui.theme.*

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            SomnaCoreTheme {
                SomnaCoreApp()
            }
        }
    }
}

@Composable
fun SomnaCoreApp() {
    var showOnboarding by remember { mutableStateOf(true) }
    var selectedTab by remember { mutableIntStateOf(0) }

    if (showOnboarding) {
        OnboardingScreen(onStart = { showOnboarding = false })
    } else {
        Box(modifier = Modifier.fillMaxSize().background(BgPrimary)) {
            // Screen content
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .padding(top = 44.dp) // status bar space
            ) {
                when (selectedTab) {
                    0 -> HomeScreen()
                    1 -> LiveScreen()
                    2 -> HistoryScreen()
                    3 -> StatsScreen()
                    4 -> ProfileScreen()
                }
            }

            // Bottom nav bar
            BottomNavBar(
                selectedTab = selectedTab,
                onTabSelected = { selectedTab = it },
                modifier = Modifier.align(Alignment.BottomCenter)
            )
        }
    }
}

@Composable
private fun BottomNavBar(
    selectedTab: Int,
    onTabSelected: (Int) -> Unit,
    modifier: Modifier = Modifier
) {
    val tabs = listOf(
        "🏠" to "Главная",
        "⚡" to "Live",
        "📅" to "История",
        "📊" to "Стат.",
        "👤" to "Профиль"
    )

    Row(
        modifier = modifier
            .fillMaxWidth()
            .background(
                Brush.verticalGradient(
                    listOf(Color.Transparent, BgPrimary.copy(alpha = 0.95f), BgPrimary)
                )
            )
            .padding(top = 12.dp, bottom = 32.dp, start = 4.dp, end = 4.dp),
        horizontalArrangement = Arrangement.SpaceAround,
        verticalAlignment = Alignment.CenterVertically
    ) {
        tabs.forEachIndexed { index, (icon, label) ->
            val isActive = index == selectedTab

            Column(
                modifier = Modifier
                    .clip(RoundedCornerShape(12.dp))
                    .then(
                        if (isActive) {
                            Modifier.background(Color(0x14_7C3AED))
                        } else Modifier
                    )
                    .clickable { onTabSelected(index) }
                    .padding(horizontal = 8.dp, vertical = 4.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Text(
                    text = icon,
                    fontSize = 17.sp,
                    modifier = Modifier.size(26.dp),
                )
                Text(
                    text = label,
                    fontSize = 9.sp,
                    fontWeight = FontWeight.SemiBold,
                    color = if (isActive) AccentLight else TextMuted
                )
                if (isActive) {
                    Spacer(Modifier.height(2.dp))
                    Box(
                        modifier = Modifier
                            .size(4.dp)
                            .clip(CircleShape)
                            .background(Accent)
                    )
                }
            }
        }
    }
}

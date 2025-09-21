#pragma once

// Configuration file - modify these values and recompile
// Set ENABLE_CONFIG_MODE to true to use these hardcoded values
// Set to false to use normal web setup mode

#define ENABLE_CONFIG_MODE false

#if ENABLE_CONFIG_MODE

// WiFi Configuration
#define CONFIG_WIFI_SSID "Your_WiFi_Name"
#define CONFIG_WIFI_PASSWORD "your_wifi_password"

// API Configuration
#define CONFIG_API_KEY "your_coingecko_api_key"

// Currency Configuration (usd, eur, gbp, etc.)
#define CONFIG_CURRENCY "usd"

// Cryptocurrency Configuration (up to 5)
#define CONFIG_CRYPTO_1 "bitcoin"
#define CONFIG_CRYPTO_2 "ethereum"
#define CONFIG_CRYPTO_3 "cardano"
#define CONFIG_CRYPTO_4 ""  // Leave empty if not used
#define CONFIG_CRYPTO_5 ""  // Leave empty if not used

#endif
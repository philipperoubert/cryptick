#include <Arduino.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>


HTTPClient http;
Preferences preferences;
int minutes = 60000000;

// Fetches price change data for the given cryptocurrency
DynamicJsonDocument getChange(const String &coin_name)
{
  Serial.println("Getting price changes");

  // Add a short delay
  delay(2000);

  // Get the coingecko api key 
  preferences.begin("CryptoCreds", true);
  String api_key = preferences.getString("api_key", "");
  String currency = preferences.getString("currency", "usd");
  preferences.end();

  // Construct the API URL
  String apiUrl = "https://api.coingecko.com/api/v3/coins/markets?vs_currency=" + currency + "&ids=" + coin_name + "&order=market_cap_desc&per_page=100&page=1&sparkline=false&price_change_percentage=1h%2C24h%2C7d&x_cg_demo_api_key=" + api_key;

  // Set a timeout for the HTTP request
  http.setTimeout(10000);
  http.begin(apiUrl.c_str());

  int code = http.GET();

  // Check if the HTTP request was successful
  if (code != HTTP_CODE_OK)
  {
    Serial.println("Error connecting to API");
    Serial.println(code);
    // Sleep for 10 minutes and retry
    esp_sleep_enable_timer_wakeup(10 * minutes);
    esp_deep_sleep_start();
    // Return an empty DynamicJsonDocument on error
    return DynamicJsonDocument(0);
  }

  Serial.println("Successfully downloaded price changes");

  // Parse the JSON response
  DynamicJsonDocument doc(4096);

  DeserializationError error = deserializeJson(doc, http.getString());

  // Check for deserialization errors
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    // Return an empty DynamicJsonDocument on error
    return DynamicJsonDocument(0);
  }

  http.end();

  return doc;
}

// Fetches historical data for the given cryptocurrency
JsonArray getHistory(const String &coin_name) {
  Serial.println("Getting history");

  // Add a short delay
  delay(12500);

  // Get the coingecko api key 
  preferences.begin("CryptoCreds", true);
  String api_key = preferences.getString("api_key", "");
  String currency = preferences.getString("currency", "usd");
  preferences.end();

  // Construct the API URL
  String apiUrl = "https://api.coingecko.com/api/v3/coins/" + coin_name + "/market_chart?vs_currency=" + currency + "&days=1.01&x_cg_demo_api_key=" + api_key;

  // Set a timeout for the HTTP request
  http.setTimeout(10000);
  http.begin(apiUrl.c_str());

  Serial.print("code ");
  int code = http.GET();

  // Check if the HTTP request was successful
  if (code != HTTP_CODE_OK) {
    Serial.println("Error connecting to API");
    Serial.println(code);
    // Sleep for 10 minutes and retry
    esp_sleep_enable_timer_wakeup(10 * minutes);
    esp_deep_sleep_start();
  }

  Serial.println("Successfully downloaded history");

  // Parse the JSON response
  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, http.getString());

  // Check for deserialization errors
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    // Return an empty JsonArray on error
    return JsonArray();
  }

  // Extract the 'prices' array from the JSON response
  JsonArray prices = doc["prices"];

  http.end();

  return prices;
}

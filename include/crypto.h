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
    http.end();
    // Return an empty but valid DynamicJsonDocument
    DynamicJsonDocument empty_doc(256);
    
    // If requested, go to sleep before returning (keeping original behavior)
    preferences.begin("CryptoCreds", true);
    bool sleep_on_error = preferences.getBool("sleep_on_error", true); // default to true for backward compatibility
    preferences.end();
    
    if (sleep_on_error) {
      Serial.println("Sleep on error enabled. Going to sleep for 10 minutes.");
      esp_sleep_enable_timer_wakeup(10 * minutes);
      esp_deep_sleep_start();
    } else {
      Serial.println("Sleep on error disabled. Continuing with partial data.");
    }
    
    return empty_doc;
  }

  Serial.println("Successfully downloaded price changes");

  // First, get the content length to estimate required memory
  size_t capacity = http.getSize();
  if (capacity == 0) {
    capacity = 8192; // Default to 8KB if size unknown
  } else {
    capacity = capacity * 2; // Double it to be safe with JSON parsing overhead
  }
  
  // Cap the capacity to avoid memory issues
  if (capacity > 16384) {
    capacity = 16384; // Cap at 16KB max for this endpoint
  }
  
  Serial.printf("Change data JSON capacity: %d bytes\n", capacity);

  // Create a dynamic document with the estimated capacity
  DynamicJsonDocument doc(capacity);
  
  // Get the response and end the connection before parsing
  String payload = http.getString();
  http.end();
  
  Serial.printf("Change data payload size: %d bytes\n", payload.length());

  // Parse the JSON response
  DeserializationError error = deserializeJson(doc, payload);

  // Check for deserialization errors
  if (error)
  {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    // Return an empty DynamicJsonDocument on error
    return DynamicJsonDocument(256);
  }

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
  String apiUrl = "https://api.coingecko.com/api/v3/coins/" + coin_name + "/market_chart?vs_currency=" + currency + "&days=1&x_cg_demo_api_key=" + api_key;

  // Set a timeout for the HTTP request
  http.setTimeout(10000);
  http.begin(apiUrl.c_str());

  Serial.print("code ");
  int code = http.GET();

  // Check if the HTTP request was successful
  if (code != HTTP_CODE_OK) {
    Serial.println("Error connecting to API");
    Serial.println(code);
    http.end();
    
    // If requested, go to sleep before returning (keeping original behavior)
    preferences.begin("CryptoCreds", true);
    bool sleep_on_error = preferences.getBool("sleep_on_error", true); // default to true for backward compatibility
    preferences.end();
    
    if (sleep_on_error) {
      Serial.println("Sleep on error enabled. Going to sleep for 10 minutes.");
      esp_sleep_enable_timer_wakeup(10 * minutes);
      esp_deep_sleep_start();
    } else {
      Serial.println("Sleep on error disabled. Continuing with partial data.");
    }
    
    // Return an empty but valid JsonArray
    DynamicJsonDocument empty_doc(256);
    JsonArray empty_array = empty_doc.to<JsonArray>();
    return empty_array;
  }

  Serial.println("Successfully downloaded history");

  // First, get the content length to estimate required memory
  size_t capacity = http.getSize();
  if (capacity == 0) {
    capacity = 32768; // Default to 32KB if size unknown
  } else {
    capacity = capacity * 2; // Double it to be safe with JSON parsing overhead
  }
  
  // Cap the capacity to avoid memory issues
  if (capacity > 65536) {
    capacity = 65536; // Cap at 64KB max
  }
  
  Serial.printf("JSON capacity: %d bytes\n", capacity);
  
  // Create a dynamic document with the estimated capacity
  DynamicJsonDocument doc(capacity);
  
  // Get the response and end the connection before parsing
  String payload = http.getString();
  http.end();
  
  Serial.printf("Payload size: %d bytes\n", payload.length());

  // Try to parse the JSON
  DeserializationError error = deserializeJson(doc, payload);

  // Check for deserialization errors
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    
    if (error == DeserializationError::NoMemory) {
      Serial.println("Not enough memory to parse JSON. Trying to process the data directly.");
      
      // Simple alternative approach: Extract a subset of prices instead of the full array
      // This is a fallback method to extract just a few price points for drawing
      DynamicJsonDocument smallDoc(4096);
      JsonArray priceArray = smallDoc.to<JsonArray>();
      
      // Extract a few price points from the payload using string operations
      int priceStartPos = payload.indexOf("\"prices\":[[");
      if (priceStartPos > 0) {
        priceStartPos += 10; // Skip over "prices":[[
        int priceEndPos = payload.indexOf("]]", priceStartPos);
        
        if (priceEndPos > priceStartPos) {
          String pricesStr = payload.substring(priceStartPos, priceEndPos);
          
          // Extract just a subset of prices (e.g., every 24 data points for hourly data)
          int count = 0;
          int lastPos = 0;
          int step = 24; // Sample every ~hour if the data is minute-by-minute
          
          while (lastPos < pricesStr.length()) {
            int bracketPos = pricesStr.indexOf('[', lastPos);
            if (bracketPos < 0) break;
            
            int endBracketPos = pricesStr.indexOf(']', bracketPos);
            if (endBracketPos < 0) break;
            
            count++;
            if (count % step == 0 || count == 1 || lastPos == 0) {
              String pricePair = pricesStr.substring(bracketPos + 1, endBracketPos);
              
              int commaPos = pricePair.indexOf(',');
              if (commaPos > 0) {
                long timestamp = pricePair.substring(0, commaPos).toInt();
                float price = pricePair.substring(commaPos + 1).toFloat();
                
                // Add to our simplified array
                JsonArray point = priceArray.createNestedArray();
                point.add(timestamp);
                point.add(price);
              }
            }
            
            lastPos = endBracketPos + 1;
          }
          
          Serial.printf("Extracted %d price points from raw data\n", priceArray.size());
          return priceArray;
        }
      }
    }
    
    // Return an empty but valid JsonArray on error
    DynamicJsonDocument empty_doc(256);
    JsonArray empty_array = empty_doc.to<JsonArray>();
    return empty_array;
  }

  // Check if "prices" key exists in the response
  if (!doc.containsKey("prices") || doc["prices"].isNull()) {
    Serial.println("Response does not contain 'prices' data");
    // Return an empty but valid JsonArray
    DynamicJsonDocument empty_doc(256);
    JsonArray empty_array = empty_doc.to<JsonArray>();
    return empty_array;
  }

  // Extract the 'prices' array from the JSON response
  JsonArray prices = doc["prices"];
  Serial.printf("Extracted %d price points\n", prices.size());

  return prices;
}

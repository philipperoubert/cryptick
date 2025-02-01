#include <Arduino.h>
#include <AsyncTCP.h>
#include <DNSServer.h>
#include <ESPAsyncWebServer.h>
#include <Preferences.h>
#include <Ticker.h>
#include <WiFi.h>
#include "roboto.h"
#include "battery_0.h"
#include "battery_1.h"
#include "battery_2.h"
#include "battery_3.h"
#include "battery_4.h"
#include "Esp.h"

const char *ssid = "Cryptick";
const char *password = "tothemoon";

// Remove the duplicate declaration of 'preferences'
AsyncWebServer server(80);


String getTime()
{
  struct tm timeinfo;
  if (getLocalTime(&timeinfo))
  {
    char time[6]; // HH:MM format requires 5 characters + 1 for the null terminator
    sprintf(time, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);

    Serial.println(time);
    return String(time);
  }
  else
  {
    Serial.println("Failed to get local time");
    return "00:00";
  }
}

String getWiFiNetworks()
{
  String networks = "<ul>";
  int numNetworks = WiFi.scanNetworks();
  for (int i = 0; i < numNetworks; i++)
  {
    networks += "<li>";
    networks += WiFi.SSID(i);
    networks += "</li>";
  }
  networks += "</ul>";
  return networks;
}

String setupAP()
{
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access point IP: ");
  Serial.println(IP);
  return IP.toString();
}

bool tryConnectSavedNetwork()
{
  preferences.begin("wifi", false);
  String savedSSID = preferences.getString("ssid", "");
  String savedPassword = preferences.getString("password", "");
  preferences.end();

  if (savedSSID == "")
  {
    return false;
  }

  WiFi.begin(savedSSID.c_str(), savedPassword.c_str());

  unsigned long startTime = millis();
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if (millis() - startTime > 10000)
    {
      Serial.println("Failed to connect to saved network");
      return false;
    }
  }
  return true;
}

void saveNetworkCredentials(const String &inputSSID,
                            const String &inputPassword)
{
  preferences.begin("wifi", false);
  preferences.putString("ssid", inputSSID);
  preferences.putString("password", inputPassword);
  preferences.end();

  // Debug prints
  preferences.begin("wifi", true);
  String savedSSID = preferences.getString("ssid", "");
  String savedPassword = preferences.getString("password", "");
  preferences.end();
  Serial.print("Saved SSID: ");
  Serial.println(savedSSID);
  Serial.print("Saved Password: ");
  Serial.println(savedPassword);
}

Ticker wifiConnectTicker;
bool connectToNewNetwork = false;

void connectToWiFi()
{
  if (connectToNewNetwork)
  {
    preferences.begin("wifi", true);
    String inputSSID = preferences.getString("ssid", "");
    String inputPassword = preferences.getString("password", "");
    preferences.end();

    // print inputssid
    Serial.print("Input SSID: ");
    Serial.println(inputSSID);

    WiFi.softAPdisconnect(true);
    WiFi.begin(inputSSID.c_str(), inputPassword.c_str());

    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
      if (millis() - startTime > 30000)
      {
        Serial.println("Connection failed. Restarting...");
        ESP.restart();
      }
    }

    Serial.println(
        "\nConnected to new network. Redirecting to cryptocurrency form...");
    connectToNewNetwork = false;
  }
}

void clearSavedWiFiCredentials()
{
  preferences.begin("wifi", false);
  preferences.remove("ssid");
  preferences.remove("password");
  preferences.end();
  Serial.println("Saved WiFi credentials cleared.");
}

void displayInstructions(FontProperties *props, String &ip, uint8_t *framebuffer)
{
  int cursor_x = 30;
  int cursor_y = 65;
  write_mode((GFXfont *)&roboto, "WiFi Set-Up and Cryptocurrency Selection",
             &cursor_x, &cursor_y, framebuffer, BLACK_ON_WHITE, props);
  cursor_x = 30;
  cursor_y = 120;
  write_mode((GFXfont *)&roboto, "1. Connect to Device:", &cursor_x, &cursor_y,
             framebuffer, BLACK_ON_WHITE, props);
  cursor_x = 30;
  cursor_y = 150;
  write_mode((GFXfont *)&roboto, "        SSID: Cryptick", &cursor_x, &cursor_y,
             framebuffer, BLACK_ON_WHITE, props);
  cursor_x = 30;
  cursor_y = 180;
  write_mode((GFXfont *)&roboto, "        Password: tothemoon", &cursor_x,
             &cursor_y, framebuffer, BLACK_ON_WHITE, props);
  cursor_x = 30;
  cursor_y = 210;
  write_mode((GFXfont *)&roboto, "2. Open a browser and go to:", &cursor_x,
             &cursor_y, framebuffer, BLACK_ON_WHITE, props);
  cursor_x = 30;
  cursor_y = 240;
  ip = "        http://" + ip + " (Make sure to type in the 'http://' at the beginning of the url.";
  write_mode((GFXfont *)&roboto, &ip[0], &cursor_x, &cursor_y, framebuffer,
             BLACK_ON_WHITE, props);
  cursor_x = 30;
  cursor_y = 270;
  ip = "        Most browsers will be default try 'https://' if you don't add it.)";
  write_mode((GFXfont *)&roboto, &ip[0], &cursor_x, &cursor_y, framebuffer,
             BLACK_ON_WHITE, props);
  cursor_x = 30;
  cursor_y = 300;
  write_mode((GFXfont *)&roboto, "3. Connect to your WiFi", &cursor_x,
             &cursor_y, framebuffer, BLACK_ON_WHITE, props);
  cursor_x = 30;
  cursor_y = 330;
  write_mode((GFXfont *)&roboto, "4. Select your cryptocurrencies", &cursor_x,
             &cursor_y, framebuffer, BLACK_ON_WHITE, props);
  cursor_x = 30;
  cursor_y = 360;
  write_mode((GFXfont *)&roboto, "5. Click Save", &cursor_x, &cursor_y,
             framebuffer, BLACK_ON_WHITE, props);
}

void configureWebServerRoutes(AsyncWebServer &server)
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      String html = "<!DOCTYPE html><html><head>";
      html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
      html += "<style>body {font-family: Arial, sans-serif; padding: 15px;} input, select {width: 100%; margin: 5px 0;} input[type=submit] {background-color: #4CAF50; color: white; cursor: pointer;}</style>";
      html += "<script>function submitForm(event) {";
      html += "event.preventDefault();";
      html += "var formData = new FormData(event.target);";
      html += "var xhr = new XMLHttpRequest();";
      html += "xhr.open('POST', '/connect', true);";
      html += "xhr.onload = function() {";
      html += "if (xhr.status >= 200 && xhr.status < 400) {";
      html += "var json = JSON.parse(xhr.responseText);";
      html += "if (json.result === 'success') {";
      html += "document.getElementById('status').innerText = 'WiFi credentials saved. Redirecting to cryptocurrency form...';";
      html += "setTimeout(function() {window.location.href = '/cryptos';}, 3000);";
      html += "} else {";
      html += "document.getElementById('status').innerText = 'Connection failed. Please try again.';";
      html += "}} else {";
      html += "document.getElementById('status').innerText = 'Error: ' + xhr.statusText;";
      html += "}};";
      html += "xhr.onerror = function() {";
      html += "document.getElementById('status').innerText = 'Error: Request failed.';";
      html += "};";
      html += "xhr.send(formData);";
      html += "}</script>";
      html += "</head><body><h1>ESP32 WiFi Configuration</h1>";
      html += "<form onsubmit=\"submitForm(event)\">";
      html += "<label for=\"network\">Network:</label><br>" + getWiFiNetworks();
      html += "<label for=\"ssid\">SSID:</label><br><input type=\"text\" name=\"ssid\" id=\"ssid\"><br>";
      html += "<label for=\"password\">Password:</label><br><input type=\"password\" name=\"password\" id=\"password\"><br>";
      html += "<input type=\"submit\" value=\"Connect\">";
      html += "</form><p id=\"status\"></p></body></html>";

      request->send(200, "text/html", html); });

  server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request)
            {
      if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
        String inputSSID = request->getParam("ssid", true)->value();
        String inputPassword = request->getParam("password", true)->value();

        saveNetworkCredentials(inputSSID, inputPassword);

        request->send(200, "application/json", "{\"result\": \"success\"}");

        // delay(3000); // Give the browser enough time to receive the response
        // Serial.println("Credentials saved. Restarting...");
        // ESP.restart();
      } else {
        request->send(400, "text/html", "Invalid request. Both SSID and password are required.");
      } });

  server.on("/cryptos", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      String html = "<!DOCTYPE html><html><head>";
      html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
      html += "<style>body {font-family: Arial, sans-serif; padding: 15px;} input, select {width: 100%; margin: 5px 0;} input[type=submit] {background-color: #4CAF50; color: white; cursor: pointer;}</style>";
      html += "</head><body><h1>Enter Cryptocurrencies</h1>";
      html += "<form action=\"/save-cryptos\" method=\"post\">";
      for (int i = 1; i <= 5; i++) {
        html += "<label for=\"crypto" + String(i) + "\">Crypto " + String(i) + ":</label><br>";
        if (i == 1) {
          html += "<input type=\"text\" name=\"crypto" + String(i) + "\" id=\"crypto" + String(i) + "\" value=\"bitcoin\"><br>";
          continue;
        } else if (i == 2) {
          html += "<input type=\"text\" name=\"crypto" + String(i) + "\" id=\"crypto" + String(i) + "\" value=\"ethereum\"><br>";
          continue;
        }
        html += "<input type=\"text\" name=\"crypto" + String(i) + "\" id=\"crypto" + String(i) + "\"><br>";
      }
      html += "<h1>Enter Base Currency</h1>";
      html += "<label for=\"base\">Base Currency:</label><br>";
      html += "<input type=\"text\" name=\"base\" id=\"base\" value=\"USD\"><br>";
      html += "<h1>Enter Coingecko API Key</h1>";
      html += "<label for=\"apiKey\">Coingecko API Key:</label><br>";
      html += "<input type=\"text\" name=\"apiKey\" id=\"apiKey\"><br>";
      html += "<input type=\"submit\" value=\"Save\">";
      html += "</form></body></html>";
      request->send(200, "text/html", html); });

  server.on("/save-cryptos", HTTP_POST, [](AsyncWebServerRequest *request)
            {
      preferences.begin("CryptoCreds", false);
      for (int i = 1; i <= 5; i++) {
        String paramName = "crypto" + String(i);
        if (request->hasParam(paramName.c_str(), true)) {
          String cryptoName = request->getParam(paramName.c_str(), true)->value();
          std::transform(cryptoName.begin(), cryptoName.end(), cryptoName.begin(), ::tolower);
          Serial.print(paramName.c_str());
          Serial.println(cryptoName);
          preferences.putString(paramName.c_str(), cryptoName);
        } else {
          preferences.putString(paramName.c_str(), "");
        }
      }
      preferences.putString("currency", request->getParam("base", true)->value());
      preferences.putString("api_key", request->getParam("apiKey", true)->value());
      preferences.end();
      request->send(200, "text/html", "Cryptocurrencies and API key saved. Rebooting...");
      delay(2000);
      ESP.restart(); });
}


void battery(float voltage, uint8_t *framebuffer) {
  Rect_t area = {
      .x = 900, .y = 10, .width = battery_0_width, .height = battery_0_height};

  if (voltage > 4.43) {
    epd_copy_to_framebuffer(area, (uint8_t *)battery_4_data, framebuffer);
  } else if (voltage < 4.43 && voltage >= 4.35) {
    epd_copy_to_framebuffer(area, (uint8_t *)battery_3_data, framebuffer);
  } else if (voltage < 4.35 && voltage >= 4.06) {
    epd_copy_to_framebuffer(area, (uint8_t *)battery_2_data, framebuffer);
  } else if (voltage < 4.06 && voltage >= 3.91) {
    epd_copy_to_framebuffer(area, (uint8_t *)battery_1_data, framebuffer);
  } else if (voltage < 3.1) {
    epd_copy_to_framebuffer(area, (uint8_t *)battery_0_data, framebuffer);
  }
}

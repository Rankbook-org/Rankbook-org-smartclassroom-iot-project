#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHTesp.h>

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* TEST_URL = "https://httpbin.org/post";
// Backup if httpbin.org is slow or returns 5xx: "https://postman-echo.com/post"
// Or open https://webhook.site, copy your unique URL here, and watch requests arrive live.

const char* DEVICE_ID = "pair-00";  // change to your pair's ID, e.g. "pair-07"

const int DHT_PIN = 15;
const int LED_PIN = 18;
const float WARNING_TEMPERATURE_C = 30.0;

DHTesp dht;

void connectWiFi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
  Serial.print("Connecting to WiFi");
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.setup(DHT_PIN, DHTesp::DHT22);
  connectWiFi();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }

  TempAndHumidity reading = dht.getTempAndHumidity();
  if (isnan(reading.temperature) || isnan(reading.humidity)) {
    Serial.println("Sensor read failed.");
    delay(3000);
    return;
  }

  const bool warning = reading.temperature >= WARNING_TEMPERATURE_C;
  digitalWrite(LED_PIN, warning ? HIGH : LOW);

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    // DEMO ONLY: disables TLS certificate validation. Do not use in production.
    client.setInsecure();

    HTTPClient http;
    http.begin(client, TEST_URL);
    http.addHeader("Content-Type", "application/json");

    JsonDocument doc;
    doc["device_id"] = DEVICE_ID;
    doc["temperature"] = reading.temperature;
    doc["humidity"] = reading.humidity;
    doc["status"] = warning ? "WARNING" : "NORMAL";

    String payload;
    serializeJson(doc, payload);

    Serial.println("POST payload:");
    Serial.println(payload);

    int httpCode = http.POST(payload);
    Serial.printf("HTTP status: %d\n", httpCode);
    if (httpCode > 0) {
      Serial.println("Echo response (first 500 chars):");
      Serial.println(http.getString().substring(0, 500));
    } else {
      Serial.printf("HTTP error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }

  delay(10000);
}

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHTesp.h>

// Replace these placeholders with the values your instructor gives you.
// Never put a Supabase service-role key in this sketch.
const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";
const char* SUPABASE_PROJECT_URL = "https://YOUR_SUPABASE_PROJECT_REF.supabase.co";
const char* WORKSHOP_TOKEN = "YOUR_WORKSHOP_TOKEN";

// Make this unique for your pair so you can find YOUR device on the class dashboard.
// Allowed: letters, numbers, underscore, hyphen (max 64). Example: "pair-07"
const char* DEVICE_ID = "pair-00";

const int DHT_PIN = 15;
const int LED_PIN = 18;
const float WARNING_TEMPERATURE_C = 30.0;

DHTesp dht;

String functionUrl() {
  return String(SUPABASE_PROJECT_URL) + "/functions/v1/ingest-reading";
}

void connectWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.println("Connecting to Wokwi WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connected");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi connection failed.");
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.setup(DHT_PIN, DHTesp::DHT22);
  Serial.println("Smart Classroom Monitor - Stage 4: Supabase");
  connectWiFi();
}

void loop() {
  connectWiFi();

  TempAndHumidity reading = dht.getTempAndHumidity();
  if (isnan(reading.temperature) || isnan(reading.humidity)) {
    Serial.println("Sensor read failed; not sending data.");
    delay(3000);
    return;
  }

  const bool warning = reading.temperature >= WARNING_TEMPERATURE_C;
  digitalWrite(LED_PIN, warning ? HIGH : LOW);

  JsonDocument doc;
  doc["device_id"] = DEVICE_ID;
  doc["temperature"] = reading.temperature;
  doc["humidity"] = reading.humidity;
  doc["status"] = warning ? "WARNING" : "NORMAL";

  String payload;
  serializeJson(doc, payload);

  Serial.println("Reading:");
  Serial.println(payload);

  if (WiFi.status() == WL_CONNECTED) {
    WiFiClientSecure client;
    // DEMO ONLY: skips TLS certificate verification. Not production-safe.
    client.setInsecure();

    HTTPClient http;
    http.begin(client, functionUrl());
    http.addHeader("Content-Type", "application/json");
    http.addHeader("x-workshop-token", WORKSHOP_TOKEN);

    int httpCode = http.POST(payload);
    Serial.printf("Supabase function HTTP status: %d\n", httpCode);
    if (httpCode > 0) {
      Serial.println(http.getString());
    } else {
      Serial.printf("Request error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  }

  delay(15000);
}

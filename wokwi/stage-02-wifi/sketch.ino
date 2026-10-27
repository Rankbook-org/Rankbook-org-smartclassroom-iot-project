#include <WiFi.h>

const char* WIFI_SSID = "Wokwi-GUEST";
const char* WIFI_PASSWORD = "";

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("Smart Classroom Monitor - Stage 2");
  Serial.println("Connecting to WiFi...");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi connection failed. Check Wokwi settings.");
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected; reconnecting...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD, 6);
  } else {
    Serial.println("WiFi is connected.");
  }
  delay(5000);
}

#include <DHTesp.h>

const int DHT_PIN = 15;
const int LED_PIN = 18;
const float WARNING_TEMPERATURE_C = 30.0;

DHTesp dht;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.setup(DHT_PIN, DHTesp::DHT22);
  Serial.println("Smart Classroom Monitor - Stage 1");
}

void loop() {
  TempAndHumidity reading = dht.getTempAndHumidity();

  if (isnan(reading.temperature) || isnan(reading.humidity)) {
    Serial.println("Sensor read failed. Check DHT22 wiring/library.");
    digitalWrite(LED_PIN, LOW);
    delay(2000);
    return;
  }

  const bool warning = reading.temperature >= WARNING_TEMPERATURE_C;
  digitalWrite(LED_PIN, warning ? HIGH : LOW);

  Serial.printf("Temperature: %.2f C\n", reading.temperature);
  Serial.printf("Humidity: %.2f %%\n", reading.humidity);
  Serial.printf("Status: %s\n\n", warning ? "WARNING" : "NORMAL");

  delay(3000);
}

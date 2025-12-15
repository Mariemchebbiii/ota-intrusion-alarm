#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

#define FW_VERSION "2.00"

// ============================================
// ⚠️ CONFIGURATION - UPDATE THESE VALUES!
// ============================================
const char* WIFI_SSID = "YOUR_WIFI_SSID";      // Your WiFi network name
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";  // Your WiFi password

#define PIR_PIN D5
#define LED_PIN D6
#define BUZZER_PIN D7
#define BTN_PIN D3

bool armed = true;
bool alarmOn = false;

void setup() {
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(300);
  }
  digitalWrite(LED_PIN, LOW);

  ArduinoOTA.setHostname("ESP-ALARM");
  ArduinoOTA.setPassword("ota123");
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();

  if (digitalRead(BTN_PIN) == LOW) {
    armed = !armed;
    delay(500);
  }

  if (armed && digitalRead(PIR_PIN) == HIGH) {
    alarmOn = true;
  }

  if (alarmOn) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(150);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    delay(150);
  }
}

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>

#define FW_VERSION "2.01"

// WiFi Configuration
const char* WIFI_SSID = "TOPNET_2FB0";
const char* WIFI_PASS = "3m3smnb68l";

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

  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=== ESP ALARM BASIC ===");
  Serial.printf("Version: %s\n", FW_VERSION);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(300);
  }
  digitalWrite(LED_PIN, LOW);

  Serial.println("WiFi connecte");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.setHostname("ESP-ALARM");
  ArduinoOTA.setPassword("ota123");
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();

  if (digitalRead(BTN_PIN) == LOW) {
    delay(50);
    if (digitalRead(BTN_PIN) == LOW) {
      armed = !armed;
      alarmOn = false;
      Serial.printf("System %s\n", armed ? "ARMED" : "DISARMED");
      
      for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
      }
      
      while (digitalRead(BTN_PIN) == LOW) delay(10);
    }
  }

  if (armed && digitalRead(PIR_PIN) == HIGH) {
    if (!alarmOn) {
      Serial.println("⚠️ INTRUSION DETECTED!");
    }
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

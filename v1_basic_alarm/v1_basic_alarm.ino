/*
 * ESP32 Intrusion Alarm with OTA Updates
 * Version: 3.00 - ESP32 EDITION (POWERFUL & FAST!)
 *
 * ESP32 ADVANTAGES:
 * - MUCH faster HTTPS downloads (powerful CPU)
 * - More memory (no SSL issues!)
 * - Dual-core processor
 * - Better WiFi stability
 */

#include <WiFi.h>
#include <ArduinoOTA.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>

// =====================================================
// FIRMWARE VERSION - CHANGE THIS TO TRIGGER OTA
// =====================================================
#define FW_VERSION "1.00"

// =====================================================
// WIFI CONFIGURATION
// =====================================================
const char* WIFI_SSID = "TOPNET_2FB0";
const char* WIFI_PASS = "3m3smnb68l";

// =====================================================
// OTA URLs - Using HTTP for MAXIMUM SPEED!
// =====================================================
const char* VERSION_URL = "http://raw.githubusercontent.com/Mariemchebbiii/ota-intrusion-alarm/main/docs/version.txt";
const char* FIRMWARE_URL = "http://github.com/Mariemchebbiii/ota-intrusion-alarm/releases/download/v3.00/firmware.bin";

// =====================================================
// PIN CONFIGURATION (ESP32 pins)
// =====================================================
const int PIR_PIN = 18;      // GPIO18 for PIR sensor
const int BUZZER_PIN = 19;   // GPIO19 for buzzer
const int LED_PIN = 2;       // Built-in LED on GPIO2

// =====================================================
// TIMING CONFIGURATION
// =====================================================
const unsigned long OTA_CHECK_INTERVAL = 30000;  // Check every 30 seconds
unsigned long lastOtaCheck = 0;
bool motionDetected = false;
bool alarmEnabled = true;

// =====================================================
// SETUP
// =====================================================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n");
  Serial.println("========================================");
  Serial.println("   ESP32 ALARM v" FW_VERSION);
  Serial.println("   POWERFUL DUAL-CORE EDITION");
  Serial.println("========================================");
  
  // Initialize pins
  pinMode(PIR_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  
  digitalWrite(BUZZER_PIN, LOW);
  digitalWrite(LED_PIN, HIGH);
  
  // Connect to WiFi
  connectWiFi();
  
  // Setup ArduinoOTA for local updates
  setupArduinoOTA();
  
  // Initial OTA check
  Serial.println("\n[STARTUP] Initial OTA check...");
  checkForOTAUpdate();
  
  Serial.println("\n[READY] System armed and monitoring!");
  Serial.println("========================================\n");
}

// =====================================================
// MAIN LOOP
// =====================================================
void loop() {
  ArduinoOTA.handle();
  
  // Check for motion
  if (digitalRead(PIR_PIN) == HIGH && alarmEnabled) {
    if (!motionDetected) {
      motionDetected = true;
      Serial.println("[ALERT] Motion detected!");
      triggerAlarm();
    }
  } else {
    motionDetected = false;
    digitalWrite(BUZZER_PIN, LOW);
  }
  
  // Periodic OTA check
  if (millis() - lastOtaCheck > OTA_CHECK_INTERVAL) {
    lastOtaCheck = millis();
    checkForOTAUpdate();
  }
  
  delay(100);
}

// =====================================================
// WIFI CONNECTION
// =====================================================
void connectWiFi() {
  Serial.print("[WIFI] Connecting to ");
  Serial.println(WIFI_SSID);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(" OK!");
    Serial.print("[WIFI] IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println(" FAILED!");
    ESP.restart();
  }
}

// =====================================================
// ARDUINO OTA SETUP
// =====================================================
void setupArduinoOTA() {
  ArduinoOTA.setHostname("esp8266-alarm");
  ArduinoOTA.setPassword("otapass");
  
  ArduinoOTA.onStart([]() {
    Serial.println("[ArduinoOTA] Starting update...");
    alarmEnabled = false;
    digitalWrite(BUZZER_PIN, LOW);
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\n[ArduinoOTA] Update complete!");
  });
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[ArduinoOTA] Error[%u]\n", error);
    alarmEnabled = true;
  });
  
  ArduinoOTA.begin();
  Serial.println("[ArduinoOTA] Ready (password: otapass)");
}

// =====================================================
// OTA UPDATE CHECK - BULLETPROOF VERSION
// =====================================================
void checkForOTAUpdate() {
  Serial.println("\n-------- OTA CHECK --------");
  Serial.print("[OTA] Current version: ");
  Serial.println(FW_VERSION);
  Serial.print("[OTA] Free heap: ");
  Serial.println(ESP.getFreeHeap());
  
  // Check WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[OTA] WiFi not connected, reconnecting...");
    connectWiFi();
  }
  
  // Use HTTP for fast version checking
  WiFiClient client;
  
  // Step 1: Get remote version from GitHub Raw (Avoids API rate limits!)
  Serial.println("[OTA] Checking remote version via GitHub Raw...");
  
  HTTPClient http;
  http.setTimeout(30000);
  
  // Use cache busting to ensure we get the latest version
  String versionUrl = String(VERSION_URL) + "?t=" + String(millis());
  
  if (!http.begin(client, versionUrl)) {
    Serial.println("[OTA] Failed to begin HTTP connection");
    return;
  }
  
  int httpCode = http.GET();
  
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("[OTA] Version check failed: %d\n", httpCode);
    http.end();
    return;
  }
  
  String remoteVersion = http.getString();
  http.end();
  
  remoteVersion.trim();
  
  Serial.print("[OTA] Remote version: ");
  Serial.println(remoteVersion);
  
  // Compare versions
  if (remoteVersion == FW_VERSION) {
    Serial.println("[OTA] Already up to date!");
    Serial.println("-----------------------------\n");
    return;
  }
  
  Serial.println("[OTA] NEW VERSION AVAILABLE!");
  Serial.println("[OTA] Starting firmware download...");
  Serial.println("[OTA] This may take 2-3 minutes...");
  
  // Disable alarm during update
  alarmEnabled = false;
  digitalWrite(BUZZER_PIN, LOW);
  
  // Perform OTA with multiple retries
  for (int retry = 1; retry <= 3; retry++) {
    Serial.printf("[OTA] Attempt %d/3...\n", retry);
    
    if (performOTAUpdate()) {
      // If we get here, update failed but we can retry
      Serial.println("[OTA] Retrying in 5 seconds...");
      delay(5000);
    }
    // If update succeeds, ESP will restart automatically
  }
  
  Serial.println("[OTA] All attempts failed!");
  alarmEnabled = true;
  Serial.println("-----------------------------\n");
}

// =====================================================
// PERFORM OTA UPDATE - ESP32 EDITION (FAST & RELIABLE!)
// =====================================================
bool performOTAUpdate() {
  // Use HTTP for MAXIMUM DOWNLOAD SPEED (3-5x faster than HTTPS!)
  WiFiClient client;
  
  // Configure HTTPUpdate (ESP32 style)
  httpUpdate.setLedPin(LED_PIN, LOW);
  httpUpdate.rebootOnUpdate(true);
  httpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  
  // Set callbacks for progress (ESP32 style)
  httpUpdate.onStart([]() {
    Serial.println("[UPDATE] Download started...");
    Serial.println("[UPDATE] ESP32 POWER - This will be FAST!");
  });
  
  httpUpdate.onProgress([](int current, int total) {
    static int lastPercent = -1;
    int percent = (current * 100) / total;
    if (percent != lastPercent && percent % 5 == 0) {
      Serial.printf("[UPDATE] %d%% (%d/%d bytes)\n", percent, current, total);
      lastPercent = percent;
    }
  });
  
  httpUpdate.onEnd([]() {
    Serial.println("[UPDATE] Download complete, installing...");
  });
  
  httpUpdate.onError([](int error) {
    Serial.printf("[UPDATE] Error: %d\n", error);
  });
  
  // Use cache-busting
  String firmwareUrl = String(FIRMWARE_URL) + "?t=" + String(millis());
  
  Serial.println("[UPDATE] Downloading via HTTP (FAST MODE!)...");
  Serial.println("[UPDATE] ESP32 at MAXIMUM SPEED!");
  Serial.println(firmwareUrl);
  Serial.print("[UPDATE] Free heap: ");
  Serial.println(ESP.getFreeHeap());
  
  // Perform update - ESP32 will do this FAST!
  t_httpUpdate_return result = httpUpdate.update(client, firmwareUrl);
  
  switch (result) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("[UPDATE] FAILED: Error(%d)\n", httpUpdate.getLastError());
      return true;  // Return true to indicate we should retry
      
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("[UPDATE] No update available");
      return false;
      
    case HTTP_UPDATE_OK:
      Serial.println("[UPDATE] SUCCESS! Rebooting...");
      return false;  // Will never reach here due to reboot
  }
  
  return true;
}

// =====================================================
// ALARM TRIGGER
// =====================================================
void triggerAlarm() {
  for (int i = 0; i < 5; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, LOW);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
  }
  digitalWrite(BUZZER_PIN, HIGH);  // Keep buzzer on
}

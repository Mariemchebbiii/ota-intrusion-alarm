/*
 * ESP8266 Intrusion Alarm with OTA Updates
 * Version: 3.00 - ULTIMATE BULLETPROOF EDITION
 *
 * CRITICAL FIXES:
 * - Uses jsDelivr CDN (faster than raw.githubusercontent.com)
 * - 3-minute timeout for slow connections
 * - Optimized SSL buffers
 * - Progress indicator
 * - Multiple retry with delay
 */

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecureBearSSL.h>

// =====================================================
// FIRMWARE VERSION - CHANGE THIS TO TRIGGER OTA
// =====================================================
#define FW_VERSION "2.02"

// =====================================================
// WIFI CONFIGURATION
// =====================================================
const char* WIFI_SSID = "TOPNET_2FB0";
const char* WIFI_PASS = "3m3smnb68l";

// =====================================================
// OTA URLs - Using GitHub raw (simpler SSL for ESP8266)
// =====================================================
const char* VERSION_URL = "https://raw.githubusercontent.com/Mariemchebbiii/ota-intrusion-alarm/main/docs/version.txt";
const char* FIRMWARE_URL = "https://raw.githubusercontent.com/Mariemchebbiii/ota-intrusion-alarm/main/docs/firmware.bin";

// =====================================================
// PIN CONFIGURATION
// =====================================================
const int PIR_PIN = D5;
const int BUZZER_PIN = D1;
const int LED_PIN = LED_BUILTIN;

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
  Serial.println("   ESP8266 ALARM v" FW_VERSION);
  Serial.println("   ULTIMATE BULLETPROOF EDITION");
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
  
  // Create secure client with INSECURE mode (skip certificate validation)
  // This is required because ESP8266 has limited memory for full SSL
  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  
  // Set optimal buffer sizes
  // 4096 for receive, 512 for send - this is enough for SSL handshake
  client.setBufferSizes(4096, 512);
  
  // Step 1: Get remote version
  Serial.println("[OTA] Checking remote version...");
  
  HTTPClient http;
  http.setTimeout(30000);  // 30 second timeout for version check
  
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
  remoteVersion.trim();
  http.end();
  
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
// PERFORM OTA UPDATE
// =====================================================
bool performOTAUpdate() {
  // ULTIMATE FIX: HTTPS with EXTREME optimizations
  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  
  // CRITICAL: 1024 RX buffer minimum for SSL stability, 512 TX buffer
  client.setBufferSizes(1024, 512);
  
  // CRITICAL: Set TCP timeout to 10 MINUTES (600 seconds)
  client.setTimeout(600000);
  
  // Keep connection alive
  client.setNoDelay(true);
  
  // Configure ESPhttpUpdate with MAXIMUM tolerance
  ESPhttpUpdate.setLedPin(LED_PIN, LOW);
  ESPhttpUpdate.rebootOnUpdate(true);
  ESPhttpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
  
  // Disable connection close to keep TCP alive
  ESPhttpUpdate.closeConnectionsOnUpdate(false);
  
  // Set callbacks for progress
  ESPhttpUpdate.onStart([]() {
    Serial.println("[UPDATE] Download started...");
    Serial.println("[UPDATE] BE PATIENT - This takes 3-5 minutes!");
  });
  
  ESPhttpUpdate.onProgress([](int current, int total) {
    static int lastPercent = -1;
    int percent = (current * 100) / total;
    if (percent != lastPercent && percent % 5 == 0) {
      Serial.printf("[UPDATE] %d%% (%d/%d) - Keep waiting!\n", percent, current, total);
      lastPercent = percent;
    }
  });
  
  ESPhttpUpdate.onEnd([]() {
    Serial.println("[UPDATE] Download complete, installing...");
  });
  
  ESPhttpUpdate.onError([](int error) {
    Serial.printf("[UPDATE] Error: %d - %s\n", error, ESPhttpUpdate.getLastErrorString().c_str());
  });
  
  // Use HTTPS with cache-busting
  String firmwareUrl = String(FIRMWARE_URL) + "?t=" + String(millis());
  
  Serial.println("[UPDATE] Downloading via HTTPS...");
  Serial.println("[UPDATE] WARNING: This is SLOW on ESP8266!");
  Serial.println(firmwareUrl);
  Serial.print("[UPDATE] Free heap: ");
  Serial.println(ESP.getFreeHeap());
  
  // Perform update with 10-minute timeout tolerance
  t_httpUpdate_return result = ESPhttpUpdate.update(client, firmwareUrl);
  
  switch (result) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("[UPDATE] FAILED: Error(%d): %s\n", 
                    ESPhttpUpdate.getLastError(),
                    ESPhttpUpdate.getLastErrorString().c_str());
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

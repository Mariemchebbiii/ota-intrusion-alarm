/*
 * ESP8266 Intrusion Alarm with OTA Updates
 * Version: 2.03
 * 
 * Features:
 * - PIR motion detection
 * - WiFi connectivity with retry
 * - OTA firmware updates from GitHub
 * - ArduinoOTA for local updates
 */

#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecureBearSSL.h>

#define FW_VERSION "2.03"

// ============================================
// WiFi Configuration
// ============================================
const char* WIFI_SSID = "TOPNET_2FB0";
const char* WIFI_PASS = "3m3smnb68l";

// ============================================
// OTA Update URLs (MUST be HTTPS for GitHub)
// ============================================
const char* VERSION_URL = "https://raw.githubusercontent.com/Mariemchebbiii/ota-intrusion-alarm/main/docs/version.txt";
const char* FW_URL = "https://raw.githubusercontent.com/Mariemchebbiii/ota-intrusion-alarm/main/docs/firmware.bin";

// ============================================
// Pin Definitions
// ============================================
#define PIR_PIN D5
#define LED_PIN D6
#define BUZZER_PIN D7
#define BTN_PIN D3

// ============================================
// Global Variables
// ============================================
bool armed = true;
bool alarmOn = false;
unsigned long lastOTACheck = 0;
const unsigned long OTA_CHECK_INTERVAL = 300000; // 5 minutes

// ============================================
// WiFi Connection with Extended Retry
// ============================================
bool connectWiFi() {
  Serial.print("Connexion WiFi ");
  Serial.print(WIFI_SSID);
  Serial.println("...");
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  // Wait up to 30 seconds for connection
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 60) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(500);
    attempts++;
    if (attempts % 10 == 0) {
      Serial.printf("Tentative %d/60...\n", attempts);
    }
  }
  
  digitalWrite(LED_PIN, LOW);
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connecte!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Signal (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
    return true;
  } else {
    Serial.println("ERREUR: WiFi connection failed!");
    Serial.println("Verifiez SSID et mot de passe");
    return false;
  }
}

// ============================================
// OTA Update Check
// ============================================
void checkForUpdate() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi non connecte - update check annule");
    return;
  }

  Serial.println("\n========================================");
  Serial.println("=== Verification mise a jour OTA ===");
  Serial.println("========================================");
  Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());

  // Create secure client for HTTPS
  BearSSL::WiFiClientSecure client;
  client.setInsecure(); // Skip certificate verification
  client.setTimeout(15000);

  HTTPClient https;
  https.setTimeout(15000);
  https.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  
  // ===== STEP 1: Get version from server =====
  Serial.print("1. Lecture version: ");
  Serial.println(VERSION_URL);
  
  if (!https.begin(client, VERSION_URL)) {
    Serial.println("   ERREUR: Impossible de se connecter au serveur");
    return;
  }

  int httpCode = https.GET();
  Serial.printf("   HTTP Response: %d\n", httpCode);
  
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("   ERREUR HTTP: %d - %s\n", httpCode, https.errorToString(httpCode).c_str());
    https.end();
    return;
  }

  String serverVersion = https.getString();
  https.end();
  serverVersion.trim();
  
  Serial.printf("   Version locale:  %s\n", FW_VERSION);
  Serial.printf("   Version serveur: %s\n", serverVersion.c_str());

  // Compare versions
  float localVer = String(FW_VERSION).toFloat();
  float serverVer = serverVersion.toFloat();
  
  if (serverVer <= localVer) {
    Serial.println("2. Firmware a jour - aucune mise a jour necessaire");
    Serial.println("========================================\n");
    return;
  }

  // ===== STEP 2: Download and install firmware =====
  Serial.println("2. Nouvelle version detectee!");
  Serial.printf("   Mise a jour: %s -> %s\n", FW_VERSION, serverVersion.c_str());
  Serial.println("3. Telechargement du firmware...");
  Serial.print("   URL: ");
  Serial.println(FW_URL);
  Serial.printf("   Free heap avant: %u bytes\n", ESP.getFreeHeap());
  
  // Create new secure client for firmware download
  BearSSL::WiFiClientSecure updateClient;
  updateClient.setInsecure();
  updateClient.setTimeout(60000); // 60 seconds for large file
  
  // Configure updater
  ESPhttpUpdate.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
  ESPhttpUpdate.rebootOnUpdate(true);
  
  Serial.println("   Demarrage du telechargement...");
  
  t_httpUpdate_return ret = ESPhttpUpdate.update(updateClient, FW_URL);

  switch (ret) {
    case HTTP_UPDATE_OK:
      Serial.println("   SUCCESS! Redemarrage...");
      break;
      
    case HTTP_UPDATE_FAILED:
      Serial.printf("   ECHEC! Erreur %d: %s\n", 
        ESPhttpUpdate.getLastError(),
        ESPhttpUpdate.getLastErrorString().c_str());
      Serial.printf("   Free heap apres: %u bytes\n", ESP.getFreeHeap());
      break;
      
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("   Pas de mise a jour disponible");
      break;
  }
  
  Serial.println("========================================\n");
}

// ============================================
// Setup
// ============================================
void setup() {
  // Initialize pins
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  // Initialize serial
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n========================================");
  Serial.println("    ESP8266 INTRUSION ALARM SYSTEM");
  Serial.println("========================================");
  Serial.printf("Version: %s\n", FW_VERSION);
  Serial.printf("Chip ID: %08X\n", ESP.getChipId());
  Serial.printf("Flash size: %u bytes\n", ESP.getFlashChipSize());
  Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
  Serial.println("========================================\n");

  // Connect to WiFi
  if (connectWiFi()) {
    // Setup ArduinoOTA for local updates
    ArduinoOTA.setHostname("ESP-ALARM");
    ArduinoOTA.setPassword("ota123");
    ArduinoOTA.onStart([]() {
      Serial.println("OTA Update starting...");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nOTA Update complete!");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("OTA Error[%u]: ", error);
    });
    ArduinoOTA.begin();
    
    // Check for OTA update after WiFi connected
    delay(2000);
    checkForUpdate();
  }
  
  Serial.println("Systeme pret - ARMED");
  Serial.println("Appuyez sur le bouton pour armer/desarmer");
}

// ============================================
// Main Loop
// ============================================
void loop() {
  // Handle ArduinoOTA
  if (WiFi.status() == WL_CONNECTED) {
    ArduinoOTA.handle();
  }

  // Periodic OTA check (every 5 minutes)
  if (WiFi.status() == WL_CONNECTED && millis() - lastOTACheck > OTA_CHECK_INTERVAL) {
    lastOTACheck = millis();
    Serial.println("Verification periodique OTA...");
    checkForUpdate();
  }

  // Button handling with debounce
  if (digitalRead(BTN_PIN) == LOW) {
    delay(50); // Debounce
    if (digitalRead(BTN_PIN) == LOW) {
      armed = !armed;
      alarmOn = false;
      Serial.printf(">>> Systeme %s <<<\n", armed ? "ARME" : "DESARME");
      
      // Visual/audio feedback
      for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        digitalWrite(BUZZER_PIN, HIGH);
        delay(50);
        digitalWrite(LED_PIN, LOW);
        digitalWrite(BUZZER_PIN, LOW);
        delay(50);
      }
      
      // Wait for button release
      while (digitalRead(BTN_PIN) == LOW) delay(10);
    }
  }

  // Motion detection
  if (armed && digitalRead(PIR_PIN) == HIGH) {
    if (!alarmOn) {
      Serial.println("!!! INTRUSION DETECTEE !!!");
    }
    alarmOn = true;
  }

  // Alarm output
  if (alarmOn) {
    digitalWrite(LED_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
    delay(150);
    digitalWrite(LED_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
    delay(150);
  }
}

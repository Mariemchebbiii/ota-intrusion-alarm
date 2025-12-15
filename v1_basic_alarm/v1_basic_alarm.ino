#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <WiFiClientSecureBearSSL.h>

#define FW_VERSION "2.00"

// ============================================
// ⚠️ CONFIGURATION - UPDATE THESE VALUES!
// ============================================
const char* WIFI_SSID = "YOUR_WIFI_SSID";      // Your WiFi network name
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";  // Your WiFi password

// GitHub repository settings (CHANGE TO YOUR REPO!)
#define GITHUB_USER "YOUR_GITHUB_USERNAME"     // Your GitHub username
#define GITHUB_REPO "ota-intrusion-alarm"      // Your repository name
#define GITHUB_BRANCH "main"                   // Branch name

// ✅ Auto-generated OTA URLs (no need to modify these)
const char* VERSION_URL =
  "https://raw.githubusercontent.com/" GITHUB_USER "/" GITHUB_REPO "/" GITHUB_BRANCH "/docs/version.txt";
const char* FW_URL =
  "https://raw.githubusercontent.com/" GITHUB_USER "/" GITHUB_REPO "/" GITHUB_BRANCH "/docs/firmware.bin";

  

#define PIR_PIN D5
#define LED_PIN D6
#define BUZZER_PIN D7
#define BTN_PIN D3

bool armed = true;
bool alarmOn = false;
unsigned long lastOTACheck = 0;
const unsigned long OTA_CHECK_INTERVAL = 300000; // Check for updates every 5 minutes

// Test if firmware URL is accessible
bool testFirmwareURL() {
  std::unique_ptr<BearSSL::WiFiClientSecure> testClient(new BearSSL::WiFiClientSecure);
  testClient->setInsecure();
  testClient->setTimeout(5000);
  
  HTTPClient testHttp;
  testHttp.setTimeout(5000);
  
  if (!testHttp.begin(*testClient, FW_URL)) {
    Serial.println("Cannot create HTTP connection to firmware URL");
    return false;
  }
  
  // Use HEAD request to check if file exists (faster)
  int code = testHttp.sendRequest("HEAD");
  Serial.printf("Firmware URL test response: %d\n", code);
  
  if (code == 200) {
    Serial.printf("Firmware file size: %d bytes\n", testHttp.getSize());
  } else {
    Serial.printf("Firmware URL not accessible! HTTP code: %d\n", code);
  }
  
  testHttp.end();
  return (code == 200);
}

void checkForUpdate() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi non connecte - update check cancelled");
    return;
  }

  delay(500); // Allow WiFi to stabilize

  Serial.println("\n=== Verification mise a jour ===");

  // Create secure client for version check
  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  client->setTimeout(10000); // 10 second timeout

  HTTPClient https;
  https.setTimeout(10000);
  
  // Get version from server
  Serial.print("Connexion a ");
  Serial.println(VERSION_URL);
  
  if (!https.begin(*client, VERSION_URL)) {
    Serial.println("HTTP error: connection failed (version check)");
    return;
  }

  int httpCode = https.GET();
  Serial.printf("HTTP GET response: %d\n", httpCode);
  
  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("Erreur GET version: %d\n", httpCode);
    Serial.println(https.errorToString(httpCode).c_str());
    https.end();
    return;
  }

  String payload = https.getString();
  https.end();
  payload.trim();

  // Parse version safely
  String serverVersionStr = payload;
  serverVersionStr.trim();
  
  Serial.print("Version carte: ");
  Serial.println(FW_VERSION);
  Serial.print("Version serveur: ");
  Serial.println(serverVersionStr);

  // Normalize versions for comparison (1.0 == 1.00)
  float localVer = String(FW_VERSION).toFloat();
  float serverVer = serverVersionStr.toFloat();
  
  Serial.printf("Debug: localVer=%.2f, serverVer=%.2f\n", localVer, serverVer);

  // Compare as floats (more reliable for version numbers)
  if (serverVer > localVer) {
    Serial.println("Nouvelle version detectee -> telechargement firmware...");

    // Test firmware URL accessibility first
    Serial.println("\nTest: Verification acces URL firmware...");
    if (!testFirmwareURL()) {
      Serial.println("ERREUR: Impossible d'acceder au fichier firmware !");
      Serial.println("Verifications suggérees:");
      Serial.print("1. L'URL existe: ");
      Serial.println(FW_URL);
      Serial.println("2. Le fichier firmware.bin a été uploadé sur GitHub");
      Serial.println("3. Votre connexion WiFi est stable");
      Serial.println("4. GitHub n'est pas bloqué par votre reseau");
      return;
    }
    
    Serial.println("URL firmware OK, demarrage mise à jour...");
    delay(1000);

    // Create new secure client for firmware download
    std::unique_ptr<BearSSL::WiFiClientSecure> client2(new BearSSL::WiFiClientSecure);
    client2->setInsecure();
    client2->setTimeout(30000); // 30 second timeout for file download
    client2->setBufferSizes(512, 512); // Smaller buffers for stability

    ESPhttpUpdate.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    
    // Add User-Agent to avoid being blocked
    ESPhttpUpdate.setAuthorization(""); // Clear auth if any

    Serial.print("URL firmware: ");
    Serial.println(FW_URL);
    
    Serial.printf("Free heap before update: %u bytes\n", ESP.getFreeHeap());

    t_httpUpdate_return ret = ESPhttpUpdate.update(*client2, FW_URL);

    if (ret == HTTP_UPDATE_OK) {
      Serial.println("Mise a jour reussie ! (reboot imminent)");
    } else {
      Serial.printf("Echec update. Code=%d\n", ESPhttpUpdate.getLastError());
      Serial.print("Erreur: ");
      Serial.println(ESPhttpUpdate.getLastErrorString().c_str());
      
      // Additional debugging
      Serial.println("\nDiagnostique:");
      Serial.printf("Erreur code ESP: %d\n", ESPhttpUpdate.getLastError());
      Serial.printf("Free heap after attempt: %u bytes\n", ESP.getFreeHeap());
      
      // Diagnose specific error codes
      switch(ESPhttpUpdate.getLastError()) {
        case -1:
          Serial.println("ERREUR -1: HTTP status code error OR connection failed");
          Serial.println("→ Vérifiez que firmware.bin existe sur GitHub");
          Serial.println("→ Vérifiez l'URL est accessible");
          break;
        case -2:
          Serial.println("ERREUR -2: Write error - espace flash insuffisant");
          break;
        case -3:
          Serial.println("ERREUR -3: Read error - corruption du fichier");
          break;
        case -4:
          Serial.println("ERREUR -4: End error - somme de contrôle échouée");
          break;
        case -5:
          Serial.println("ERREUR -5: Magic byte error - format binaire incorrect");
          break;
        case -6:
          Serial.println("ERREUR -6: Segment error");
          break;
        case -7:
          Serial.println("ERREUR -7: CRC check error");
          break;
        case -8:
          Serial.println("ERREUR -8: Flash write error");
          break;
        case -9:
          Serial.println("ERREUR -9: New binary is same as current");
          break;
        case -10:
          Serial.println("ERREUR -10: ROM write error");
          break;
      }
    }
  } else {
    Serial.println("Aucune mise a jour ou erreur");
  }
  
  Serial.println("===========================\n");
}

void setup() {
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  Serial.begin(115200);
  delay(1000);
  Serial.println("\n\n=== ESP ALARM v1 STARTUP ===");
  Serial.printf("Version firmware: %s\n", FW_VERSION);
  Serial.printf("Flash size: %u bytes\n", ESP.getFlashChipSize());
  Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());

  Serial.print("Connexion WiFi ");
  Serial.print(WIFI_SSID);
  Serial.println("...");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    delay(300);
    attempts++;
  }
  
  digitalWrite(LED_PIN, LOW);
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi connecte");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("ERREUR: WiFi connection failed");
  }

  ArduinoOTA.setHostname("ESP-ALARM");
  ArduinoOTA.setPassword("ota123");
  ArduinoOTA.begin();

  // Check for updates after 2 seconds to ensure WiFi is stable
  delay(2000);
  checkForUpdate();
}

void loop() {
  ArduinoOTA.handle();

  // Periodic OTA check (every 5 minutes)
  if (millis() - lastOTACheck > OTA_CHECK_INTERVAL) {
    lastOTACheck = millis();
    Serial.println("Periodic OTA check...");
    checkForUpdate();
  }

  // Button handling with debounce
  if (digitalRead(BTN_PIN) == LOW) {
    delay(50); // Debounce
    if (digitalRead(BTN_PIN) == LOW) {
      armed = !armed;
      alarmOn = false;  // Reset alarm when toggling armed state
      Serial.printf("System %s\n", armed ? "ARMED" : "DISARMED");
      
      // Visual feedback
      for (int i = 0; i < 3; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(100);
        digitalWrite(LED_PIN, LOW);
        delay(100);
      }
      
      while (digitalRead(BTN_PIN) == LOW) delay(10); // Wait for release
    }
  }

  // Motion detection
  if (armed && digitalRead(PIR_PIN) == HIGH) {
    if (!alarmOn) {
      Serial.println("⚠️ INTRUSION DETECTED!");
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

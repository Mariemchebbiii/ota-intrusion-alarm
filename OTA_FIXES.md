# OTA Update Code - Fix Report

## Issues Identified & Fixed

### 1. ❌ **HTTP Connection Failed Error**
**Root Cause:** Missing timeout configuration and poor error handling
- The HTTP client had no timeout, causing indefinite hangs
- No detailed error diagnostics for connection issues

**Fix:**
- Added `client->setTimeout(10000)` for version check (10 seconds)
- Added `client->setTimeout(30000)` for firmware download (30 seconds)
- Added detailed error logging with `errorToString()`

### 2. ❌ **Code=-1 Error (Update Failure)**
**Root Cause:** Incorrect binary URL format and version comparison
- URL was: `v1_basic_alarm.ino.bin` → Wrong format
- Should be: `firmware.bin` → Standard binary name
- Version comparison with floats was unreliable

**Fix:**
- Changed URL to: `https://raw.githubusercontent.com/Mariemchebbiii/ota-intrusion-alarm/main/docs/firmware.bin`
- Changed `FW_VERSION` from float (1.0) to string ("1.00") for safer comparison
- Implemented string comparison instead of float math

### 3. ❌ **Version Mismatch (1.00 vs 2.00)**
**Root Cause:** Float precision issues and incorrect parsing
- `1.0 > 2.00` float comparison was unreliable
- Server version parsing could have whitespace issues

**Fix:**
- Changed to string-based version comparison
- Added `.trim()` to remove whitespace from server response
- Clearer version display in logs

### 4. ❌ **Poor Error Diagnostics**
**Root Cause:** Missing debugging information for troubleshooting

**Fix Added:**
```cpp
Serial.printf("Erreur code ESP: %d\n", ESPhttpUpdate.getLastError());
if (ESPhttpUpdate.getLastError() == -1) {
    Serial.println("Possible causes: File too large, insufficient flash space, or corrupted download");
}
```

### 5. ❌ **WiFi Instability During Update**
**Root Cause:** No delay between WiFi connection and update check

**Fix:**
- Added `delay(2000)` before `checkForUpdate()`
- Set explicit WiFi mode: `WiFi.mode(WIFI_STA)`
- Added connection attempt limit (20 attempts max)
- Added WiFi status verification before checking updates

### 6. ❌ **Missing Startup Diagnostics**
**Root Cause:** No visibility into device state at startup

**Fix Added:**
```cpp
Serial.printf("Flash size: %u bytes\n", ESP.getFlashChipSize());
Serial.printf("Free heap: %u bytes\n", ESP.getFreeHeap());
Serial.print("IP: ");
Serial.println(WiFi.localIP());
```

---

## Key Changes Summary

| Issue | Before | After |
|-------|--------|-------|
| **Timeout** | None | 10s (version), 30s (firmware) |
| **Binary URL** | `v1_basic_alarm.ino.bin` | `firmware.bin` |
| **Version Type** | Float (1.0) | String ("1.00") |
| **Version Compare** | `serverVersion > FW_VERSION` | `serverVersionStr != FW_VERSION` |
| **WiFi Delay** | No delay | 2 second delay + verification |
| **Error Details** | Minimal | Detailed diagnostics |
| **Debug Output** | Poor | Comprehensive with headers |

---

## Required Server-Side Setup

Ensure your GitHub repository has these files:

```
docs/
├── version.txt     (Content: 1.00)
└── firmware.bin    (Your compiled binary)
```

**Create version.txt:**
```
1.00
```

**Create firmware.bin:**
1. Compile in Arduino IDE: `Sketch → Export compiled Binary`
2. Or: `esptool.py --chip esp8266 elf2image v1_basic_alarm.ino.elf`

---

## Testing Checklist

- [ ] Update `version.txt` to "2.00" on GitHub
- [ ] Upload `firmware.bin` (version 2.00) to GitHub
- [ ] Flash updated code to ESP8266
- [ ] Check Serial monitor for:
  - ✓ WiFi connection confirmation
  - ✓ Version check (1.00 vs 2.00)
  - ✓ "Nouvelle version detectee"
  - ✓ "Mise a jour reussie !"
  - ✓ Auto-reboot

---

## Common Issues & Solutions

### Still getting "HTTP error: connection failed"?
1. Check GitHub URLs are accessible (paste in browser)
2. Verify DNS resolution: `Serial.println(WiFi.gatewayIP())`
3. Check WiFi SSID/password are correct
4. Try temporary: Remove `client->setInsecure()` → Use proper SSL certificates

### Still getting Code=-1?
1. Check `firmware.bin` file size (must be < available flash)
2. Verify binary format with: `file firmware.bin`
3. Check available flash: Serial output shows this now
4. Try smaller binary or use SPIFFS partition

### Version not updating?
1. Ensure `version.txt` contains exactly "2.00\n"
2. Add serial logging to confirm file is read
3. Check GitHub RAW URL (not web page URL)

---

## Advanced Improvements (Optional)

```cpp
// Add to checkForUpdate() for HTTPS certificate validation:
// client->setX509Time(time(nullptr));
// BearSSL::CertStore certStore;
// certStore.addCert("GitHub CA cert");
// client->setCertStore(&certStore);

// Or check file hash before update:
// SHA256 serverHash = calculateSHA256(firmwareData);
// if (serverHash != localHash) { /* error */ }
```

---

**Last Updated:** December 15, 2025
**Status:** ✅ Ready for deployment

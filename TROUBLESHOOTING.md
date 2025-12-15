# OTA Update - Complete Troubleshooting & Fix Guide

## Current Status: PARTIALLY WORKING ✓⚠️

**What's Working:**
- ✅ WiFi connection
- ✅ Version check (HTTP 200)
- ✅ Version comparison now works (1.0 == 1.00)

**What's Failing:**
- ❌ Firmware download returns "HTTP error: connection failed"

---

## Root Cause Analysis

### Issue: Firmware Download Fails While Version Check Succeeds

**Why This Happens:**
1. Version file is text (small, fast to download) ✅
2. Firmware binary file is large (may not exist at URL) ❌
3. SSL/TLS connection drops during large file transfer
4. Memory constraints during download
5. Firmware URL is incorrect or file doesn't exist

---

## Changes Made in Latest Update

### 1. **Fixed Version Comparison**
```cpp
// Before: String comparison (1.0 != 1.00)
if (serverVersionStr != FW_VERSION)

// After: Float comparison (1.0 == 1.00)
float localVer = String(FW_VERSION).toFloat();
float serverVer = serverVersionStr.toFloat();
if (serverVer > localVer)
```
✅ Now "1.0" and "1.00" are treated as the same version

### 2. **Added URL Accessibility Test**
```cpp
bool testFirmwareURL() {
  // Tests if firmware.bin actually exists
  // Returns file size
  // Gives clear error messages
}
```
This runs BEFORE attempting the full download

### 3. **Enhanced Error Diagnostics**
Now shows specific error codes:
- `-1` = HTTP connection failed or status error
- `-2` = Not enough flash space
- `-3` = File corruption
- `-4` = Checksum failure
- `-5` = Invalid binary format
- etc.

---

## What You Need to Do NOW

### Step 1: Verify GitHub Repository Structure

Your GitHub repo must have this exact structure:

```
ota-intrusion-alarm/
├── docs/
│   ├── version.txt      ← Contains: "1.00"
│   └── firmware.bin     ← Your compiled binary (NOT .ino)
├── README.md
└── ...
```

### Step 2: Create version.txt

1. Go to: https://github.com/Mariemchebbiii/ota-intrusion-alarm
2. Navigate to `docs/` folder (create if doesn't exist)
3. Click "Add file" → "Create new file"
4. Name: `version.txt`
5. Content: `1.00`
6. Commit

### Step 3: Create firmware.bin

**Option A: Arduino IDE (Recommended)**
1. Open `v1_basic_alarm.ino` in Arduino IDE
2. `Sketch` → `Export compiled Binary`
3. This creates `v1_basic_alarm.ino.bin` in the project folder
4. Rename to `firmware.bin`

**Option B: Using esptool.py**
```bash
esptool.py --chip esp8266 elf2image v1_basic_alarm.ino.elf -o firmware.bin
```

**Option C: From VS Code with Arduino Extension**
1. Right-click sketch
2. "Arduino: Export compiled sketch"
3. Rename output to `firmware.bin`

### Step 4: Upload firmware.bin to GitHub

1. Go to `docs/` folder on GitHub
2. Click "Add file" → "Upload files"
3. Select your compiled `firmware.bin`
4. Commit

### Step 5: Test the Connection

Flash the updated code with these improvements:
```
Version comparison fixed (1.0 == 1.00) ✓
URL accessibility test added ✓
Better error codes and diagnostics ✓
```

Monitor Serial output:
```
WiFi connecte ✓
HTTP GET response: 200 ✓
Firmware URL test response: 200 ✓
[Update should work now]
```

---

## If Still Failing: Debug Checklist

### "HTTP error: connection failed"

- [ ] Is `firmware.bin` uploaded to GitHub `docs/` folder?
- [ ] Can you access the URL in a browser? Try:
  ```
  https://raw.githubusercontent.com/YOUR_USERNAME/ota-intrusion-alarm/main/docs/firmware.bin
  ```
- [ ] Is the file size reasonable? (Should be 200KB-500KB for ESP8266)
- [ ] Is GitHub blocking automated downloads? (Unlikely but possible)
- [ ] Is your WiFi stable? (Retest with better signal)

### "Firmware file size: 0 bytes"

- Firmware binary is corrupted or not compiled correctly
- Re-export from Arduino IDE: `Sketch → Export compiled Binary`

### "Free heap before update: 50112 bytes"

- **CRITICAL**: 50KB free heap is VERY LOW
- ESP8266 needs ~200KB for update
- **Solution**: Add memory optimization to setup()

```cpp
void setup() {
  // ... pin setup ...
  Serial.begin(115200);
  delay(1000);
  
  // Release unused memory
  WiFi.disconnect(true); // Turn off WiFi before version check
  
  // ... WiFi setup ...
  checkForUpdate();
  // WiFi will be needed anyway, so reconnect after
}
```

---

## Memory Issue Solutions (If Needed)

### Option 1: Reduce Sketch Size
Remove unused libraries from securite.ino:
```cpp
// Remove if not using
// #include <SomeUnusedLibrary.h>
```

### Option 2: Use SPIFFS for Update
```cpp
// Instead of RAM, use SPIFFS
// ESPhttpUpdate.setLedPin(LED_BUILTIN);
// This reserves less RAM
```

### Option 3: Check Flash Layout
Add to Serial output:
```cpp
Serial.printf("Flash Size: %u (real: %u)\n", 
  ESP.getFlashChipSize(), 
  ESP.getFlashChipRealSize());
Serial.printf("Sketch size: %u\n", ESP.getSketchSize());
Serial.printf("OTA size: %u\n", ESP.getFreeSketchSpace());
```

---

## Complete Test Sequence

1. **Upload new code with fixes:**
   - Version comparison works (1.0 == 1.00) ✓
   - URL test added ✓
   - Better diagnostics ✓

2. **Prepare GitHub (one-time):**
   - Create `docs/version.txt` with "1.00"
   - Create `docs/firmware.bin` (compiled binary)

3. **Run on ESP8266:**
   - Monitor Serial @ 115200 baud
   - Check if version.txt downloads ✓
   - Check if firmware.bin is accessible ✓
   - Attempt download

4. **Update to v2.00:**
   - Change `docs/version.txt` to "2.00"
   - Upload new `docs/firmware.bin` (recompile)
   - Restart ESP8266
   - Should detect and update ✓

---

## Code Changes Summary

| Change | Purpose | Impact |
|--------|---------|--------|
| Float version comparison | Handle 1.0 vs 1.00 | ✅ Fixes false version detections |
| `testFirmwareURL()` function | Verify file exists before download | ✅ Better error diagnosis |
| Error code diagnostics | Identify exact failure reason | ✅ Faster troubleshooting |
| `setBufferSizes(512, 512)` | Lower memory pressure | ✅ More stable on low-memory devices |
| Free heap monitoring | Show memory usage | ✅ Identify memory constraints |
| HTTP HEAD request | Check file without full download | ✅ Faster than full GET |

---

**Next Update Target:** 2.00 once GitHub is ready
**Status:** Ready to test with proper GitHub setup

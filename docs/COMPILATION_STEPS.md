# OTA Firmware Compilation & Upload Guide

## STEP 1: Compile Firmware in Arduino IDE (REQUIRED)

### Important: You MUST compile the firmware yourself because:
- Each ESP8266 device is unique
- The binary is device-specific and includes your WiFi credentials
- Only Arduino IDE can properly compile with all dependencies

### How to compile:

1. **Open Arduino IDE**
2. **Open your sketch:** `File → Open → v1_basic_alarm.ino`
3. **Select your board:**
   - `Tools → Board → ESP8266 Boards → NodeMCU 1.0` (or your specific board)
   - `Tools → Port → COM3` (or your ESP8266's port)

4. **Export compiled binary:**
   - `Sketch → Export compiled Binary`
   - This creates a file: `v1_basic_alarm.ino.bin` in your sketch folder

5. **Rename the file:**
   - Rename: `v1_basic_alarm.ino.bin` → `firmware.bin`

6. **Copy to docs folder:**
   ```
   Copy "firmware.bin" to: C:\Users\marie\OneDrive\Bureau\Proj Securité\docs\
   ```

---

## STEP 2: Upload files to GitHub

### Files to upload to `docs/` folder:

1. **version.txt** (already created ✓)
   - Location: `docs/version.txt`
   - Content: `1.00`

2. **firmware.bin** (you create from Arduino IDE)
   - Location: `docs/firmware.bin`
   - Your compiled binary

### How to upload to GitHub:

**Option A: Using GitHub Web Interface (Easiest)**

1. Go to: https://github.com/Mariemchebbiii/ota-intrusion-alarm
2. Click **"Add file" → "Upload files"**
3. Drag & drop your compiled `firmware.bin`
4. Make sure it goes into `docs/` folder (should show: `docs/firmware.bin`)
5. Click **"Commit changes"**

**Option B: Using Git (from Terminal)**

```powershell
cd "C:\Users\marie\OneDrive\Bureau\Proj Securité"
git add docs/
git commit -m "Add OTA firmware and version files"
git push origin main
```

---

## STEP 3: Verify on GitHub

After uploading, check that your repo has:

```
ota-intrusion-alarm/
├── docs/
│   ├── version.txt    ✓
│   └── firmware.bin   ✓
├── secure/
├── v1_basic_alarm/
└── ...
```

---

## STEP 4: Test OTA Update

1. **Flash your ESP8266** with the updated code
2. **Monitor Serial output** (115200 baud):
   - Should see "HTTP GET response: 200" (success!)
   - Should see "Firmware file size: XXXXX bytes"
   - Should start downloading firmware

---

## If you still get 404 errors:

- Verify URLs are correct in your code:
  ```cpp
  const char* VERSION_URL =
    "https://raw.githubusercontent.com/Mariemchebbiii/ota-intrusion-alarm/main/docs/version.txt";
  const char* FW_URL =
    "https://raw.githubusercontent.com/Mariemchebbiii/ota-intrusion-alarm/main/docs/firmware.bin";
  ```

- Test URLs directly in browser:
  - https://raw.githubusercontent.com/Mariemchebbiii/ota-intrusion-alarm/main/docs/version.txt
  - https://raw.githubusercontent.com/Mariemchebbiii/ota-intrusion-alarm/main/docs/firmware.bin

---

**Status:** 
✅ version.txt created locally (ready to upload)
⏳ firmware.bin (you create from Arduino IDE)

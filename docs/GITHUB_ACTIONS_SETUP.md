# 🔧 Complete GitHub Actions Setup Guide

## Step-by-Step Instructions (100% Guaranteed to Work!)

---

## 📝 Step 1: Create GitHub Repository

1. Go to **https://github.com/new**
2. Fill in:
   - **Repository name:** `ota-intrusion-alarm`
   - **Description:** `ESP8266 Intrusion Alarm with OTA Updates`
   - **Visibility:** Public (recommended for OTA) or Private
3. **DO NOT** initialize with README (we have our own)
4. Click **"Create repository"**

---

## 📤 Step 2: Push Your Code to GitHub

Open PowerShell in your project folder and run:

```powershell
# Navigate to project folder
cd "C:\Users\cheri\Downloads\Proj Securité\Proj Securité"

# Initialize git (if not done)
git init
git branch -M main

# Add all files
git add .

# Commit
git commit -m "Initial commit with GitHub Actions CI/CD"

# Add your GitHub remote (replace with YOUR username!)
git remote add origin https://github.com/YOUR_USERNAME/ota-intrusion-alarm.git

# Push to GitHub
git push -u origin main
```

---

## ⚙️ Step 3: Enable Workflow Permissions (CRITICAL!)

1. Go to your repository on GitHub
2. Click **Settings** (top right)
3. Scroll down and click **Actions** → **General**
4. Scroll to **"Workflow permissions"**
5. Select: **✅ Read and write permissions**
6. Check: **✅ Allow GitHub Actions to create and approve pull requests**
7. Click **Save**

---

## 🚀 Step 4: Trigger Your First Build

### Option A: Manual Trigger
1. Go to **Actions** tab
2. Click **"Build and Release ESP8266 Firmware"** (left sidebar)
3. Click **"Run workflow"** (right side)
4. Click **"Run workflow"** button
5. Watch the build progress!

### Option B: Create a Tag (Creates Official Release)
```powershell
git tag v2.00
git push origin v2.00
```

---

## ✅ Step 5: Verify Everything Works

After the workflow completes:

1. **Check Actions Tab:**
   - Should show green checkmark ✅

2. **Check Releases:**
   - Go to repository main page
   - Click **"Releases"** (right sidebar)
   - Should see your release with firmware files

3. **Check OTA Files:**
   - Go to `docs/` folder in your repo
   - Should contain `firmware.bin` and `version.txt`

4. **Test OTA URLs:**
   - Open in browser: `https://raw.githubusercontent.com/YOUR_USERNAME/ota-intrusion-alarm/main/docs/version.txt`
   - Should show version number

---

## 🔄 How to Create New Releases

### For version updates:

1. **Edit version in code:**
   ```cpp
   // In v1_basic_alarm.ino, change:
   #define FW_VERSION "2.01"  // Increment this
   ```

2. **Commit and tag:**
   ```powershell
   git add .
   git commit -m "Bump version to 2.01"
   git tag v2.01
   git push origin main --tags
   ```

3. **GitHub Actions will automatically:**
   - Compile both sketches
   - Create a new release with binaries
   - Update `docs/firmware.bin` and `docs/version.txt`
   - Your ESP8266 devices will automatically download the update!

---

## 📡 OTA Update Flow

```
┌─────────────────┐
│  You push code  │
│  with new tag   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ GitHub Actions  │
│ compiles code   │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Creates Release │
│ + Updates docs/ │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│   ESP8266       │
│ checks version  │◄─── Periodically or on boot
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ Downloads new   │
│ firmware.bin    │
└────────┬────────┘
         │
         ▼
┌─────────────────┐
│ ESP8266 reboots │
│ with new code!  │
└─────────────────┘
```

---

## 🐛 Troubleshooting

### ❌ Workflow fails with "Permission denied"
**Solution:** Enable workflow permissions (Step 3)

### ❌ Compilation errors
**Solution:** Make sure code compiles in Arduino IDE first

### ❌ Release not created
**Solution:** Tag must start with `v` (e.g., `v2.01`, not `2.01`)

### ❌ OTA update returns 404
**Solution:** 
1. Wait 1-2 minutes after release
2. Check `docs/firmware.bin` exists in repo
3. Verify URLs in your code match repository path

### ❌ ESP8266 not updating
**Solution:**
1. Check WiFi connection
2. Verify version.txt has higher version than device
3. Check serial monitor for error messages

---

## 📋 File Reference

| File | Purpose |
|------|---------|
| `.github/workflows/build-and-release.yml` | Main CI/CD workflow |
| `.github/workflows/test-build.yml` | Simple test build |
| `docs/version.txt` | Current version (auto-updated) |
| `docs/firmware.bin` | OTA firmware (auto-updated) |
| `securite/securite.ino` | Main sketch |
| `v1_basic_alarm/v1_basic_alarm.ino` | OTA-enabled sketch |

---

## 🎉 You're Done!

Your project now has:
- ✅ Automatic compilation on every push
- ✅ Automatic releases with compiled binaries
- ✅ Automatic OTA file updates
- ✅ ESP8266 devices auto-update!

**No more manual compilation or file uploads!** 🚀

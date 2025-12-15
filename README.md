# 🚨 ESP8266 Intrusion Alarm with OTA Updates

An ESP8266-based intrusion detection system with **automatic Over-The-Air (OTA) firmware updates** via GitHub Actions.

---

## ✨ Features

- 🔔 PIR motion detection with buzzer alarm
- 📡 WiFi connectivity
- 🔄 **Automatic OTA updates** from GitHub
- 🏗️ **GitHub Actions CI/CD** - auto-builds and releases
- 🏷️ **Version-based releases** - version is extracted from code!

---

## 🚀 Quick Start

### 1. Configure Your Settings

Edit `v1_basic_alarm/v1_basic_alarm.ino`:

```cpp
// ⚠️ CHANGE THESE VALUES!
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

#define GITHUB_USER "YOUR_GITHUB_USERNAME"
#define GITHUB_REPO "ota-intrusion-alarm"
```

### 2. Push to GitHub

```bash
git add .
git commit -m "Configure for my setup"
git push origin main
```

### 3. Done! 🎉

GitHub Actions will automatically:
- ✅ Compile both sketches
- ✅ Create a release with tag matching `FW_VERSION`
- ✅ Upload `firmware.bin` for OTA updates

---

## 📦 How Releases Work

The workflow **automatically extracts the version** from your code:

```cpp
#define FW_VERSION "2.00"  // ← This becomes the release tag!
```

**To create a new release:**
1. Change `FW_VERSION` in your code (e.g., `"2.01"`)
2. Commit and push
3. GitHub Actions creates release `v2.01` automatically!

---

## 🔧 Hardware Setup

| Component | Pin |
|-----------|-----|
| PIR Sensor | D5 |
| LED | D6 |
| Buzzer | D7 |
| Button | D3 |

---

## 📡 OTA Update Flow

```
┌──────────────────┐
│ Change FW_VERSION│
│ in code & push   │
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│  GitHub Actions  │
│  compiles code   │
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│ Creates release  │
│ v{FW_VERSION}    │
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│ Updates docs/    │
│ firmware.bin     │
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│ ESP8266 checks   │
│ every 5 minutes  │
└────────┬─────────┘
         │
         ▼
┌──────────────────┐
│ Auto-downloads   │
│ new firmware!    │
└──────────────────┘
```

---

## ⚙️ Repository Setup (One Time)

### 1. Enable Workflow Permissions

Go to **Settings → Actions → General → Workflow permissions**:
- ✅ Select **"Read and write permissions"**
- ✅ Check **"Allow GitHub Actions to create and approve pull requests"**
- Click **Save**

### 2. That's it!

No secrets or tokens needed - uses built-in `GITHUB_TOKEN`.

---

## 📁 Project Structure

```
├── .github/workflows/
│   ├── build-and-release.yml   # Main CI/CD workflow
│   └── test-build.yml          # Simple test
├── docs/
│   ├── firmware.bin            # Auto-updated by CI
│   └── version.txt             # Auto-updated by CI
├── securite/
│   └── securite.ino            # Basic alarm sketch
├── v1_basic_alarm/
│   └── v1_basic_alarm.ino      # OTA-enabled sketch (main)
└── README.md
```

---

## 🐛 Troubleshooting

### Build fails?
- Check Actions tab for error logs
- Ensure code compiles locally first

### OTA not updating?
- Verify `GITHUB_USER` and `GITHUB_REPO` in code
- Check Serial Monitor for error messages
- Ensure `docs/firmware.bin` exists in repo

### Release not created?
- Version must be different from existing tags
- Check workflow permissions in Settings

---

## 📋 Version History

| Version | Changes |
|---------|---------|
| 2.00 | Initial release with OTA |

---

**Made with ❤️ for ESP8266 IoT Projects**

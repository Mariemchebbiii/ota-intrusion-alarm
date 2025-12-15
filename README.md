# 🚀 ESP8266 Alarm System - GitHub Actions CI/CD

## ✅ Automated Build & Release System

This repository uses GitHub Actions for **automatic compilation and release** of ESP8266 firmware.

---

## 📋 How It Works

### Automatic Triggers:
| Event | Action |
|-------|--------|
| Push to `main`/`master` | Builds firmware + Creates "latest" pre-release |
| Push a tag `v*` (e.g., `v2.01`) | Builds firmware + Creates official release |
| Pull Request | Builds firmware (verification only) |
| Manual trigger | Builds with custom version number |

---

## 🎯 Creating a New Release

### Method 1: Tag-based Release (Recommended)
```bash
# Update version in code first (v1_basic_alarm.ino)
# Change: #define FW_VERSION "2.01"

git add .
git commit -m "Bump version to 2.01"
git tag v2.01
git push origin main --tags
```

### Method 2: Manual Workflow Dispatch
1. Go to **Actions** tab on GitHub
2. Select **"Build and Release ESP8266 Firmware"**
3. Click **"Run workflow"**
4. Enter version number (optional)
5. Click **"Run workflow"**

---

## 📦 Release Contents

Each release includes:

| File | Description |
|------|-------------|
| `securite_firmware.bin` | Main security sketch |
| `v1_basic_alarm_firmware.bin` | V1 alarm with OTA support |
| `firmware.bin` | OTA-ready firmware |
| `version.txt` | Version number for OTA checks |

---

## 📡 OTA Update System

The firmware automatically checks for updates using these URLs:
- **Version:** `https://raw.githubusercontent.com/YOUR_USERNAME/YOUR_REPO/main/docs/version.txt`
- **Firmware:** `https://raw.githubusercontent.com/YOUR_USERNAME/YOUR_REPO/main/docs/firmware.bin`

The GitHub Action **automatically updates** `docs/version.txt` and `docs/firmware.bin` on every release!

---

## ⚙️ Workflow Features

- ✅ **Automatic ESP8266 core installation**
- ✅ **Compiles both sketches** (securite + v1_basic_alarm)
- ✅ **Creates GitHub releases with all binaries**
- ✅ **Updates OTA files in docs/ folder**
- ✅ **Verifies OTA URLs are accessible**
- ✅ **Works on push, tags, and manual trigger**

---

## 🔧 Required Repository Settings

### 1. Enable Workflow Permissions
Go to: **Settings → Actions → General → Workflow permissions**
- Select: **"Read and write permissions"**
- Check: **"Allow GitHub Actions to create and approve pull requests"**

### 2. No Secrets Required!
The workflow uses `GITHUB_TOKEN` which is automatically provided.

---

## 📁 Repository Structure

```
├── .github/
│   └── workflows/
│       └── build-and-release.yml    # CI/CD workflow
├── docs/
│   ├── version.txt                  # Auto-updated by CI
│   ├── firmware.bin                 # Auto-updated by CI
│   └── COMPILATION_STEPS.md
├── securite/
│   └── securite.ino                 # Main sketch
├── v1_basic_alarm/
│   └── v1_basic_alarm.ino           # OTA-enabled sketch
└── README.md
```

---

## 🐛 Troubleshooting

### Build Fails?
1. Check the Actions tab for error logs
2. Ensure sketch compiles locally in Arduino IDE first
3. Check that all `#include` statements use standard library names

### OTA Not Working?
1. Verify `docs/firmware.bin` exists in repository
2. Check that `docs/version.txt` has correct version number
3. Test URLs manually in browser
4. Ensure ESP8266 has internet access

### Release Not Created?
1. For tagged releases, tag must start with `v` (e.g., `v2.01`)
2. Check workflow permissions in repository settings
3. Look at Actions tab for errors

---

## 📞 Support

If you encounter issues:
1. Check the **Actions** tab for build logs
2. Open an **Issue** with error details
3. Include your Arduino IDE version and ESP8266 core version

---

**Made with ❤️ for ESP8266 IoT Projects**

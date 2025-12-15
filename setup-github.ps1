# GitHub Repository Setup Script
# Run this to initialize your Git repository and push to GitHub

# ============================================
# CONFIGURATION - UPDATE THESE VALUES!
# ============================================
$GITHUB_USERNAME = "Mariemchebbiii"  # Your GitHub username
$REPO_NAME = "ota-intrusion-alarm"    # Your repository name

# ============================================
# DO NOT MODIFY BELOW THIS LINE
# ============================================

Write-Host "🚀 ESP8266 Alarm System - GitHub Setup Script" -ForegroundColor Cyan
Write-Host "================================================" -ForegroundColor Cyan

# Initialize git if not already done
if (-not (Test-Path ".git")) {
    Write-Host "📁 Initializing Git repository..." -ForegroundColor Yellow
    git init
    git branch -M main
}

# Add all files
Write-Host "📦 Adding all files..." -ForegroundColor Yellow
git add .

# Commit
Write-Host "💾 Creating initial commit..." -ForegroundColor Yellow
git commit -m "🎉 Initial commit with GitHub Actions CI/CD"

# Add remote (if not exists)
$remoteExists = git remote | Select-String "origin"
if (-not $remoteExists) {
    Write-Host "🔗 Adding GitHub remote..." -ForegroundColor Yellow
    git remote add origin "https://github.com/$GITHUB_USERNAME/$REPO_NAME.git"
}

Write-Host ""
Write-Host "✅ Local setup complete!" -ForegroundColor Green
Write-Host ""
Write-Host "📋 Next Steps:" -ForegroundColor Cyan
Write-Host "1. Create repository on GitHub: https://github.com/new" -ForegroundColor White
Write-Host "   - Name: $REPO_NAME" -ForegroundColor Gray
Write-Host "   - Make it Public or Private" -ForegroundColor Gray
Write-Host ""
Write-Host "2. Push to GitHub:" -ForegroundColor White
Write-Host "   git push -u origin main" -ForegroundColor Gray
Write-Host ""
Write-Host "3. Enable Workflow Permissions:" -ForegroundColor White
Write-Host "   Go to: Settings → Actions → General → Workflow permissions" -ForegroundColor Gray
Write-Host "   Select: 'Read and write permissions'" -ForegroundColor Gray
Write-Host ""
Write-Host "4. Create a release:" -ForegroundColor White
Write-Host "   git tag v2.00" -ForegroundColor Gray
Write-Host "   git push origin v2.00" -ForegroundColor Gray
Write-Host ""
Write-Host "🎉 Done! Your firmware will be automatically compiled and released!" -ForegroundColor Green

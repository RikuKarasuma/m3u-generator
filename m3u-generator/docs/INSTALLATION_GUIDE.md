# M3U Generator - Deployment Guide for RockPi

## Overview
This document provides step-by-step instructions for deploying the C++20-based M3U playlist generator on a Debian-based RockPi device via systemd timer.

---

## Prerequisites
- **OS:** Debian 11/12 (bullseye/bookworm)
- **Hardware:** RockPi or compatible ARM64/x86_64 device
- **WebDAV Server:** Running and accessible
- **Storage:** `/var/lib/m3u-generator/playlists` directory writable

---

## Quick Start Installation

### Step 1: Copy Project Files to Device
```bash
# SSH into RockPi and copy project files (adjust source path if deployed differently)
scp -r /project/m3u-generator/* root@rockpi:/opt/
ssh root@rockpi "mkdir -p /opt/m3u-generator"
scp -r /project/m3u-generator/* root@rockpi:/opt/m3u-generator/
```

### Step 2: Set Environment Variables (Required!)

Before running the installation script, set these environment variables:

| Variable | Description | Example Value | Required? |
|----------|-------------|---------------|-----------|
| `M3U_GENERATOR_WEBDAV_URL` | Base URL of WebDAV server | `http://192.168.1.100:80/dav` | ✅ **YES** |
| `M3U_GENERATOR_PLAYLIST_DIR` | Local directory for playlists | `/var/lib/m3u-generator/playlists` | ✅ **YES** |
| `M3U_GENERATOR_OUTPUT_FILE` | Path to generated M3U file | `/var/www/html/playlist.m3u` | ⚠️ Optional (default: `/var/www/html/playlist.m3u`) |
| `M3U_GENERATOR_LOG_LEVEL` | Logging verbosity | `info`, `debug`, `warn`, `error` | ❌ No (default: `info`) |

### Step 3: Run Installation Script
```bash
# Navigate to project directory
cd /opt/m3u-generator

# Run installation with environment variables
M3U_GENERATOR_WEBDAV_URL="http://192.168.1.100:80/dav" \
M3U_GENERATOR_PLAYLIST_DIR="/var/lib/m3u-generator/playlists" \
M3U_GENERATOR_OUTPUT_FILE="/var/www/html/playlist.m3u" \
M3U_GENERATOR_LOG_LEVEL="info" \
./scripts/install.sh
```

### Step 4: Verify Installation

Check that all components are properly installed:

```bash
# Check service status
systemctl status m3u-generator.timer
systemctl status m3u-generator.service

# Check logs for recent activity
journalctl -u m3u-generator -n 20 --no-pager

# Verify generated playlist file exists
ls -la /var/www/html/playlist.m3u

# Test health check script
/opt/m3u-generator/scripts/health-check.sh
```

### Step 5: Manual Verification (Optional)

Test the generator manually:

```bash
# Run once immediately
systemctl start m3u-generator.service

# Check latest log entries
tail -f /var/log/m3u-generator/m3u-generator.log

# Wait a few seconds, then check output file
sleep 5 && cat /var/www/html/playlist.m3u
```

---

## Configuration Options

### WebDAV URL Formats

| Protocol | Example | Notes |
|----------|---------|-------|
| HTTP (Local) | `http://192.168.1.100:80/dav` | LAN access, no SSL |
| HTTPS (Local) | `https://webdav.local:443/dav` | Local with SSL/TLS |
| HTTPS (External) | `https://mymedia.example.com/dav` | Public IP with SSL |

### Playlist Directory Permissions

The script creates the playlist directory with secure permissions:
```bash
# Default permissions set by install.sh
drwxr-x--- 2 root root /var/lib/m3u-generator/playlists
-rw-r----- 1 root root /var/www/html/playlist.m3u
```

### Log Rotation

Log files are rotated daily with 14-day retention:
- **Location:** `/var/log/m3u-generator/`
- **Max size:** 10MB per file
- **Rotation frequency:** Daily at midnight
- **Retention period:** 14 days

---

## Uninstallation

To remove the M3U generator completely:

```bash
# Navigate to project directory
cd /opt/m3u-generator

# Run uninstallation script
./scripts/uninstall.sh
```

This will:
- Stop and disable systemd units
- Remove config files
- Clean up log files (respects logrotate retention)
- Delete the binary from `/usr/local/bin/`

---

## Troubleshooting

### Common Issues

| Issue | Solution |
|-------|----------|
| Service won't start | Check WebDAV URL is accessible: `curl -I http://webdav.example.com/dav` |
| Playlist file not generated | Verify directory permissions: `ls -la /var/lib/m3u-generator/playlists/` |
| Log shows connection errors | Check firewall rules and WebDAV server accessibility |
| Timer doesn't run | Run `systemctl daemon-reload && systemctl enable --now m3u-generator.timer` |

### Debug Mode

Enable verbose logging for troubleshooting:

```bash
M3U_GENERATOR_LOG_LEVEL="debug" \
./scripts/install.sh
```

Then monitor logs:

```bash
journalctl -u m3u-generator -f
```

---

## Security Considerations

The deployment includes these security measures:

1. **Service hardening:** No network access except to WebDAV target
2. **Restricted file permissions:** 640 for config files, 750 for data directories
3. **Log rotation:** Prevents log flooding attacks
4. **No internet access:** Service only connects to configured WebDAV URL

---

## Support

- **Project Repository:** `https://github.com/RikuKarasuma/m3u-generator`
- **Configuration Template:** `/opt/m3u-generator/config/generator.conf`
- **Health Check Script:** `/opt/m3u-generator/scripts/health-check.sh`

---

## Quick Reference Commands

```bash
# Start service (immediate)
systemctl start m3u-generator.service

# Enable timer (recurring)
systemctl enable --now m3u-generator.timer

# View logs
journalctl -u m3u-generator -n 50 --no-pager

# Check health
/opt/m3u-generator/scripts/health-check.sh

# Restart service
systemctl restart m3u-generator.service

# Reinstall with new config
M3U_GENERATOR_WEBDAV_URL="..." \
M3U_GENERATOR_PLAYLIST_DIR="..." \
./scripts/install.sh
```

---

*Deployment Guide Version: 1.0*

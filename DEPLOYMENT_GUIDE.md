# M3U Generator Deployment Guide

This guide consolidates the official INSTALLATION_GUIDE.md with deployment-specific findings from media.tailor-shop.

## Table of Contents
1. [Project Overview](#project-overview)
2. [Prerequisites](#prerequisites)
3. [Deployment Steps](#deployment-steps)
4. [Configuration Variables](#configuration-variables)
5. [Systemd Integration](#systemd-integration)
6. [Troubleshooting](#troubleshooting)

---

## Project Overview

**M3U Generator** is a C++20 application that:
- Takes series of directories and generates playlists from each
- Generates a total playlist from all specified directories
- Each filename is URL encoded
- A specified path is prefixed onto each filename in the final output
- Includes systemd unit placeholders for automated scheduling

### Dependencies
- C++ 20 compiler
- libcurl

---

## Prerequisites

### System Requirements
- Debian-based system (tested on RockPi devices)
- SSH access as user `garak`
- Read/write access to `/etc/systemd/system/` and project directory
- WebDAV server access for playlist distribution

### Project Location Verification
**CRITICAL:** The installation script has a hardcoded path issue that must be addressed:

```bash
# Check current project location
PROJECT_PATH="/home/garak/projects/m3u-generator"  # media.tailor-shop

# The install.sh expects PROJECT_DIR="/project" which does NOT exist
# You MUST either:
#   Option A: Move project to /project (not recommended)
#   Option B: Modify install.sh to use correct path (recommended)
```

### Environment Variables Required

| Variable | Description | Example |
|----------|-------------|---------|
| `M3U_GENERATOR_WEBDAV_URL` | WebDAV server URL for playlist output | `http://media.tailor-shop:2222/Music/` |
| `M3U_GENERATOR_PLAYLIST_DIR` | Directory where playlists are stored | `/NAS/storage/Media/Music/playlists` |
| `M3U_GENERATOR_OUTPUT_FILE` | Output playlist filename | `/var/www/html/playlist.m3u` |
| `M3U_GENERATOR_LOG_LEVEL` | Logging verbosity (debug/info/warn/error) | `info` |

---

## Deployment Steps

### Step 1: Environment Setup

```bash
# Set required environment variables
export M3U_GENERATOR_WEBDAV_URL="http://media.tailor-shop:2222/Music/"
export M3U_GENERATOR_PLAYLIST_DIR="/NAS/storage/Media/Music/playlists"
export M3U_GENERATOR_OUTPUT_FILE="/var/www/html/playlist.m3u"
export M3U_GENERATOR_LOG_LEVEL="info"

# Verify project location (fix hardcoded path issue)
cd /home/garak/projects/m3u-generator
```

### Step 2: Fix Hardcoded Path in install.sh

**Before running install.sh**, modify the PROJECT_DIR variable:

```bash
# Edit install.sh to use correct project path
sed -i 's|PROJECT_DIR="/project"|PROJECT_DIR="'$PWD'"|' scripts/install.sh

# Verify the change
grep "PROJECT_DIR=" scripts/install.sh
```

### Step 3: Run Installation Script

```bash
# Execute installation with environment variables
cd /home/garak/projects/m3u-generator
./scripts/install.sh
```

### Step 4: Review Systemd Units

The installation will create/overwrite these files:

| Unit | Location | Purpose |
|------|----------|---------|
| `m3u-generator.service` | `/etc/systemd/system/` | Main service unit |
| `m3u-generator.timer` | `/etc/systemd/system/` | Triggers every 10 minutes |

**Verify existing units:**
```bash
# Check current systemd units
ls -la /etc/systemd/system/m3u-generator.*

# View unit contents
cat /etc/systemd/system/m3u-generator.service
cat /etc/systemd/system/m3u-generator.timer
```

### Step 5: Enable and Start Services

```bash
# Reload systemd to pick up new units
systemctl daemon-reload

# Enable timer (runs every 10 minutes)
systemctl enable m3u-generator.timer

# Start timer
systemctl start m3u-generator.timer

# Verify status
systemctl status m3u-generator.service
systemctl list-timers | grep m3u-generator
```

### Step 6: Run Health Check

```bash
./scripts/health-check.sh
```

---

## Configuration Files

After installation, review and edit these configuration files:

### `/etc/m3u-generator/generator.conf`
Contains all runtime configuration. Edit as needed before restarting services.

### `/etc/logrotate.d/m3u-generator.conf`
Log rotation configuration (daily rotation, 14 days retention).

---

## Systemd Integration Details

### Service Unit (`m3u-generator.service`)
- **WorkingDirectory:** `/NAS/storage/Media/Music`
- **ExecUser:** `garak`
- **Output URL:** `http://media.tailor-shop:2222/Music/`

### Timer Unit (`m3u-generator.timer`)
- **Trigger:** Every 10 minutes (`OnUnitActiveSec=10min`)
- **Persistent:** Yes (continues after system reboot)

---

## Troubleshooting

### Common Issues

#### Issue: "cat: /etc/systemd/system/m3u-generator.service: No such file or directory"
**Cause:** Hardcoded PROJECT_DIR path mismatch
**Fix:** Modify install.sh to use correct project path before running (Step 2 above)

#### Issue: Service won't start after installation
**Check:**
```bash
journalctl -u m3u-generator.service -xe
systemctl status m3u-generator.service
```

#### Issue: Timer not triggering
**Check:**
```bash
systemctl list-timers | grep m3u-generator
systemctl status m3u-generator.timer
```

### Manual Debug Steps

1. **Rebuild from source if needed:**
   ```bash
   cd /home/garak/projects/m3u-generator/build
   cmake ..
   make
   sudo make install
   ```

2. **Verify binary installation:**
   ```bash
   which m3u-generator
   m3u-generator --version
   ```

3. **Check configuration file:**
   ```bash
   cat /etc/m3u-generator/generator.conf
   ```

---

## Post-Installation Verification Checklist

- [ ] Environment variables are set correctly
- [ ] install.sh PROJECT_DIR path has been fixed
- [ ] Dependencies installed successfully
- [ ] Binary installed at `/usr/bin/m3u-generator` (or configured prefix)
- [ ] Systemd units exist and are valid
- [ ] Timer enabled and active
- [ ] Health check passes
- [ ] Log rotation configured

---

## Maintenance

### Daily Operations
- Monitor logs: `journalctl -u m3u-generator.service -f`
- Check timer status: `systemctl list-timers | grep m3u-generator`

### Periodic Tasks
- Review generated playlists in output directory
- Monitor log rotation (config at `/etc/logrotate.d/m3u-generator.conf`)
- Verify WebDAV connectivity

### Updating the Project
```bash
# Pull latest changes (if using git)
git pull

# Rebuild and reinstall
./scripts/install.sh
systemctl daemon-reload
systemctl restart m3u-generator.service
```

---

## Security Considerations

- Ensure WebDAV URL uses appropriate authentication
- Verify file permissions on `/NAS/storage/Media/Music`
- Monitor log files for unauthorized access attempts
- Keep system and dependencies updated

---

*Generated: 2025-01-16*
*Platform: media.tailor-shop*
*User: garak*

#!/bin/bash
# Health check script for M3U Generator service
# Checks if the service is running and responding correctly

set -e

SERVICE_NAME="m3u-generator"
LOG_FILE="/var/log/m3u-generator/m3u-generator.log"

echo "[$(date '+%Y-%m-%d %H:%M:%S')] Health Check Started" >> "$LOG_FILE"

# Check if systemd service is active
if systemctl is-active --quiet "$SERVICE_NAME" 2>/dev/null; then
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] Service Status: ACTIVE" >> "$LOG_FILE"
    
    # Check recent log entries for errors
    if [ -f "$LOG_FILE" ]; then
        ERROR_COUNT=$(grep -c "ERROR\|FATAL" "$LOG_FILE" 2>/dev/null || echo "0")
        if [ "$ERROR_COUNT" -gt 10 ]; then
            echo "[$(date '+%Y-%m-%d %H:%M:%S')] WARNING: High error count ($ERROR_COUNT) in logs" >> "$LOG_FILE"
            exit 1
        fi
    fi
    
    # Check process is actually running
    if pgrep -x "$(basename $SERVICE_NAME)" > /dev/null 2>&1; then
        echo "[$(date '+%Y-%m-%d %H:%M:%S')] Process Status: RUNNING (PID: $(pgrep -x $(basename $SERVICE_NAME)))" >> "$LOG_FILE"
        echo "HEALTHY: Service is running normally"
        exit 0
    else
        echo "[$(date '+%Y-%m-%d %H:%M:%S')] WARNING: Service unit active but process not found" >> "$LOG_FILE"
        echo "UNHEALTHY: Service unit is active but process not running"
        exit 1
    fi
else
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] Service Status: INACTIVE" >> "$LOG_FILE"
    echo "UNHEALTHY: Service is not running"
    exit 1
fi

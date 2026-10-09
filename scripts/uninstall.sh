#!/bin/bash
set -euo pipefail

# Configuration variables with defaults
INSTALL_PREFIX="${M3U_GENERATOR_INSTALL_PREFIX:-/usr/local}"
CONFIG_DIR="/etc/m3u-generator"
LOG_DIR="/var/log/m3u-generator"
SYSTEMD_DIR="/etc/systemd/system"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log() {
    echo -e "[$(date '+%Y-%m-%d %H:%M:%S')] $1"
}

log_info() { log "${GREEN}[INFO]${NC} $1"; }
log_warn() { log "${YELLOW}[WARN]${NC} $1"; }
log_error() { log "${RED}[ERROR]${NC} $1"; }

remove_service() {
    log_info "Removing systemd service and timer..."
    
    # Stop and disable the service
    systemctl stop m3u-generator 2>/dev/null || true
    systemctl disable m3u-generator 2>/dev/null || true
    
    # Remove unit files
    rm -f \
        "$SYSTEMD_DIR/m3u-generator.service" \
        "$SYSTEMD_DIR/m3u-generator.timer" \
        "/etc/systemd/system/multi-user.target.wants/m3u-generator.service" \
        "/etc/systemd/system/timers.target.wants/m3u-generator.timer" 2>/dev/null || true
    
    # Reload systemd daemon
    systemctl daemon-reload 2>/dev/null || log_warn "Failed to reload systemd daemon"
}

remove_binaries() {
    log_info "Removing installed binaries..."
    
    rm -f \
        "$INSTALL_PREFIX/bin/m3u-generator" \
        "$INSTALL_PREFIX/lib/libm3u-generator*" \
        2>/dev/null || true
    
    # Remove from package manager cache if installed via apt
    dpkg -l | grep -q m3u-generator && apt-get remove --purge m3u-generator 2>/dev/null || true
}

remove_config() {
    log_info "Removing configuration files..."
    
    rm -rf \
        "$CONFIG_DIR" \
        "/etc/default/m3u-generator" \
        2>/dev/null || true
    
    # Remove any generated config from install script
    find /var/www/html -name "*m3u*" -type f -delete 2>/dev/null || true
}

cleanup_logs() {
    log_info "Cleaning up log files..."
    
    rm -f \
        "$LOG_DIR/m3u-generator.log" \
        "$LOG_DIR/errors.log" \
        2>/dev/null || true
}

main() {
    log_info "Starting m3u-generator uninstallation..."
    
    # Read confirmation
    echo ""
    read -p "This will remove m3u-generator and all related files. Continue? [y/N]: " confirm
    
    case "$confirm" in
        [Yy]*) ;;
        *) log_warn "Uninstallation cancelled."
            return 0 ;;
    esac
    
    remove_service || true
    remove_binaries || true
    remove_config || true
    cleanup_logs || true
    
    log_info "Uninstallation complete!"
    echo -e "${GREEN}m3u-generator removed successfully!${NC}"
}

main "$@"

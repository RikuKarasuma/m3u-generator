#!/bin/bash
set -euo pipefail

# Configuration variables with defaults
INSTALL_PREFIX="${M3U_GENERATOR_INSTALL_PREFIX:-/usr/local}"
CONFIG_DIR="/etc/m3u-generator"
LOG_DIR="/var/log/m3u-generator"
SYSTEMD_DIR="/etc/systemd/system"
PROJECT_DIR="/project"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

log() {
    echo -e "[$(date '+%Y-%m-%d %H:%M:%S')] $1"
}

log_info() { log "${GREEN}[INFO]${NC} $1"; }
log_warn() { log "${YELLOW}[WARN]${NC} $1"; }
log_error() { log "${RED}[ERROR]${NC} $1"; }

# Parse and validate arguments from environment variables
WEBDAV_URL="${M3U_GENERATOR_WEBDAV_URL:-}"
PLAYLIST_DIR="${M3U_GENERATOR_PLAYLIST_DIR:-}"
OUTPUT_FILE="${M3U_GENERATOR_OUTPUT_FILE:-/var/www/html/playlist.m3u}"
LOG_LEVEL="${M3U_GENERATOR_LOG_LEVEL:-info}"

# Validate required arguments
if [[ -z "$WEBDAV_URL" || -z "$PLAYLIST_DIR" ]]; then
    log_error "Missing required arguments:"
    log_error "  M3U_GENERATOR_WEBDAV_URL=<webdav-url>"
    log_error "  M3U_GENERATOR_PLAYLIST_DIR=<playlist-directory>"
    echo ""
    echo "Usage: install.sh [OPTIONS]"
    echo ""
    echo "Required environment variables:"
    echo "  M3U_GENERATOR_WEBDAV_URL=<webdav-url>         e.g., http://webdav.example.com/dav"
    echo "  M3U_GENERATOR_PLAYLIST_DIR=<playlist-dir>     e.g., /var/lib/m3u-generator/playlists"
    echo ""
    echo "Optional environment variables:"
    echo "  M3U_GENERATOR_OUTPUT_FILE=<output-file>       e.g., /var/www/html/playlist.m3u"
    echo "  M3U_GENERATOR_LOG_LEVEL=<level>               e.g., debug|info|warn|error (default: info)"
    echo ""
    usage_manual_string
    exit 1
fi

install_dependencies() {
    log_info "Installing dependencies..."
    apt-get update -qq
    apt-get install -y \
        libcurl4-openssl-dev \
        cmake \
        build-essential \
        wget \
        curl \
        gnupg \
        ca-certificates || true
}

setup_directories() {
    log_info "Setting up directories..."
    
    # Create directories with proper permissions
    mkdir -p "$CONFIG_DIR"
    chmod 750 "$CONFIG_DIR"
    
    mkdir -p "$LOG_DIR"
    chmod 750 "$LOG_DIR"
    
    mkdir -p "$PLAYLIST_DIR"
    chmod 750 "$PLAYLIST_DIR"
    
    # Write config file with provided arguments (merge with defaults)
    cat << CONFIGEOF > "$CONFIG_DIR/generator.conf"
# m3u-generator configuration
webdav_url=$WEBDAV_URL
playlist_dir=$PLAYLIST_DIR
output_file=$OUTPUT_FILE
log_level=$LOG_LEVEL
CONFIGEOF
    
    log_info "Configuration written to $CONFIG_DIR/generator.conf"
}

build_project() {
    log_info "Building project..."
    cd "$PROJECT_DIR"
    
    # Create build directory if it doesn't exist
    mkdir -p build
    cd build
    
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
        -DM3U_GENERATOR_WEBDAV_URL="$WEBDAV_URL" \
        -DM3U_GENERATOR_PLAYLIST_DIR="$PLAYLIST_DIR" \
        -DM3U_GENERATOR_OUTPUT_FILE="$OUTPUT_FILE" \
        -DM3U_GENERATOR_LOG_LEVEL="$LOG_LEVEL"
    
    make -j$(nproc)
    make install
    
    log_info "Binary installed to $INSTALL_PREFIX/bin/m3u-generator"
}

setup_systemd() {
    log_info "Setting up systemd units..."
    
    # Copy service unit
    cp "$PROJECT_DIR/m3u-generator.service" "$SYSTEMD_DIR/"
    chmod 644 "$SYSTEMD_DIR/m3u-generator.service"
    
    # Copy timer unit
    cp "$PROJECT_DIR/m3u-generator.timer" "$SYSTEMD_DIR/"
    chmod 644 "$SYSTEMD_DIR/m3u-generator.timer"
    
    # Reload systemd and enable timer
    log_info "Reloading systemd daemon and enabling timer..."
    systemctl daemon-reload
    
    if ! systemctl is-systemd --quiet 2>/dev/null; then
        log_warn "This is not a systemd system. Timer enablement skipped."
        return 0
    fi
    
    systemctl enable --now m3u-generator.timer || log_warn "Timer enablement failed, continuing anyway"
}

setup_logs() {
    log_info "Setting up log rotation..."
    
    # Create logrotate config directory if needed
    mkdir -p "$(dirname /etc/logrotate.d/m3u-generator.conf)"
    
    cat << LOGEOF > /etc/logrotate.d/m3u-generator.conf
/var/log/m3u-generator/*.log {
    daily
    rotate 14
    compress
    delaycompress
    missingok
    notifempty
    create 0640 root adm
}
LOGEOF
    
    log_info "Log rotation configured"
}

setup_webdav() {
    log_info "Setting up WebDAV access..."
    
    # Create directory for web-accessible playlists if needed
    mkdir -p /var/www/html/playlist.m3u 2>/dev/null || true
}

main() {
    log_info "Starting m3u-generator installation..."
    
    # Display configuration summary
    echo ""
    log_info "Configuration:"
    echo "  WebDAV URL: $WEBDAV_URL"
    echo "  Playlist Directory: $PLAYLIST_DIR"
    echo "  Output File: $OUTPUT_FILE"
    echo "  Log Level: $LOG_LEVEL"
    echo ""
    
    # Install dependencies
    install_dependencies || log_warn "Some dependencies may already be installed"
    
    # Setup directories (writes config file with arguments)
    setup_directories
    
    # Build the project
    build_project || (log_error "Build failed, aborting installation"; exit 1)
    
    # Setup systemd
    setup_systemd
    
    # Setup logs and webdav
    setup_logs
    setup_webdav
    
    log_info "Installation complete!"
    echo ""
    echo -e "${GREEN}m3u-generator installed successfully!${NC}"
    echo ""
    echo "Configuration written to: $CONFIG_DIR/generator.conf"
    echo ""
    echo "The timer service has been enabled and started."
    echo ""
    echo "Next steps:"
    echo "  1. Review and edit $CONFIG_DIR/generator.conf if needed"
    echo "  2. Check status with 'systemctl status m3u-generator'"
    echo "  3. Run 'health-check.sh' for a quick health check"
}

main "$@"

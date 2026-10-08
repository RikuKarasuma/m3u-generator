#!/bin/bash
# Deployment wrapper script for m3u-generator on RockPi
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
SYSTEMD_DIR="$PROJECT_ROOT/systemd"
INSTALL_PREFIX="${1:-/usr/local}"

echo "=== M3U Generator Deployment Script ==="
echo "Project root: $PROJECT_ROOT"
echo "Install prefix: $INSTALL_PREFIX"

# Build with CMake
echo "[1/4] Building m3u-generator..."
cd "$PROJECT_ROOT"
cmake -B build \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DCMAKE_BUILD_TYPE="Release"

cmake --build build -j$(nproc)

# Install to system
echo "[2/4] Installing to $INSTALL_PREFIX..."
make -C build install

# Copy systemd units (requires root or appropriate permissions)
echo "[3/4] Setting up systemd units..."
if [[ "$EUID" -eq 0 ]]; then
    cp "$SYSTEMD_DIR/m3u-generator.service" /etc/systemd/system/
    cp "$SYSTEMD_DIR/m3u-generator.timer" /etc/systemd/system/
    systemctl daemon-reload
    echo "✓ Systemd units installed to /etc/systemd/system/"
else
    echo "⚠ Skipping systemd installation (not root)"
    echo "   Copy files manually:"
    echo "   cp $SYSTEMD_DIR/m3u-generator.service /etc/systemd/system/"
    echo "   cp $SYSTEMD_DIR/m3u-generator.timer /etc/systemd/system/"
fi

# Create wrapper for manual execution
echo "[4/4] Creating manual run wrapper..."
cat << 'WRAPPER' > /usr/local/bin/run-m3u-generator
#!/bin/bash
/usr/local/bin/m3u-generator "$@"
WRAPPER
chmod +x /usr/local/bin/run-m3u-generator 2>/dev/null || true

echo "=== Deployment complete! ==="
echo ""
echo "To enable automatic execution:"
echo "  systemctl enable m3u-generator.timer"
echo "  systemctl start m3u-generator.timer"
echo ""
echo "To run manually:"
echo "  run-m3u-generator --output /var/www/html/playlists.m3u --source-dir /data/videos"

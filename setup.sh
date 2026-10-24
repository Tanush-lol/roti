#!/usr/bin/env bash
# setup.sh - one-time dev environment setup for the w11 Wayland project.
#
# 1. verifies a Wayland environment + required tools
# 2. locates xdg-shell.xml
# 3. runs wayland-scanner to generate the header + glue code into include/
# 4. compiles the glue code (as C) into src/xdg-shell-protocol.o
#
# Usage:  ./setup.sh
#         SKIP_SESSION_CHECK=1 ./setup.sh   # e.g. CI / SSH where no compositor runs

set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
INCLUDE_DIR="$ROOT/include"
SRC_DIR="$ROOT/src"

PROTOCOL_C="$INCLUDE_DIR/xdg-shell-protocol.c"
PROTOCOL_H="$INCLUDE_DIR/xdg-shell-client-protocol.h"
PROTOCOL_O="$SRC_DIR/xdg-shell-protocol.o"

info() { printf '\033[1;34m[setup]\033[0m %s\n' "$*"; }
ok()   { printf '\033[1;32m[ ok ]\033[0m %s\n' "$*"; }
die()  { printf '\033[1;31m[fail]\033[0m %s\n' "$*" >&2; exit 1; }

# ---------------------------------------------------------------- 1. verify
info "Checking for a Wayland environment..."

if [[ "${SKIP_SESSION_CHECK:-0}" != "1" ]]; then
    if [[ "${XDG_SESSION_TYPE:-}" != "wayland" && -z "${WAYLAND_DISPLAY:-}" ]]; then
        die "Not running in a Wayland session (XDG_SESSION_TYPE='${XDG_SESSION_TYPE:-unset}', WAYLAND_DISPLAY unset).
       Log in to a Wayland session, or re-run with SKIP_SESSION_CHECK=1 to only generate files."
    fi
    ok "Wayland session detected (WAYLAND_DISPLAY=${WAYLAND_DISPLAY:-unset})"
else
    info "Skipping session check (SKIP_SESSION_CHECK=1)"
fi

for tool in wayland-scanner gcc find; do
    command -v "$tool" >/dev/null 2>&1 \
        || die "'$tool' not found. Install it (Arch: wayland, base-devel | Debian/Ubuntu: libwayland-dev, build-essential | Fedora: wayland-devel, gcc)."
done

if command -v pkg-config >/dev/null 2>&1; then
    pkg-config --exists wayland-client \
        || die "wayland-client development files not found (needed for -lwayland-client)."
fi
ok "Required tools present"

# ---------------------------------------------------------------- 2. find xml
info "Locating xdg-shell.xml..."

SEARCH_DIRS=()
IFS=':' read -r -a XDG_DIRS <<< "${XDG_DATA_DIRS:-/usr/local/share:/usr/share}"
for d in "${XDG_DIRS[@]}" /usr/share /usr/local/share; do
    [[ -d "$d" ]] && SEARCH_DIRS+=("$d")
done

XML_PATH="$(find "${SEARCH_DIRS[@]}" -type f -path '*/stable/xdg-shell/xdg-shell.xml' -print -quit 2>/dev/null || true)"

if [[ -z "$XML_PATH" ]]; then
    die "xdg-shell.xml not found. Install the wayland-protocols package
       (Arch: wayland-protocols | Debian/Ubuntu: wayland-protocols | Fedora: wayland-protocols-devel)."
fi
ok "Found $XML_PATH"

# ---------------------------------------------------------------- 3. generate
info "Generating protocol files into include/ ..."
mkdir -p "$INCLUDE_DIR" "$SRC_DIR"

wayland-scanner client-header "$XML_PATH" "$PROTOCOL_H"
wayland-scanner private-code  "$XML_PATH" "$PROTOCOL_C"
ok "Wrote $(basename "$PROTOCOL_H") and $(basename "$PROTOCOL_C")"

# ---------------------------------------------------------------- 4. compile
# Must be compiled as C (gcc), not C++ (g++), or the interface symbols
# get internal linkage and the link step fails with undefined references.
info "Compiling glue code to src/xdg-shell-protocol.o ..."
gcc -I"$INCLUDE_DIR" -c "$PROTOCOL_C" -o "$PROTOCOL_O"

if command -v nm >/dev/null 2>&1; then
    nm "$PROTOCOL_O" | grep -q ' [DR] xdg_wm_base_interface$' \
        || die "xdg_wm_base_interface is not exported from $PROTOCOL_O"
fi
ok "Built $PROTOCOL_O"

# ---------------------------------------------------------------- done
cat <<EOF

Setup complete. Build the project with:

  g++ -Iinclude src/main.cpp src/buffer.cpp src/shm.cpp src/xdg_surface.cpp \\
      src/xdg-shell-protocol.o -o app -lwayland-client
EOF

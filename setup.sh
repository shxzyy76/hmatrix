#!/bin/sh
# Builds hmatrix and installs its default config to ~/.config/hmatrix/hmatrix.toml.
#
# Usage: ./setup.sh [--force]
#   --force   overwrite an existing ~/.config/hmatrix/hmatrix.toml
set -eu

root="$(cd "$(dirname "$0")" && pwd)"
src="$root/config/hmatrix.toml"
dir="${HOME:?HOME is not set}/.config/hmatrix"
dest="$dir/hmatrix.toml"

case "${1:-}" in
    ""|--force) ;;
    *) echo "Usage: $0 [--force]" >&2; exit 2 ;;
esac

[ -f "$src" ] || { echo "error: $src not found" >&2; exit 1; }
command -v cmake >/dev/null 2>&1 || {
    echo "error: cmake not found. Install the build tools first (see README, Installation)." >&2
    exit 1
}

echo "==> Building hmatrix"
cmake -S "$root" -B "$root/build"
cmake --build "$root/build" --parallel

echo "==> Setting up config"
mkdir -p "$dir"
if [ -e "$dest" ] && [ "${1:-}" != "--force" ]; then
    echo "Keeping existing config: $dest (use --force to overwrite)"
else
    cp "$src" "$dest"
    echo "Config installed: $dest"
fi

echo "==> Done. Run: $root/build/hmatrix"

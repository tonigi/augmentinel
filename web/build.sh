#!/usr/bin/env bash
set -euo pipefail
repo_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
if ! command -v emcmake >/dev/null && [[ -f /workspace/.tools/activate-web.sh ]]; then
    source /workspace/.tools/activate-web.sh
fi
emcmake cmake -S "$repo_dir" -B "$repo_dir/build-web" -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build "$repo_dir/build-web" --parallel "${BUILD_JOBS:-4}"

#!/usr/bin/env bash
set -euo pipefail

# Build a minimal WebAssembly version of Augmentinel using Emscripten.
# The script assumes the Emscripten SDK lives in ./emsdk. If the em++
# compiler isn't on PATH we source the SDK environment.

ROOT_DIR=$(cd "$(dirname "$0")" && pwd)

if ! command -v em++ >/dev/null 2>&1; then
  if [[ -f "$ROOT_DIR/emsdk/emsdk_env.sh" ]]; then
    source "$ROOT_DIR/emsdk/emsdk_env.sh" > /dev/null
  else
    echo "Emscripten SDK not found. Install it into $ROOT_DIR/emsdk" >&2
    exit 1
  fi
fi

em++ \
  src/emscripten_main.cpp \
  src/game/GameState.cpp \
  src/render/Renderer.cpp \
  -std=c++17 -O2 -sWASM=1 \
  -o augmentinel.html


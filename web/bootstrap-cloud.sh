#!/usr/bin/env bash
# Unprivileged Debian cloud installation. A normal emsdk installation also works.
set -euo pipefail
repo_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
tools_dir=/workspace/.tools
if [[ ! -x "$tools_dir/sysroot/usr/share/emscripten/emcc" ]] || { ! command -v ffmpeg >/dev/null && [[ ! -x "$tools_dir/sysroot/usr/bin/ffmpeg" ]]; }; then
    mkdir -p "$tools_dir/apt/lists/partial" "$tools_dir/apt/archives/partial"
    cat > "$tools_dir/apt/sources.list" <<'SOURCES'
deb [signed-by=/usr/share/keyrings/debian-archive-keyring.gpg] https://deb.debian.org/debian trixie main
SOURCES
    apt_opts=(-o "Dir::Etc::sourcelist=$tools_dir/apt/sources.list"
        -o Dir::Etc::sourceparts=- -o "Dir::State::lists=$tools_dir/apt/lists"
        -o "Dir::Cache::archives=$tools_dir/apt/archives"
        -o "Dir::State::extended_states=$tools_dir/apt/extended_states"
        -o Debug::NoLocking=true -o Acquire::Retries=0)
    apt-get "${apt_opts[@]}" update
    apt-get "${apt_opts[@]}" --download-only -y install emscripten=3.1.69+dfsg-3 cmake ninja-build ffmpeg
    mkdir -p "$tools_dir/sysroot"
    for package in "$tools_dir"/apt/archives/*.deb; do dpkg-deb -x "$package" "$tools_dir/sysroot"; done
fi
node_bin=$(command -v node)
if [[ -x "$tools_dir/sysroot/usr/bin/ffmpeg" ]]; then
    mkdir -p "$tools_dir/bin"
    ln -sf "$tools_dir/sysroot/usr/bin/ffmpeg" "$tools_dir/bin/ffmpeg"
fi
mkdir -p "$tools_dir/bin"
ln -sf "$tools_dir/sysroot/usr/bin/cmake" "$tools_dir/bin/cmake"
ln -sf "$tools_dir/sysroot/usr/bin/ninja" "$tools_dir/bin/ninja"
cat > "$tools_dir/emscripten-config.py" <<CONFIG
LLVM_ROOT = '$tools_dir/sysroot/usr/bin'
BINARYEN_ROOT = '$tools_dir/sysroot/usr'
NODE_JS = '$node_bin'
CACHE = '$tools_dir/sysroot/usr/share/emscripten/cache'
FROZEN_CACHE = False
LLVM_ADD_VERSION = '19'
CLANG_ADD_VERSION = '19'
CONFIG
cat > "$tools_dir/activate-web.sh" <<'ACTIVATE'
export PATH="/workspace/.tools/sysroot/usr/share/emscripten:/workspace/.tools/bin:$PATH"
export LD_LIBRARY_PATH="/workspace/.tools/sysroot/usr/lib/x86_64-linux-gnu${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
export EM_CONFIG=/workspace/.tools/emscripten-config.py
ACTIVATE
source "$tools_dir/activate-web.sh"
emcc --version
"$repo_dir/web/build.sh"

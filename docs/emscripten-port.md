# Emscripten Port Plan

This document outlines the initial steps required to port **Augmentinel** to run in a web browser using [Emscripten](https://emscripten.org/).

## Goals

* Compile the core game logic to WebAssembly.
* Replace platform specific APIs (Win32, D3D11, XAudio) with web friendly alternatives.
* Provide an entry point and build scripts for Emscripten builds.

## Initial Steps

1. **Ignore local SDK files.**
   The Emscripten SDK is large and should not be committed. Update `.gitignore` to exclude the `emsdk/` directory.
2. **Add a web entry point.**
   Introduce a minimal `main` function guarded by `__EMSCRIPTEN__` so the project can produce a WebAssembly target without touching the existing Windows specific code paths.
3. **Isolate game logic and rendering.**
   Begin separating platform neutral game logic and 3D rendering into `src/game` and `src/render` modules. The Emscripten `main` now drives these stubs via `emscripten_set_main_loop`.
4. **Stub platform headers.**
   When compiling with Emscripten the Windows headers are unavailable. Create minimal type aliases so the code can be compiled without the Win32 headers.
5. **Add a web build script.**
   A `build_web.sh` helper script compiles the stub modules into `augmentinel.html`, `augmentinel.js` and `augmentinel.wasm` using `em++`.

## Next Steps

* Replace the Win32 windowing and message loop with an event loop driven by `emscripten_set_main_loop`.
* Migrate rendering from D3D11 to WebGL via OpenGL ES 3.0.
* Replace XAudio2 based audio with the Emscripten WebAudio API.
* Ensure required assets are preloaded using the Emscripten virtual file system.

These steps lay the groundwork for a full browser port while keeping the existing Windows build intact.

# Augmentinel for the browser

This target builds the existing Sentinel emulation and Augmentinel game logic as
WebAssembly. WebGL 2 replaces Direct3D 11, browser events replace Win32 input,
Web Audio replaces XAudio2, and localStorage retains settings and landscape codes.
The Windows Visual Studio solution remains available.

## Build and run

Requirements: Emscripten 3.1.69 or newer, CMake 3.20+, Ninja, FFmpeg, Python 3,
and Git. Activate your Emscripten SDK before building. The initial CMake configure
fetches a pinned DirectXMath revision and a checksum-verified SAL compatibility
header; subsequent builds reuse the dependencies.

```sh
./web/build.sh
python3 -m http.server 8000 --directory build-web
```

Open the server in a browser with WebGL 2 enabled. Serve the directory over HTTP
or HTTPS rather than opening the HTML as a file. The output is a static app:
`index.html`, `index.js`, `index.wasm`, and `index.data` must be deployed together.
Configure the server to send `.wasm` with `application/wasm`. No backend, API keys,
pthreads, or cross-origin isolation headers are required. Build outputs remain
inside ignored `build-web/`.

For the Debian Codex cloud machine, `./web/bootstrap-cloud.sh` installs the tools
under `/workspace/.tools` without root access, then builds the application.
Future shells can run `source /workspace/.tools/activate-web.sh`. Tool downloads
use APT's signed repository metadata and package checksums. Use the existing
checkout; cloud tasks are already isolated and need no additional Git worktree.

FFmpeg converts the bundled Amiga sound pack and music from Microsoft ADPCM to
PCM WAV under `build-web/assets/` before asset packaging. Original files are
unchanged. Other sound packs are not included in the browser bundle. Use
`BUILD_JOBS=2 ./web/build.sh` to reduce build concurrency.

## Controls

Press Enter at the title, then Enter again to play the selected landscape.
Click the canvas or **Play / capture mouse** to enable mouse look and audio.
Escape releases the mouse; in the game it also returns to landscape selection.

| Input | Action |
| --- | --- |
| Arrow keys | Select landscape / turn / look up and down |
| A / left mouse | Absorb |
| T | Create tree |
| B / right mouse | Create boulder |
| R / middle mouse | Create robot |
| Q | Transfer to robot |
| H | Hyperspace |
| U | Turn around |
| P | Pause / resume |
| M / N | Toggle music / tunes |
| + / − | Music volume |

Sound starts after a keyboard or mouse gesture, as required by browser autoplay
rules. Settings and unlocked landscape codes are stored per origin in
`localStorage` under `augmentinel.settings`. If storage is unavailable, gameplay
continues without persistent saves. The simulation stops advancing while the
tab is hidden.

## Browser smoke test

```sh
npm ci --prefix web
# If Chromium is not installed: npx --prefix web playwright install chromium
npm test --prefix web
```

The test starts and closes its own local server. It uses system Chromium at
`/usr/bin/chromium` when available, `CHROMIUM_PATH` when specified, or Playwright's
Chromium otherwise. Software WebGL is enabled for headless validation. It checks
real rendered pixels, title/landscape/game transitions, short key taps, turning,
pause/resume, pointer-lock mouse look, audio decoding and activation, resizing,
and persisted preferences after reload. Screenshots are saved in ignored
`web/test-results/`. These checks do not establish audio quality or complete all
57,344 landscapes.

## Scope

This is a desktop flat-screen browser port. OpenVR, Windows options dialogs,
DLL-based HRTF, and alternative audio packs remain Windows-only. The browser
uses Web Audio's HRTF panner for positioned effects. Mobile/touch controls and
WebXR are not implemented. Windows compilation still requires the original
Visual Studio/Windows SDK toolchain and has not been tested on this Linux host.

DirectXMath is MIT-licensed; its license is available with the fetched source.
SAL comes from the MIT-licensed .NET runtime. The existing GPLv3 project license
and bundled asset notices continue to apply. Include source and applicable
license notices when distributing a build.

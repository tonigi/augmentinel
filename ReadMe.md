# Augmentinel

By Simon Owen (simon@simonowen.com)

---

## Introduction

Augmentinel is re-skinned version of the Geoff Crammond classic: [The
Sentinel](https://en.wikipedia.org/wiki/The_Sentinel_(video_game)) (aka The
Sentry).

It emulates the Spectrum version of the game for the original gameplay, then
adds the best features from other ports, plus a few modern extras.

For more details see: https://simonowen.com/spectrum/augmentinel/

## Features

- Accelerated 3D rendering with mouse free look.
- VR support for OpenVR-compatible headsets.
- Palette and landscape colours from PC version.
- BBC/C64/Spectrum/Amiga tunes and HRTF spatial sound effects.
- Background music from Amiga version.
- Sky view from PC/ST/Amiga versions.
- Unlocked the hex landscapes for a total of 57344.
- Pixel-perfect object selection.
- All remaining game logic runs as normal.

## Building

The Windows version requires Visual Studio 2019 or later and uses Win32 and
Direct3D 11.

A flat-screen browser version builds with Emscripten/WebAssembly and WebGL 2.
See [web/README.md](web/README.md) for prerequisites, build commands, controls,
and browser tests. The browser target reuses the original game logic and CPU
emulation; VR remains available in the Windows version.

## License

The Augmentinel source code is licensed under the [GNU GPL v3.0
license](https://www.gnu.org/licenses/gpl-3.0.html).

Z80 CPU emulation by [Manuel Sainz de Baranda y Goñi](https://github.com/redcode/Z80),
licensed under GNU GPL v3.

X3DAudio HRTF support by [Roman Kosmos](https://github.com/kosumosu/x3daudio1_7_hrtf),
licensed under GNU GPL v3.



## Disclaimer

This is an unofficial fan creation, distributed without charge. I have no
affiliation with the original developer, publisher, or other rights holders.

## Contact

Simon Owen  
[https://simonowen.com](https://simonowen.com)

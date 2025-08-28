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

Building currently requires Visual Studio 2019 or later.

The current code uses the Win32 and D3D11 APIs so it's not yet portable to
non-Windows platforms. Experimental work has begun on an
[Emscripten](https://emscripten.org/) port to run the game in a web browser,
with early `src/game` and `src/render` modules providing platform-neutral
stubs. See `docs/emscripten-port.md` for the current roadmap.
`build_web.sh` invokes `em++` to compile these stubs into `augmentinel.html`
for experimentation.

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

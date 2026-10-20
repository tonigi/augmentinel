#pragma once
#include <algorithm>
#include <array>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>
#include <limits>
#include <locale>
#include <codecvt>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <emscripten.h>
#include <GLES3/gl3.h>
namespace fs = std::filesystem;
using namespace DirectX;
using UINT32 = uint32_t;
using DWORD = uint32_t;
// Shared bindings keep Windows virtual-key numbers; browser events are mapped to these.
constexpr int VK_LBUTTON=1, VK_RBUTTON=2, VK_MBUTTON=4, VK_XBUTTON1=5;
constexpr int VK_RETURN=13, VK_PAUSE=19, VK_ESCAPE=27, VK_SPACE=32;
constexpr int VK_PRIOR=33, VK_NEXT=34, VK_END=35, VK_HOME=36;
constexpr int VK_LEFT=37, VK_UP=38, VK_RIGHT=39, VK_DOWN=40;
constexpr int VK_OEM_PLUS=187, VK_OEM_MINUS=189;
inline void DebugBreak() { throw std::runtime_error("Unexpected game state"); }
inline void OutputDebugString(const wchar_t*) {}
#define APP_NAME "Augmentinel"
#define APP_VERSION "v1.6.0-web"
#include "SharedConstants.h"
#include "Utils.h"
#include "Sentinel.h"

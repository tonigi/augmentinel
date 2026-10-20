cmake_minimum_required(VERSION 3.20)
project(AugmentinelWeb LANGUAGES C CXX)
if(NOT EMSCRIPTEN)
  message(FATAL_ERROR "Use emcmake cmake for the browser build. Windows uses Augmentinel.sln.")
endif()
include(FetchContent)
# Header-only math and collision code used by both renderers; no Direct3D dependency.
FetchContent_Declare(directxmath
  GIT_REPOSITORY https://github.com/microsoft/DirectXMath.git
  GIT_TAG 0d821781da4ed0f20cad4e30e618afab45a08f0f)
FetchContent_MakeAvailable(directxmath)
set(SAL_DIR "${CMAKE_BINARY_DIR}/sal")
file(MAKE_DIRECTORY "${SAL_DIR}")
file(DOWNLOAD https://raw.githubusercontent.com/dotnet/runtime/v8.0.0/src/coreclr/pal/inc/rt/sal.h
  "${SAL_DIR}/sal.h"
  EXPECTED_HASH SHA256=7dae281adc3a09a691291fb90526f05e4f9ef8b16d7f33d716ba690f7241a492
  TLS_VERIFY ON)
# Existing packs contain Microsoft ADPCM, which Web Audio does not decode consistently.
# Convert into the build directory; retain the original assets unchanged.
find_program(FFMPEG_EXECUTABLE ffmpeg REQUIRED)
file(GLOB_RECURSE SOUND_FILES CONFIGURE_DEPENDS
  "${CMAKE_SOURCE_DIR}/sounds/Commodore Amiga/*.wav"
  "${CMAKE_SOURCE_DIR}/sounds/music/*.wav")
set(WEB_SOUNDS)
foreach(SOUND IN LISTS SOUND_FILES)
  file(RELATIVE_PATH REL_SOUND "${CMAKE_SOURCE_DIR}" "${SOUND}")
  set(OUTPUT "${CMAKE_BINARY_DIR}/assets/${REL_SOUND}")
  get_filename_component(OUTPUT_DIR "${OUTPUT}" DIRECTORY)
  add_custom_command(OUTPUT "${OUTPUT}"
    COMMAND "${CMAKE_COMMAND}" -E make_directory "${OUTPUT_DIR}"
    COMMAND "${FFMPEG_EXECUTABLE}" -nostdin -hide_banner -loglevel error -y -i "${SOUND}" -c:a pcm_s16le "${OUTPUT}"
    DEPENDS "${SOUND}" VERBATIM)
  list(APPEND WEB_SOUNDS "${OUTPUT}")
endforeach()
add_custom_target(web_audio_assets DEPENDS ${WEB_SOUNDS})
add_executable(augmentinel
  web/main.cpp web/WebView.cpp web/ViewCommon.cpp web/WebAudio.cpp web/WebSettings.cpp web/WebUtils.cpp
  src/Augmentinel.cpp src/Spectrum.cpp src/Model.cpp src/Camera.cpp src/Animate.cpp z80/Z80.c)
add_dependencies(augmentinel web_audio_assets)
set_property(TARGET augmentinel APPEND PROPERTY LINK_DEPENDS ${WEB_SOUNDS})
target_compile_features(augmentinel PRIVATE cxx_std_17)
target_include_directories(augmentinel PRIVATE src z80 resources "${directxmath_SOURCE_DIR}/Inc" "${SAL_DIR}")
target_compile_definitions(augmentinel PRIVATE _XM_NO_INTRINSICS_ CPU_Z80_STATIC CPU_Z80_USE_LOCAL_HEADER CPU_Z80_DEPENDENCIES_H="Z80-support.h")
target_compile_options(augmentinel PRIVATE -fexceptions)
target_link_options(augmentinel PRIVATE
  -fexceptions -sALLOW_MEMORY_GROWTH=1 -sMAX_WEBGL_VERSION=2 -sMIN_WEBGL_VERSION=2
  -sENVIRONMENT=web -sASSERTIONS=1 -sEXIT_RUNTIME=0 -sMINIFY_HTML=0
  "-sEXPORTED_RUNTIME_METHODS=['stringToNewUTF8']"
  "--shell-file=${CMAKE_SOURCE_DIR}/web/shell.html"
  "--preload-file=${CMAKE_SOURCE_DIR}/48.rom@/48.rom"
  "--preload-file=${CMAKE_SOURCE_DIR}/sentinel.sna@/sentinel.sna"
  "--preload-file=${CMAKE_BINARY_DIR}/assets/sounds@/sounds")
set_target_properties(augmentinel PROPERTIES OUTPUT_NAME index SUFFIX .html)
set_property(TARGET augmentinel APPEND PROPERTY LINK_DEPENDS "${CMAKE_SOURCE_DIR}/web/shell.html")

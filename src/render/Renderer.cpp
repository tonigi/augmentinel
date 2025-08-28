#include "Renderer.h"
#include <cstdio>

#ifdef __EMSCRIPTEN__
#include <emscripten/html5.h>
#include <GLES3/gl3.h>
static EMSCRIPTEN_WEBGL_CONTEXT_HANDLE g_ctx = 0;
#endif

bool Renderer::Init()
{
#ifdef __EMSCRIPTEN__
    EmscriptenWebGLContextAttributes attr;
    emscripten_webgl_init_context_attributes(&attr);
    attr.majorVersion = 2;
    attr.minorVersion = 0;
    g_ctx = emscripten_webgl_create_context("#canvas", &attr);
    if (g_ctx <= 0) {
        std::puts("Failed to create WebGL2 context");
        return false;
    }
    emscripten_webgl_make_context_current(g_ctx);
#endif
    std::puts("Renderer initialised");
    return true;
}

void Renderer::Render()
{
#ifdef __EMSCRIPTEN__
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);
#endif
    // TODO: draw a frame
}


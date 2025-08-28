#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include "game/GameState.h"
#include "render/Renderer.h"

static GameState g_game;
static Renderer g_renderer;

static void MainLoop()
{
    g_game.Update(1.0f / 60.0f);
    g_renderer.Render();
}

int main()
{
    if (!g_renderer.Init())
        return 1;

    emscripten_set_main_loop(MainLoop, 0, true);
    return 0;
}
#endif // __EMSCRIPTEN__


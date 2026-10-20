#include "stdafx.h"
#include "Augmentinel.h"
#include "Audio.h"
#include "View.h"
#include "Settings.h"
#include <emscripten/html5.h>
namespace {
std::shared_ptr<View> view;
std::shared_ptr<Audio> audio;
std::unique_ptr<Augmentinel> game;
double lastTime=0;
void failure(const char* message)
{
    emscripten_cancel_main_loop();
    EM_ASM({ Module.failure=UTF8ToString($0); Module.setStatus(Module.failure); document.body.dataset.failed='true'; console.error(Module.failure); },message);
}
EM_BOOL key(int type,const EmscriptenKeyboardEvent* e,void*)
{
    if(e->ctrlKey||e->altKey||e->metaKey) return false;
    if(!e->repeat) view->UpdateKey(e->keyCode,type==EMSCRIPTEN_EVENT_KEYDOWN?KeyState::DownEdge:KeyState::UpEdge);
    return true;
}
EM_BOOL mouse(int type,const EmscriptenMouseEvent* e,void*)
{
    if(type==EMSCRIPTEN_EVENT_MOUSEMOVE) {
        EmscriptenPointerlockChangeEvent lock;
        emscripten_get_pointerlock_status(&lock);
        if(lock.isActive) view->MouseMove(e->movementX,e->movementY);
    } else {
        const int buttons[]={VK_LBUTTON,VK_MBUTTON,VK_RBUTTON,VK_XBUTTON1};
        if(e->button>=0&&e->button<4) view->UpdateKey(buttons[e->button],type==EMSCRIPTEN_EVENT_MOUSEDOWN?KeyState::DownEdge:KeyState::UpEdge);
        if(type==EMSCRIPTEN_EVENT_MOUSEDOWN) emscripten_request_pointerlock("#canvas",false);
    }
    return true;
}
EM_BOOL blur(int,const EmscriptenFocusEvent*,void*) { view->ReleaseKeys(); return false; }
void frame()
{
    try {
        double now=emscripten_get_now(); float dt=std::min((now-lastTime)/1000.0,0.1); lastTime=now;
        // Avoid advancing the simulation while the tab is hidden.
        if(EM_ASM_INT({return document.hidden;})) { view->ReleaseKeys(); return; }
        game->Frame(dt);
        EM_ASM({ Module.gameState=$0; },static_cast<int>(game->State()));
        static GameState previous=GameState::Unknown;
        if (previous!=game->State()) {
            previous=game->State();
            const char* status="Playing";
            switch(previous) {
                case GameState::TitleScreen: status="Press Enter to choose a landscape"; break;
                case GameState::LandscapePreview: status="Choose a landscape with the arrow keys, then press Enter"; break;
                case GameState::Game: status="Click the game for mouse look · P pauses · Esc returns to landscapes"; break;
                case GameState::PlayerDead: status="Robot lost — press a key to continue"; break;
                case GameState::SkyView: status="Sky view — press a key to return"; break;
                case GameState::Complete: status="Landscape complete"; break;
                default: break;
            }
            EM_ASM({ Module.setStatus(UTF8ToString($0)); },status);
        }
        view->BeginScene(); view->Render(game.get()); view->EndScene();
        EM_ASM({ Module.frames=(Module.frames||0)+1; });
    } catch(const std::exception& e) { failure(e.what()); }
}
}
int main()
{
    try {
        InitSettings(APP_NAME);
        view=std::make_shared<View>(); audio=std::make_shared<Audio>();
        game=std::make_unique<Augmentinel>(view,audio);
        emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,nullptr,true,key);
        emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,nullptr,true,key);
        emscripten_set_mousedown_callback("#canvas",nullptr,true,mouse);
        emscripten_set_mouseup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,nullptr,true,mouse);
        emscripten_set_mousemove_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,nullptr,true,mouse);
        emscripten_set_blur_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW,nullptr,true,blur);
        EM_ASM({Module.setStatus('Ready — press Enter to play'); document.body.dataset.ready='true';});
        lastTime=emscripten_get_now(); emscripten_set_main_loop(frame,0,true);
    } catch(const std::exception& e) { failure(e.what()); }
    return 0;
}

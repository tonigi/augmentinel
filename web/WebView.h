#pragma once
#include "Action.h"
#include "Camera.h"
#include "Game.h"

enum class ViewEffect { Dissolve, Desaturate, Fade, ZFade, FogDensity };
enum class KeyState { Up, UpEdge, Down, DownEdge };
constexpr float NEAR_CLIP=.1f, FAR_CLIP=500.f;
constexpr float DEFAULT_EFFECT_TRANSITION_TIME = 0.04f;

class View final : public IScene
{
public:
    View();
    ~View();
    XMFLOAT3 GetEyePosition() const;
    XMFLOAT3 GetViewPosition() const;
    XMFLOAT3 GetViewDirection() const;
    XMFLOAT3 GetUpDirection() const;
    XMFLOAT3 GetCameraPosition() const;
    XMFLOAT3 GetCameraDirection() const;
    XMFLOAT3 GetCameraRotation() const;
    void SetCameraPosition(XMFLOAT3 pos);
    void SetCameraRotation(XMFLOAT3 rot);
    void SetMouseSpeed(int percent);
    void SetPitchLimits(float min_pitch, float max_pitch);
    void SetFillColour(int index);
    void SetFogColour(int index);
    void SetPalette(const std::vector<XMFLOAT4>& palette);
    void EnableAnimatedNoise(bool enable);
    bool PixelShaderEffectsActive() const;
    float GetEffect(ViewEffect effect) const;
    void SetEffect(ViewEffect effect, float value);
    bool TransitionEffect(ViewEffect effect, float target, float elapsed, float time=DEFAULT_EFFECT_TRANSITION_TIME);
    bool IsVR() const;
    bool IsSuspended() const;
    bool IsPointerVisible() const override { return false; }
    XMVECTOR GetEyePositionVector() const { return m_camera.GetPositionVector(); }
    XMVECTOR GetViewPositionVector() const { return m_camera.GetPositionVector(); }
    XMVECTOR GetViewDirectionVector() const { return m_camera.GetDirectionVector(); }
    XMVECTOR GetViewUpVector() const { return m_camera.GetUpVector(); }
    void GetSelectionRay(XMVECTOR& pos, XMVECTOR& dir) const { pos=GetEyePositionVector(); dir=GetViewDirectionVector(); }
    void SetVerticalFOV(float fov) { m_fov=fov; }
    void ResetHMD(bool = false) {}
    void BeginScene();
    void Render(IGame* game);
    void EndScene() {}
    void DrawModel(Model& model, const Model& linked={}) override;
    void DrawControllers() override {}
    void EnableFreeLook(bool enable);
    void MouseMove(int x, int y);
    void UpdateKey(int key, KeyState state);
    KeyState GetKeyState(int key);
    bool AnyKeyPressed();
    void ReleaseKeys();
    void SetInputBindings(const std::vector<ActionBinding>& bindings);
    void PollInputBindings(const std::vector<ActionBinding>& bindings);
    bool InputAction(Action action);
    void OutputAction(Action action);
private:
    Camera m_camera;
    bool m_freelook=true, m_invert_mouse=false, m_noise_enabled=true;
    float m_mouse_divider=740.0f, m_fov=45.0f, m_noise=0.0f;
    int m_fill_colour_idx=0;
    struct { std::array<XMFLOAT4, PALETTE_SIZE> Palette{}; int fog_colour_idx=0; float z_fade=0, fog_density=0; } m_vertexConstants;
    struct { float view_dissolve=0, view_desaturate=0, view_fade=0; } m_pixelConstants;
    std::map<int, KeyState> m_keys;
    std::set<int> m_pressed;
    std::map<Action, std::vector<int>> m_key_bindings;
    GLuint m_program=0, m_vao=0, m_framebuffer=0, m_texture=0, m_depth=0, m_postProgram=0;
    int m_width=0, m_height=0;
    XMMATRIX m_viewProjection{}, m_orthographic{};
};

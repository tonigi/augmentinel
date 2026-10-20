#include "stdafx.h"
#include "View.h"

XMFLOAT3 View::GetEyePosition() const
{
	XMFLOAT3 pos{};
	XMStoreFloat3(&pos, GetEyePositionVector());
	return pos;
}

XMFLOAT3 View::GetViewPosition() const
{
	XMFLOAT3 pos{};
	XMStoreFloat3(&pos, GetViewPositionVector());
	return pos;
}

XMFLOAT3 View::GetViewDirection() const
{
	XMFLOAT3 dir{};
	XMStoreFloat3(&dir, GetViewDirectionVector());
	return dir;
}

XMFLOAT3 View::GetUpDirection() const
{
	XMFLOAT3 up{};
	XMStoreFloat3(&up, GetViewUpVector());
	return up;
}

XMFLOAT3 View::GetCameraPosition() const
{
	return m_camera.GetPosition();
}

XMFLOAT3 View::GetCameraDirection() const
{
	return m_camera.GetDirection();
}

XMFLOAT3 View::GetCameraRotation() const
{
	return m_camera.GetRotations();
}

void View::SetCameraPosition(XMFLOAT3 pos)
{
	m_camera.SetPosition(pos);
}

void View::SetCameraRotation(XMFLOAT3 rot)
{
	m_camera.SetRotation(rot);
}

void View::SetMouseSpeed(int percent)
{
	percent = std::min(std::max(percent, 0), 100);
	m_mouse_divider = 2000.0f - (18.0f * percent);
}

void View::SetPitchLimits(float min_pitch, float max_pitch)
{
	m_camera.SetPitchLimits(min_pitch, max_pitch);
}

void View::SetFillColour(int fill_colour_idx)
{
	m_fill_colour_idx = fill_colour_idx;
}

void View::SetFogColour(int fog_colour_idx)
{
	m_vertexConstants.fog_colour_idx = fog_colour_idx;
}

void View::SetPalette(const std::vector<XMFLOAT4>& palette)
{
	auto copy_count = std::min(palette.size(), m_vertexConstants.Palette.size());
	std::copy(palette.begin(), palette.begin() + copy_count, m_vertexConstants.Palette.begin());
}

void View::EnableAnimatedNoise(bool enable)
{
	m_noise_enabled = enable;
}

bool View::PixelShaderEffectsActive() const
{
	// Pixel shader effects require an extra rendering step. If we know
	// none are active we can save a little time.
	return m_pixelConstants.view_dissolve != 0.0f ||
		m_pixelConstants.view_desaturate != 0.0f ||
		m_pixelConstants.view_fade != 0.0f;
}

float View::GetEffect(ViewEffect effect) const
{
	switch (effect)
	{
	case ViewEffect::Dissolve:
		return m_pixelConstants.view_dissolve;
	case ViewEffect::Desaturate:
		return m_pixelConstants.view_desaturate;
	case ViewEffect::Fade:
		return m_pixelConstants.view_fade;
	case ViewEffect::ZFade:
		return m_vertexConstants.z_fade;
	case ViewEffect::FogDensity:
		return m_vertexConstants.fog_density;
	default:
		assert(false);
		return 0.0f;
	}
}

void View::SetEffect(ViewEffect effect, float value)
{
	switch (effect)
	{
	case ViewEffect::Dissolve:
		m_pixelConstants.view_dissolve = value;
		break;
	case ViewEffect::Desaturate:
		m_pixelConstants.view_desaturate = value;
		break;
	case ViewEffect::Fade:
		m_pixelConstants.view_fade = value;
		break;
	case ViewEffect::ZFade:
		m_vertexConstants.z_fade = value;
		break;
	case ViewEffect::FogDensity:
		m_vertexConstants.fog_density = value;
		break;
	default:
		assert(false);
	}
}

bool View::TransitionEffect(ViewEffect effect, float target_value, float elapsed, float total_fade_time)
{
	auto current_value = GetEffect(effect);
	auto value = current_value;
	auto value_change = elapsed / total_fade_time;

	if (target_value > value)
		value = std::min(value + value_change, target_value);
	else
		value = std::max(value - value_change, target_value);

	SetEffect(effect, value);
	return current_value == target_value;
}

bool View::IsSuspended() const
{
	return false;
}

bool View::IsVR() const
{
	return false;
}


void View::EnableFreeLook(bool enable)
{
	m_freelook = enable;
}

void View::MouseMove(int x, int y)
{
	if (m_freelook)
	{
		m_camera.Yaw(x / m_mouse_divider);
		m_camera.Pitch(y / m_mouse_divider * (m_invert_mouse ? -1 : 1));
	}
}

void View::UpdateKey(int key, KeyState state)
{
    // Keep a short tap until the simulation consumes it, even if keyup occurs
    // between two animation frames. Track held keys separately.
    if (state == KeyState::DownEdge) { m_pressed.insert(key); m_keys[key]=KeyState::Down; }
    else if (state == KeyState::UpEdge) m_keys.erase(key);
}
KeyState View::GetKeyState(int key)
{
    if (m_pressed.erase(key)) return KeyState::DownEdge;
    auto it=m_keys.find(key);
    return it==m_keys.end()?KeyState::Up:it->second;
}
bool View::AnyKeyPressed()
{
    if (m_pressed.empty()) return false;
    m_pressed.erase(m_pressed.begin()); return true;
}
void View::ReleaseKeys() { m_keys.clear(); m_pressed.clear(); }

void View::SetInputBindings(const std::vector<ActionBinding>& bindings)
{
	m_key_bindings.clear();

	for (auto& binding : bindings)
	{
		if (binding.action != Action::None && !binding.virt_keys.empty())
			m_key_bindings[binding.action] = binding.virt_keys;
	}
}

void View::PollInputBindings(const std::vector<ActionBinding>&/*bindings*/)
{
	// VR only.
}

bool View::InputAction(Action action)
{
	auto it = m_key_bindings.find(action);
	if (it == m_key_bindings.end())
		return false;

	auto& virt_keys = it->second;
	for (auto& key : virt_keys)
	{
		if (key == VK_ANY)
			return AnyKeyPressed();
		else if (GetKeyState(key) == KeyState::DownEdge)
			return true;
	}

	return false;
}

void View::OutputAction(Action /*action*/)
{
	// VR only.
}

#pragma once
enum class AudioType { Unknown, Effect, LoopingEffect, Tune, Music };
class Audio
{
public:
    Audio();
    ~Audio();
    bool Available() const;
    bool IsPlaying(AudioType type=AudioType::Unknown) const;
    bool SetMusicVolume(float volume);
    bool SetMusicPlaying(bool play);
    void LoadWAV(fs::path path);
    float LengthInSeconds(const std::wstring& filename);
    bool Play(const std::wstring& filename, AudioType type=AudioType::Effect, XMFLOAT3 pos={}, float speed=1.0f);
    void Stop(AudioType type=AudioType::Unknown);
    void PositionListener(XMFLOAT3 pos, XMFLOAT3 front, XMFLOAT3 up);
};

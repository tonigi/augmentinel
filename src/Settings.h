#pragma once

static constexpr auto DEFAULT_SECTION = L"Main";
extern std::wstring settings_path;

void InitSettings(const std::string& app_name);
std::vector<std::wstring> GetSettingKeys(const std::wstring& section = DEFAULT_SECTION);
std::wstring GetSetting(const std::wstring& key, const std::wstring& default_value, const std::wstring& section = DEFAULT_SECTION);
int GetSetting(const std::wstring& key, int default_value, const std::wstring& section = DEFAULT_SECTION);
bool GetFlag(const std::wstring& key, bool default_value, const std::wstring& section = DEFAULT_SECTION);
void RemoveSetting(const std::wstring& key, const std::wstring& section = DEFAULT_SECTION);

#ifdef __EMSCRIPTEN__
void WriteSetting(const std::wstring& key, const std::wstring& value, const std::wstring& section);
#endif

template<typename T>
void SetSetting(const std::wstring& key, const T& value, const std::wstring& section = DEFAULT_SECTION)
{
	std::wstringstream ss;
	ss << value;

	assert(!settings_path.empty());
#ifdef __EMSCRIPTEN__
	WriteSetting(key, ss.str(), section);
#else
	WritePrivateProfileString(section.c_str(), key.c_str(), ss.str().c_str(), settings_path.c_str());
#endif
}

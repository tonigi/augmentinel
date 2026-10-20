#include "stdafx.h"
#include "Settings.h"
std::wstring settings_path;
namespace {
std::map<std::wstring,std::map<std::wstring,std::wstring>> settings;
void save()
{
    std::ostringstream out;
    for (auto& section:settings) for (auto& entry:section.second)
        out<<to_string(section.first)<<'\t'<<to_string(entry.first)<<'\t'<<to_string(entry.second)<<'\n';
    auto text=out.str();
    EM_ASM({ try { localStorage.setItem('augmentinel.settings',UTF8ToString($0)); } catch(e) { console.warn('Settings could not be saved',e); } },text.c_str());
}
}
void InitSettings(const std::string&)
{
    settings_path=L"browser-localStorage";
    char* text=reinterpret_cast<char*>(EM_ASM_PTR({
        try { return stringToNewUTF8(localStorage.getItem('augmentinel.settings')||''); } catch(e) { return stringToNewUTF8(''); }
    }));
    std::istringstream in(text); free(text);
    std::string line;
    while (std::getline(in,line)) {
        auto a=line.find('\t'), b=line.find('\t',a==std::string::npos?0:a+1);
        if (a!=std::string::npos && b!=std::string::npos)
            settings[to_wstring(line.substr(0,a))][to_wstring(line.substr(a+1,b-a-1))]=to_wstring(line.substr(b+1));
    }
}
std::vector<std::wstring> GetSettingKeys(const std::wstring& section)
{
    std::vector<std::wstring> keys; for(auto& e:settings[section]) keys.push_back(e.first); return keys;
}
std::wstring GetSetting(const std::wstring& key,const std::wstring& fallback,const std::wstring& section)
{
    auto& map=settings[section]; auto it=map.find(key); return it==map.end()?fallback:it->second;
}
int GetSetting(const std::wstring& key,int fallback,const std::wstring& section)
{
    try { return std::stoi(GetSetting(key,std::wstring{},section)); } catch(...) { return fallback; }
}
bool GetFlag(const std::wstring& key,bool fallback,const std::wstring& section) { return GetSetting(key,static_cast<int>(fallback),section)!=0; }
void WriteSetting(const std::wstring& key,const std::wstring& value,const std::wstring& section) { settings[section][key]=value; save(); }
void RemoveSetting(const std::wstring& key,const std::wstring& section) { settings[section].erase(key); save(); }

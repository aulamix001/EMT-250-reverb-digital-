#pragma once
#ifdef _WIN32
#include <windows.h>
#include <filesystem>
#include <fstream>
#include "PresetBrowser.h"
inline std::filesystem::path browserFavoritesPath(){wchar_t path[32768]{};auto n=GetEnvironmentVariableW(L"LOCALAPPDATA",path,32768);return n&&n<32768?std::filesystem::path(path)/L"Aulamix"/L"EMT250"/L"favorites.txt":std::filesystem::path();}
inline void loadBrowserFavorites(PresetBrowser&browser){auto path=browserFavoritesPath();if(path.empty())return;std::ifstream file(path);std::string line;while(std::getline(file,line)){if(!line.empty()&&line.back()=='\r')line.pop_back();for(int i=0;i<int(std::size(presets));++i)if(PresetBrowser::name(i)==line)browser.favorites.insert(i);}}
inline void saveBrowserFavorites(const PresetBrowser&browser){auto path=browserFavoritesPath();if(path.empty())return;std::error_code error;std::filesystem::create_directories(path.parent_path(),error);if(error)return;auto temp=path;temp+=L"."+std::to_wstring(GetCurrentProcessId())+L"."+std::to_wstring(GetCurrentThreadId())+L".tmp";{std::ofstream file(temp);if(!file)return;for(int id:browser.favorites)file<<PresetBrowser::name(id)<<'\n';file.flush();if(!file)return;}if(!MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))std::filesystem::remove(temp,error);}
#endif

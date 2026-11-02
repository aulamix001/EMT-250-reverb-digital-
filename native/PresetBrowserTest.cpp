#include "PresetBrowser.h"
#ifdef _WIN32
#include "BrowserPreferences.h"
#endif
#include <stdexcept>
#include <iostream>
void check(bool value,const char*message){if(!value)throw std::runtime_error(message);}
int main(){try{
 PresetBrowser b;b.rebuild();check(b.visible.size()==30,"complete catalog");
 for(int i=0;i<30;++i){check(b.visible[i]==i,"stable preset indices");check(!PresetBrowser::name(i).empty(),"nonempty name");}
 b.query="cHoRuS";b.rebuild();check(b.visible.size()==4,"case insensitive search");
 b.query="Male Vox Plate";b.rebuild();check(b.visible.size()==2&&b.visible[0]==20&&b.visible[1]==13,"exact name precedes substring match");
 b.query="";b.toggleTag("Guitar");check(b.visible.size()==7,"guitar mapping");b.toggleTag("Chorus");check(b.visible.size()==7,"union without duplicates");
 b.toggleTag("Vocal");check(b.visible.size()==12,"tag union");
 b.favorites={0,9,23};b.favOnly=true;b.rebuild();check(b.visible.size()==1&&b.visible[0]==0,"favorites intersect tags");
 b.selectedTags.clear();b.query="default";b.rebuild();check(b.visible.size()==1&&b.rowAt(336)==9,"filtered row maps to factory id");check(b.rowAt(335)==-1&&b.rowAt(368)==-1&&b.rowAt(688)==-1,"empty and outside rows ignored");
 b.query="no such preset";b.scroll=100;b.rebuild();check(b.scroll==0&&b.rowAt(350)==-1,"empty filter clamps scrolling");
 b.query="";b.favOnly=false;b.rebuild();b.scroll=999;b.rebuild();check(b.scroll==19,"scroll limit");check(b.rowAt(687)==29,"last row maps to last preset");b.reveal(0);check(b.scroll==0,"reveal first");b.reveal(29);check(b.scroll==19,"reveal last");
 b.toggleTag("Phase");check(b.visible.size()==1&&b.visible[0]==23,"last tag is accessible and exact");check(!PresetBrowser::hasTag(23,"Ph"),"no substring tag matches");

#ifdef _WIN32
 wchar_t original[32768]{},temp[32768]{};GetEnvironmentVariableW(L"LOCALAPPDATA",original,32768);GetTempPathW(32768,temp);auto folder=std::filesystem::path(temp)/(L"EMT250-browser-test-"+std::to_wstring(GetCurrentProcessId()));SetEnvironmentVariableW(L"LOCALAPPDATA",folder.c_str());
 b.favorites={0,9,29};saveBrowserFavorites(b);PresetBrowser loaded;loadBrowserFavorites(loaded);check(loaded.favorites==b.favorites,"favorites persist across instances");b.favorites.erase(9);saveBrowserFavorites(b);loaded.favorites.clear();loadBrowserFavorites(loaded);check(loaded.favorites==b.favorites,"removed favorite stays removed");std::error_code error;std::filesystem::remove_all(folder,error);SetEnvironmentVariableW(L"LOCALAPPDATA",*original?original:nullptr);
#endif
 std::cout<<"PRESET BROWSER TEST PASSED\n";return 0;
}catch(const std::exception&e){std::cerr<<e.what()<<'\n';return 1;}}

#pragma once
#include "Presets.h"
#include <algorithm>
#include <array>
#include <cctype>
#include <iterator>
#include <set>
#include <string>
#include <vector>
struct PresetBrowser {
 static constexpr std::array<const char*,13> tags{"Reverb","Best Of","Long","Short","Guitar","Vocal","Keys","Drums","Delay","Space","Echo","Chorus","Phase"};
 static constexpr int rows=11,listTop=336,rowHeight=32,tagTop=116,tagStep=34;
 std::string query;std::set<std::string> selectedTags;std::set<int> favorites;std::vector<int> visible;
 bool favOnly=false;int scroll=0,tagScroll=0;
 static std::string name(int id){std::string s=presets[id].name;return s.substr(0,s.find(" ["));}
 static std::string lower(std::string value){for(char&c:value)c=char(std::tolower(static_cast<unsigned char>(c)));return value;}
 static bool hasTag(int id,const std::string& tag){std::string all=presets[id].name;auto begin=all.find('[');if(begin==std::string::npos)return false;auto end=all.find(']',begin);std::string list=all.substr(begin+1,end-begin-1);size_t at=0;while(at<list.size()){auto comma=list.find(", ",at);if(list.substr(at,comma==std::string::npos?comma:comma-at)==tag)return true;if(comma==std::string::npos)break;at=comma+2;}return false;}
 void rebuild(){visible.clear();for(int i=0;i<int(std::size(presets));++i){bool tagged=selectedTags.empty();for(const auto&tag:selectedTags)tagged=tagged||hasTag(i,tag);if(tagged&&lower(name(i)).find(lower(query))!=std::string::npos&&(!favOnly||favorites.count(i)))visible.push_back(i);}if(!query.empty())std::stable_partition(visible.begin(),visible.end(),[&](int id){return lower(name(id))==lower(query);});scroll=std::clamp(scroll,0,maxScroll());tagScroll=std::clamp(tagScroll,0,1);}
 int maxScroll()const{return std::max(0,int(visible.size())-rows);}
 void toggleTag(const std::string&tag){if(!selectedTags.erase(tag))selectedTags.insert(tag);scroll=0;rebuild();}
 void reveal(int id){rebuild();auto found=std::find(visible.begin(),visible.end(),id);if(found==visible.end())return;int at=int(found-visible.begin());if(at<scroll)scroll=at;else if(at>=scroll+rows)scroll=at-rows+1;}
 int rowAt(int y)const{int row=(y-listTop)/rowHeight;return y>=listTop&&y<listTop+rows*rowHeight&&row+scroll<int(visible.size())?visible[row+scroll]:-1;}
};

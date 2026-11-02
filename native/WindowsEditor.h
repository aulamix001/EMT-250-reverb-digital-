#pragma once
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <fstream>
#include "public.sdk/source/common/pluginview.h"
#include "pluginterfaces/gui/iplugviewcontentscalesupport.h"
#include <functional>
#include <set>
#include <sstream>
#include <vector>
#include <algorithm>
#include <mutex>
#include "UiDiagnostics.h"
#include "NativeDpi.h"
#include "UiSkin.h"
#include "PresetBrowser.h"
#include "BrowserPreferences.h"
extern HINSTANCE ghInst;
class WindowsEditor:public Steinberg::CPluginView,public Steinberg::IPlugViewContentScaleSupport {
 HWND window=nullptr,popup=nullptr,search=nullptr;std::function<double(int)> read;std::function<void(int,double)> write;std::function<void()> done;
 std::function<void(int,bool)> gesture;std::function<void(int,int)> resizeHost;std::string formatName;bool firstPaint=true;float scale=1.f;bool scaleRequest=false;
 inline static std::mutex classMutex;inline static int classUsers=0;
 static const char* editorClass(){static std::string s="EMT250Editor-"+std::to_string((uintptr_t)ghInst);return s.c_str();}
 static const char* presetClass(){static std::string s="EMT250Presets-"+std::to_string((uintptr_t)ghInst);return s.c_str();}
 int drag=-1,popupDrag=0,popupDragOffset=0;PresetBrowser browser;std::array<double,13> clipboard{},slot{};bool copied=false,hasSlot=false;
 static constexpr int W=958,H=403;std::unique_ptr<UiArtwork> artwork;UiSkin skin;bool aSelected=true;std::array<double,14> lastPaint{};bool hasPaint=false;
 static COLORREF rgb(int n){return RGB(n,n,n);} 
 static void box(HDC d,int x,int y,int w,int h,COLORREF c){RECT r{x,y,x+w,y+h};auto b=CreateSolidBrush(c);FillRect(d,&r,b);DeleteObject(b);}
 static void text(HDC d,int x,int y,const std::string&s,int size=12,COLORREF c=RGB(220,220,220),bool bold=true){auto f=CreateFontA(-size,0,0,0,bold?FW_BOLD:FW_NORMAL,0,0,0,DEFAULT_CHARSET,0,0,ANTIALIASED_QUALITY,0,"Arial");auto old=SelectObject(d,f);SetBkMode(d,TRANSPARENT);SetTextColor(d,c);TextOutA(d,x,y,s.c_str(),int(s.size()));SelectObject(d,old);DeleteObject(f);}
 static void line(HDC d,int x,int y,int x2,int y2,COLORREF c,int width=1){auto p=CreatePen(PS_SOLID,width,c);auto old=SelectObject(d,p);MoveToEx(d,x,y,nullptr);LineTo(d,x2,y2);SelectObject(d,old);DeleteObject(p);}
 static void ellipse(HDC d,int x,int y,int w,int h,COLORREF fill,COLORREF edge){auto b=CreateSolidBrush(fill);auto p=CreatePen(PS_SOLID,1,edge);auto ob=SelectObject(d,b);auto op=SelectObject(d,p);Ellipse(d,x,y,x+w,y+h);SelectObject(d,ob);SelectObject(d,op);DeleteObject(b);DeleteObject(p);}
 static void gradient(HDC d,int x,int y,int w,int h,COLORREF a,COLORREF b,bool horizontal=false){for(int i=0;i<(horizontal?w:h);++i){double t=double(i)/(horizontal?w:h);auto c=RGB(int(GetRValue(a)*(1-t)+GetRValue(b)*t),int(GetGValue(a)*(1-t)+GetGValue(b)*t),int(GetBValue(a)*(1-t)+GetBValue(b)*t));box(d,x+(horizontal?i:0),y+(horizontal?0:i),horizontal?1:w,horizontal?h:1,c);}}
 static void led(HDC d,int x,int y,int color,bool active){COLORREF lit=color==0?RGB(255,25,28):color==1?RGB(35,255,42):RGB(255,239,25);if(active)ellipse(d,x-6,y-6,12,12,color==0?RGB(85,10,12):color==1?RGB(8,75,14):RGB(80,75,5),rgb(45));ellipse(d,x-4,y-4,8,8,active?lit:color==0?RGB(35,8,8):RGB(7,35,12),rgb(65));ellipse(d,x-2,y-3,3,2,active?RGB(255,255,190):rgb(65),rgb(55));}
 static void button(HDC d,int x,int y,int w,int h,const std::string&s,bool active=false,int color=0){box(d,x,y,w,h,rgb(9));gradient(d,x+2,y+2,w-4,h-4,rgb(active?75:48),rgb(28));if(active){auto a=color==0?RGB(255,35,27):color==1?RGB(52,237,24):RGB(255,231,45);gradient(d,x+5,y+5,w-10,h-10,a,color==0?RGB(186,15,8):color==1?RGB(20,150,8):RGB(190,160,15));line(d,x+6,y+6,x+w-6,y+6,rgb(230));}text(d,x+8,y+h/2-5,s,10,active?rgb(18):rgb(215));}
 int current(){return std::clamp(int(std::round(read(0)*(std::size(presets)-1))),0,int(std::size(presets))-1);} 
 std::string name(int i){std::string n=presets[i].name;return n.substr(0,n.find(" ["));}
 void preset(int i){i=std::clamp(i,0,int(std::size(presets))-1);auto&p=presets[i];write(0,i/double(std::size(presets)-1));write(1,p.mode/5.);for(int j=0;j<4;++j)write(2+j,p.f[j]/double(j==0?15:3));write(6,p.rear);write(7,p.mix);write(11,p.left/315);write(12,p.right/315);InvalidateRect(window,nullptr,FALSE);if(popup)InvalidateRect(popup,nullptr,FALSE);}
 void saveFile(){char path[MAX_PATH]="Preset.a250";OPENFILENAMEA f{};f.lStructSize=sizeof(f);f.hwndOwner=popup?popup:window;f.lpstrFilter="Aulamix preset (*.a250)\0*.a250\0\0";f.lpstrFile=path;f.nMaxFile=MAX_PATH;f.lpstrDefExt="a250";f.Flags=OFN_OVERWRITEPROMPT;if(GetSaveFileNameA(&f)){std::ofstream out(path,std::ios::binary);for(int i=0;i<13;++i){double v=read(i);out.write((char*)&v,sizeof(v));}}}
 void openFile(){char path[MAX_PATH]="";OPENFILENAMEA f{};f.lStructSize=sizeof(f);f.hwndOwner=window;f.lpstrFilter="Aulamix preset (*.a250)\0*.a250\0\0";f.lpstrFile=path;f.nMaxFile=MAX_PATH;f.Flags=OFN_FILEMUSTEXIST;if(GetOpenFileNameA(&f)){std::ifstream in(path,std::ios::binary);std::array<double,13> values{};for(auto&v:values)if(!in.read((char*)&v,sizeof(v))||!std::isfinite(v)||v<0||v>1)return;for(int i=0;i<13;++i)write(i,values[i]);}}
 void rebuild(){browser.rebuild();}
 void star(HDC d,int x,int y,bool filled){POINT points[10];for(int i=0;i<10;++i){double angle=-1.57079632679+i*3.14159265359/5,r=i%2?3.5:8.;points[i]={LONG(std::lround(x+9+r*std::cos(angle))),LONG(std::lround(y+9+r*std::sin(angle)))};}auto pen=CreatePen(PS_SOLID,1,filled?rgb(220):rgb(118));auto brush=CreateSolidBrush(filled?rgb(180):rgb(36));auto oldPen=SelectObject(d,pen),oldBrush=SelectObject(d,brush);Polygon(d,points,10);SelectObject(d,oldPen);SelectObject(d,oldBrush);DeleteObject(pen);DeleteObject(brush);}
 void paintPopup(HDC d){
  box(d,0,0,390,710,rgb(23));box(d,1,1,388,34,rgb(57));text(d,124,11,"EMT 250 REVERB PRESETS",10);line(d,372,12,382,22,rgb(191),2);line(d,382,12,372,22,rgb(191),2);
  box(d,9,37,372,36,rgb(17));ellipse(d,21,48,10,10,rgb(17),rgb(148));line(d,30,57,35,62,rgb(148),2);
  box(d,9,80,372,172,rgb(36));text(d,22,93,"TAGS",10);text(d,279,93,"FAVORITES",10,browser.favOnly?rgb(238):rgb(190));star(d,360,87,browser.favOnly);
  for(int row=0;row<4;++row)for(int col=0;col<3;++col){int id=(row+browser.tagScroll)*3+col;if(id>=int(PresetBrowser::tags.size()))continue;int x=16+col*118,y=116+row*34;bool selected=browser.selectedTags.count(PresetBrowser::tags[id]);box(d,x,y,112,29,selected?rgb(61):rgb(25));line(d,x,y,x+111,y,rgb(12));text(d,x+56-int(std::string(PresetBrowser::tags[id]).size())*3,y+9,PresetBrowser::tags[id],10);}
  box(d,373,116,8,132,rgb(17));box(d,374,118+browser.tagScroll*28,6,100,rgb(62));
  box(d,10,260,371,67,rgb(38));line(d,11,260,380,260,rgb(69));std::string title=name(current());text(d,195-int(title.size())*3,272,title,11);star(d,356,265,browser.favorites.count(current()));
  const char* labels[]={"DELETE","RENAME","SAVE"};for(int i=0;i<3;++i){int x=22+i*119;box(d,x,294,113,28,rgb(i==2?61:55));line(d,x,294,x+112,294,rgb(78));text(d,x+34,303,labels[i],10,i==2?rgb(224):rgb(91));}
  rebuild();box(d,10,334,371,367,rgb(20));for(int row=0;row<PresetBrowser::rows;++row){int index=browser.scroll+row;if(index>=int(browser.visible.size()))break;int id=browser.visible[index],y=336+row*32;if(id==current())box(d,15,y,357,32,rgb(59));text(d,25,y+10,name(id),11);if(browser.favorites.count(id))text(d,355,y+9,"*",13,rgb(172));}
  box(d,375,337,7,351,rgb(12));if(browser.maxScroll()>0){int height=std::max(24,351*PresetBrowser::rows/int(browser.visible.size()));int top=337+(351-height)*browser.scroll/browser.maxScroll();box(d,376,top,5,height,rgb(69));}
 }
 void scrollPopup(int y){if(popupDrag==1){browser.rebuild();int height=std::max(24,351*PresetBrowser::rows/std::max(1,int(browser.visible.size())));browser.scroll=std::clamp(int(std::lround((y-337-popupDragOffset)*browser.maxScroll()/double(std::max(1,351-height)))),0,browser.maxScroll());}else if(popupDrag==2)browser.tagScroll=y>182?1:0;InvalidateRect(popup,nullptr,FALSE);}
 void popupClick(int x,int y){
  if(y<35){if(x>353)ShowWindow(popup,SW_HIDE);else SendMessage(popup,WM_NCLBUTTONDOWN,HTCAPTION,0);return;}
  if(y>=80&&y<111&&x>=275){browser.favOnly=!browser.favOnly;browser.scroll=0;}
  else if(y>=116&&y<248){if(x>=373){popupDrag=2;SetCapture(popup);scrollPopup(y);return;}if(x>=16&&x<370){int col=(x-16)/118,row=(y-116)/34;int id=(row+browser.tagScroll)*3+col;if(col<3&&(x-16)%118<112&&(y-116)%34<29&&id<int(PresetBrowser::tags.size()))browser.toggleTag(PresetBrowser::tags[id]);}}
  else if(y>=260&&y<293&&x>340){int id=current();if(!browser.favorites.erase(id))browser.favorites.insert(id);saveBrowserFavorites(browser);}
  else if(y>=294&&y<323){if(x>=260&&x<373)saveFile();}
  else if(y>=336&&y<688){if(x>=373){rebuild();int height=std::max(24,351*PresetBrowser::rows/std::max(1,int(browser.visible.size())));int top=337+(351-height)*browser.scroll/std::max(1,browser.maxScroll());popupDragOffset=y>=top&&y<top+height?y-top:height/2;popupDrag=1;SetCapture(popup);scrollPopup(y);return;}rebuild();int id=browser.rowAt(y);if(id>=0)preset(id);}
  InvalidateRect(popup,nullptr,FALSE);
 }
 void floating(){
  if(popup){browser.reveal(current());ShowWindow(popup,SW_SHOW);SetForegroundWindow(popup);SetFocus(search);return;}
  RECT r;GetWindowRect(window,&r);RECT client;GetClientRect(window,&client);
  float popScale=std::min({scale,client.right/float(W),client.bottom/float(H)});
  DWORD style=WS_POPUP|WS_BORDER|WS_SYSMENU;
  RECT decoration{0,0,390,710};AdjustWindowRectEx(&decoration,style,FALSE,WS_EX_TOOLWINDOW);
  MONITORINFO monitor{};monitor.cbSize=sizeof(monitor);
  bool work=GetMonitorInfoA(MonitorFromWindow(window,MONITOR_DEFAULTTONEAREST),&monitor)!=FALSE;
  if(work){float width=float(monitor.rcWork.right-monitor.rcWork.left-(decoration.right-decoration.left-390)-16);float height=float(monitor.rcWork.bottom-monitor.rcWork.top-(decoration.bottom-decoration.top-710)-16);popScale=std::min({popScale,width/390,height/710});}
  if(popScale<=0)return;
  RECT size{0,0,int(std::lround(390*popScale)),int(std::lround(710*popScale))};AdjustWindowRectEx(&size,style,FALSE,WS_EX_TOOLWINDOW);
  int width=size.right-size.left,height=size.bottom-size.top,x=r.right+10,y=r.top;if(work&&x+width>monitor.rcWork.right)x=r.left-width-10;
  if(work){x=std::clamp(x,int(monitor.rcWork.left),int(monitor.rcWork.right-width));y=std::clamp(y,int(monitor.rcWork.top),int(monitor.rcWork.bottom-height));}
  popup=CreateWindowExA(WS_EX_TOOLWINDOW,presetClass(),"Presets",style,x,y,width,height,GetAncestor(window,GA_ROOT),nullptr,ghInst,this);
  if(!popup){uiDiagnostic("preset CreateWindowEx failed error=%lu",GetLastError());return;}
  search=CreateWindowExA(0,"EDIT","",WS_CHILD|WS_VISIBLE|WS_TABSTOP|ES_AUTOHSCROLL,int(std::lround(40*popScale)),int(std::lround(44*popScale)),int(std::lround(332*popScale)),int(std::lround(25*popScale)),popup,(HMENU)1,ghInst,nullptr);browser.reveal(current());ShowWindow(popup,SW_SHOWNORMAL);SetForegroundWindow(popup);SetFocus(search);
 }

 void move(int x,int y){
  if(drag>=2&&drag<=5){int i=drag-2,mode=int(std::round(read(1)*5));
   const double bases[]={239,253,225,225},travel[]={222,174,174,174};double n=std::clamp((bases[i]-y)/travel[i],0.,1.);int steps=i==0?15:3;write(drag,std::round(n*steps)/steps);
   if(mode==1&&i==2){int t=int(std::round(read(read(4)<.5?11:12)*63))*5;write(2,(t/20)/15.);write(3,((t%20)/5)/3.);}
   if(mode==1&&(i==0||i==1))write(read(4)<.5?11:12,(std::round(read(2)*15)*20+std::round(read(3)*3)*5)/315.);
  }else if(drag==7)write(7,std::clamp(double(x-548)/107,0.,1.));
  InvalidateRect(window,nullptr,FALSE);
 }
 void compare(bool targetA){
  if(targetA==aSelected)return;
  if(!hasSlot){for(int i=0;i<13;++i)slot[i]=read(i);hasSlot=true;}
  for(int i=0;i<13;++i){double current=read(i);write(i,slot[i]);slot[i]=current;}aSelected=targetA;
 }
 void information(){MessageBoxA(window,"EMT-250 Reverb 0.5.0 - VST3 / VST2\n\nDSP original aproximado e interface redesenhada a partir da referencia.\nSem cobranca, prazo demo ou algoritmo EMT/UAD proprietario.\nUse as funcoes de automacao/MIDI da DAW para mapear os controles.\n\nBotao direito: presets, A/B, COPY/PASTE e arquivos .a250.","EMT-250 Reverb",MB_OK|MB_ICONINFORMATION);}
 void context(int x,int y){
  HMENU menu=CreatePopupMenu();AppendMenuA(menu,MF_STRING,1,"Biblioteca de presets...");AppendMenuA(menu,MF_STRING,2,"Preset anterior");AppendMenuA(menu,MF_STRING,3,"Proximo preset");AppendMenuA(menu,MF_SEPARATOR,0,nullptr);
  AppendMenuA(menu,MF_STRING|(aSelected?MF_CHECKED:0),4,"Configuracao A");AppendMenuA(menu,MF_STRING|(!aSelected?MF_CHECKED:0),5,"Configuracao B");AppendMenuA(menu,MF_STRING,6,"COPY");AppendMenuA(menu,MF_STRING|(copied?0:MF_GRAYED),7,"PASTE");AppendMenuA(menu,MF_SEPARATOR,0,nullptr);
  AppendMenuA(menu,MF_STRING,8,"Salvar .a250...");AppendMenuA(menu,MF_STRING,9,"Importar .a250...");AppendMenuA(menu,MF_STRING,10,"LEARN / automacao MIDI...");AppendMenuA(menu,MF_STRING,11,"INFO");
  if(x==-1&&y==-1){RECT r;GetWindowRect(window,&r);x=r.left+40;y=r.top+40;}
  int action=TrackPopupMenu(menu,TPM_RETURNCMD|TPM_RIGHTBUTTON,x,y,0,window,nullptr);DestroyMenu(menu);
  command(action);
 }
 void command(int action){
  switch(action){case 1:floating();break;case 2:preset(current()-1);break;case 3:preset(current()+1);break;case 4:compare(true);break;case 5:compare(false);break;case 6:for(int i=0;i<13;++i)clipboard[i]=read(i);copied=true;break;case 7:if(copied)for(int i=0;i<13;++i)write(i,clipboard[i]);break;case 8:saveFile();break;case 9:openFile();break;case 10:case 11:information();break;}
  InvalidateRect(window,nullptr,FALSE);
 }
 void click(int x,int y){
  if(y<0){if(x<28)preset(current()-1);else if(x<56)preset(current()+1);else if(x<400)floating();else if(x<445)write(10,1-read(10));else if(x<480)compare(true);else if(x<515)compare(false);else if(x<580)command(6);else if(x<650)command(7);else if(x>880)information();return;}

  if(x>39&&x<70&&y>18&&y<52){write(10,1-read(10));return;}
  if(x>35&&x<140&&y>57&&y<81){floating();return;}
  if(x<140&&y>308&&y<340)write(9,1-read(9));
  int bx[]={191,296,328,360,392,424};for(int i=0;i<6;++i)if(x>=bx[i]&&x<bx[i]+34&&y>305&&y<342)write(1,i/5.);
  if(x>477&&x<504&&y>315&&y<345)write(6,1-read(6));
  if(x>697&&x<734&&y>306&&y<344)write(8,1-read(8));
  if(y>0&&y<286){int xs[]={240,385,531,675};for(int i=0;i<4;++i)if(std::abs(x-xs[i])<24){drag=2+i;if(gesture)gesture(drag,true);SetCapture(window);move(x,y);}}
  if(x>525&&x<680&&y>320&&y<343){drag=7;if(gesture)gesture(drag,true);SetCapture(window);move(x,y);}
 }
 void render(HWND h,HDC target,bool pop){
  RECT client{};GetClientRect(h,&client);if(client.right<=0||client.bottom<=0)return;
  if(!pop){std::array<double,14> values{};for(int i=0;i<14;++i)values[i]=read(i);auto frame=artwork->frameBuffer(client.right,client.bottom);if(!frame)return;{Gdiplus::Graphics graphics(frame);graphics.SetPageUnit(Gdiplus::UnitPixel);graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);UiArtwork::fill(graphics,0,0,float(client.right),float(client.bottom),Gdiplus::Color(255,3,3,3));float zoom=std::min(client.right/float(W),client.bottom/float(H));float x=(client.right-W*zoom)/2,y=(client.bottom-H*zoom)/2;graphics.TranslateTransform(x,y);graphics.ScaleTransform(zoom,zoom);UiArtwork::fill(graphics,0,0,958,32,Gdiplus::Color(255,36,36,36));
   auto label=[&](float x,float w,const std::string&value){artwork->text(graphics,x,3,w,25,value,11,Gdiplus::Color(255,224,224,215));};
   label(0,28,"<");label(28,28,">");label(56,344,name(current())+"  v");label(400,45,values[10]<.5?"IN":"OFF");label(445,35,aSelected?"[A]":"A");label(480,35,aSelected?"B":"[B]");label(515,65,"COPY");label(580,70,"PASTE");label(680,190,"0.5.0  "+formatName);label(880,78,"INFO");
   graphics.TranslateTransform(0,32);artwork->prepareBackground(int(std::lround(W*zoom)),int(std::lround(371*zoom)));skin.draw(*artwork,graphics,values);}Gdiplus::Graphics presentation(target);presentation.SetPageUnit(Gdiplus::UnitPixel);presentation.DrawImage(frame,Gdiplus::Rect(0,0,client.right,client.bottom),0,0,client.right,client.bottom,Gdiplus::UnitPixel);lastPaint=values;hasPaint=true;return;}
  int width=390,height=710;
  HDC buffer=CreateCompatibleDC(target);HBITMAP bitmap=CreateCompatibleBitmap(target,width,height);
  if(!buffer||!bitmap){uiDiagnostic("paint buffer failed error=%lu",GetLastError());if(bitmap)DeleteObject(bitmap);if(buffer)DeleteDC(buffer);return;}
  auto previous=SelectObject(buffer,bitmap);paintPopup(buffer);
  int oldMode=SetStretchBltMode(target,HALFTONE);POINT oldOrigin{};SetBrushOrgEx(target,0,0,&oldOrigin);
  StretchBlt(target,0,0,client.right,client.bottom,buffer,0,0,width,height,SRCCOPY);
  SetBrushOrgEx(target,oldOrigin.x,oldOrigin.y,nullptr);SetStretchBltMode(target,oldMode);SelectObject(buffer,previous);DeleteObject(bitmap);DeleteDC(buffer);
 }
 static POINT logicalPoint(HWND h,LPARAM value,bool pop){
  RECT client{};GetClientRect(h,&client);
  if(!pop){double zoom=std::min(client.right/double(W),client.bottom/double(H));if(zoom<=0)return {-1,-1};return {LONG(std::lround((GET_X_LPARAM(value)-(client.right-W*zoom)/2)/zoom)),LONG(std::lround((GET_Y_LPARAM(value)-(client.bottom-H*zoom)/2)/zoom))-32};}
  return {LONG(std::lround(double(GET_X_LPARAM(value))*(pop?390:W)/std::max(1L,client.right))),LONG(std::lround(double(GET_Y_LPARAM(value))*(pop?710:H)/std::max(1L,client.bottom)))};
 }
 static LRESULT CALLBACK proc(HWND h,UINT m,WPARAM w,LPARAM l){auto*self=(WindowsEditor*)GetWindowLongPtrA(h,0);if(m==WM_NCCREATE){auto*create=(CREATESTRUCTA*)l;self=(WindowsEditor*)create->lpCreateParams;if(!self)return FALSE;SetWindowLongPtrA(h,0,(LONG_PTR)self);SetWindowLongPtrA(h,sizeof(LONG_PTR),strcmp(create->lpszClass,presetClass())==0);uiDiagnostic("WM_NCCREATE hwnd=%p parent=%p instance=%p",h,create->hwndParent,create->hInstance);return TRUE;}if(!self)return DefWindowProcA(h,m,w,l);bool pop=GetWindowLongPtrA(h,sizeof(LONG_PTR))!=0;if(m==WM_NCDESTROY){SetWindowLongPtrA(h,0,0);if(h==self->window)self->window=nullptr;if(h==self->popup){self->popup=nullptr;self->search=nullptr;}return DefWindowProcA(h,m,w,l);}if(m==WM_COMMAND&&!pop&&LOWORD(w)>=1&&LOWORD(w)<=11){self->command(LOWORD(w));return 0;}if(m==WM_CONTEXTMENU&&!pop){self->context(GET_X_LPARAM(l),GET_Y_LPARAM(l));return 0;}if(m==WM_PRINTCLIENT){self->render(h,(HDC)w,pop);return 0;}if(m==WM_ERASEBKGND)return 1;if(m==WM_PAINT){if(!pop&&self->firstPaint){uiDiagnostic("first WM_PAINT hwnd=%p",h);self->firstPaint=false;}PAINTSTRUCT ps;HDC d=BeginPaint(h,&ps);self->render(h,d,pop);EndPaint(h,&ps);return 0;}if(m==0x02E3&&!pop&&self->formatName=="VST2"){self->setContentScaleFactor(nativeDpiScale(h));return 0;}if(m==WM_TIMER){bool dirty=!self->hasPaint;for(int i=0;i<13;++i)if(self->read(i)!=self->lastPaint[i])dirty=true;auto level=[](double meter){return meter>=1?4:meter>=.5?3:meter>=.25?2:meter>=.125?1:0;};if(level(self->read(13))!=level(self->lastPaint[13]))dirty=true;if(dirty){InvalidateRect(h,nullptr,FALSE);if(self->popup)InvalidateRect(self->popup,nullptr,FALSE);}return 0;}if(pop){if(m==WM_CTLCOLOREDIT||m==WM_CTLCOLORSTATIC){SetTextColor((HDC)w,RGB(220,220,220));SetBkColor((HDC)w,RGB(0,0,0));return (LRESULT)GetStockObject(BLACK_BRUSH);}if(m==WM_CLOSE){ShowWindow(h,SW_HIDE);return 0;}if(m==WM_COMMAND&&HIWORD(w)==EN_CHANGE){char b[256]{};GetWindowTextA((HWND)l,b,256);self->browser.query=b;self->browser.scroll=0;self->rebuild();InvalidateRect(h,nullptr,FALSE);return 0;}
 if(m==WM_MOUSEWHEEL){POINT cursor{GET_X_LPARAM(l),GET_Y_LPARAM(l)};ScreenToClient(h,&cursor);POINT point=logicalPoint(h,MAKELPARAM(cursor.x,cursor.y),true);int amount=GET_WHEEL_DELTA_WPARAM(w)>0?-1:1;if(point.y>=80&&point.y<252)self->browser.tagScroll=std::clamp(self->browser.tagScroll+amount,0,1);else{self->browser.scroll+=amount*3;self->rebuild();}InvalidateRect(h,nullptr,FALSE);return 0;}
 if(m==WM_LBUTTONDOWN){POINT point=logicalPoint(h,l,true);self->popupClick(point.x,point.y);return 0;}
 if(m==WM_MOUSEMOVE&&self->popupDrag){POINT point=logicalPoint(h,l,true);self->scrollPopup(point.y);return 0;}
 if(m==WM_LBUTTONUP){self->popupDrag=0;if(GetCapture()==h)ReleaseCapture();return 0;}
 if(m==WM_CAPTURECHANGED){self->popupDrag=0;return 0;}
 }else{if(m==WM_LBUTTONDOWN){POINT point=logicalPoint(h,l,false);self->click(point.x,point.y);InvalidateRect(h,nullptr,FALSE);return 0;}if(m==WM_MOUSEMOVE&&self->drag>=0){POINT point=logicalPoint(h,l,false);self->move(point.x,point.y);return 0;}if(m==WM_LBUTTONUP){int id=self->drag;self->drag=-1;if(id>=0&&self->gesture)self->gesture(id,false);if(GetCapture()==h)ReleaseCapture();return 0;}if(m==WM_CAPTURECHANGED){int id=self->drag;self->drag=-1;if(id>=0&&self->gesture)self->gesture(id,false);return 0;}}return DefWindowProcA(h,m,w,l);}
public:
 WindowsEditor(std::function<double(int)>r,std::function<void(int,double)>w,std::function<void()>finish,std::string format="VST3",std::function<void(int,int)>resize={},std::function<void(int,bool)>gestureCallback={}):read(r),write(w),done(finish),gesture(std::move(gestureCallback)),resizeHost(std::move(resize)),formatName(std::move(format)){
  rect={0,0,W,H};loadBrowserFavorites(browser);artwork=std::make_unique<UiArtwork>();
  std::lock_guard<std::mutex> lock(classMutex);
  if(classUsers++==0){
   WNDCLASSA wc{};wc.lpfnWndProc=proc;wc.hInstance=ghInst;wc.cbWndExtra=2*sizeof(LONG_PTR);wc.hCursor=LoadCursor(nullptr,IDC_ARROW);
   wc.lpszClassName=editorClass();ATOM main=RegisterClassA(&wc);DWORD mainError=main?0:GetLastError();
   wc.lpszClassName=presetClass();ATOM popupAtom=RegisterClassA(&wc);
   uiDiagnostic("register classes instance=%p main=%u error=%lu popup=%u error=%lu",ghInst,main,mainError,popupAtom,popupAtom?0:GetLastError());
  }
 }
 ~WindowsEditor(){removed();artwork.reset();{std::lock_guard<std::mutex> lock(classMutex);if(--classUsers==0){UnregisterClassA(presetClass(),ghInst);UnregisterClassA(editorClass(),ghInst);}}done();}
 Steinberg::uint32 PLUGIN_API addRef()override{return CPluginView::addRef();}
 Steinberg::uint32 PLUGIN_API release()override{return CPluginView::release();}
 Steinberg::tresult PLUGIN_API queryInterface(const Steinberg::TUID iid,void**object)override{
  if(!object)return Steinberg::kInvalidArgument;
  if(Steinberg::FUnknownPrivate::iidEqual(iid,Steinberg::IPlugViewContentScaleSupport::iid)){
   *object=static_cast<Steinberg::IPlugViewContentScaleSupport*>(this);addRef();return Steinberg::kResultOk;
  }
  return CPluginView::queryInterface(iid,object);
 }
 Steinberg::tresult PLUGIN_API setContentScaleFactor(ScaleFactor factor)override{
  if(!std::isfinite(factor)||factor<=0||W*double(factor)>32767||H*double(factor)>32767||std::lround(W*factor)<1||std::lround(H*factor)<1||scaleRequest)return Steinberg::kInvalidArgument;
  auto next=rect;next.right=next.left+int(std::lround(W*factor));next.bottom=next.top+int(std::lround(H*factor));
  if(factor==scale&&next.getWidth()==rect.getWidth()&&next.getHeight()==rect.getHeight())return Steinberg::kResultOk;
  uiDiagnostic("setContentScaleFactor=%g",double(factor));scaleRequest=true;
  auto result=plugFrame?plugFrame->resizeView(static_cast<Steinberg::IPlugView*>(this),&next):onSize(&next);
  scaleRequest=false;if(result!=Steinberg::kResultOk)return result;
  scale=factor;
  if(window&&resizeHost)resizeHost(next.getWidth(),next.getHeight());
  return Steinberg::kResultOk;
 }
 Steinberg::tresult PLUGIN_API canResize()override{return Steinberg::kResultTrue;}
 Steinberg::tresult PLUGIN_API checkSizeConstraint(Steinberg::ViewRect*size)override{
  return size&&size->getWidth()>0&&size->getHeight()>0&&size->getWidth()<=32767&&size->getHeight()<=32767?Steinberg::kResultTrue:Steinberg::kResultFalse;
 }
 Steinberg::tresult PLUGIN_API isPlatformTypeSupported(Steinberg::FIDString type)override{return type&&strcmp(type,Steinberg::kPlatformTypeHWND)==0?Steinberg::kResultTrue:Steinberg::kResultFalse;}
 Steinberg::tresult PLUGIN_API onSize(Steinberg::ViewRect*size)override{
  if(!size||size->getWidth()<=0||size->getHeight()<=0)return Steinberg::kInvalidArgument;
  // The host owns the container position; the child uses its local origin,
  // matching the SDK VSTGUI editor. ViewRect dimensions resize the child.
  uiDiagnostic("onSize rect=%ld,%ld,%ld,%ld hwnd=%p",long(size->left),long(size->top),long(size->right),long(size->bottom),window);
  if(window&&!SetWindowPos(window,nullptr,0,0,size->getWidth(),size->getHeight(),SWP_NOACTIVATE|SWP_NOZORDER)){uiDiagnostic("SetWindowPos failed error=%lu",GetLastError());return Steinberg::kResultFalse;}
  rect=*size;
  if(window)RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_UPDATENOW);
  return Steinberg::kResultOk;
 }
 Steinberg::tresult PLUGIN_API attached(void*parent,Steinberg::FIDString type)override{
  uiDiagnostic("attached parent=%p valid=%d platform=%s",parent,IsWindow((HWND)parent),type?type:"(null)");
  if(!IsWindow((HWND)parent)||isPlatformTypeSupported(type)!=Steinberg::kResultTrue)return Steinberg::kResultFalse;
  DWORD parentPid=0;DWORD parentThread=GetWindowThreadProcessId((HWND)parent,&parentPid);RECT parentBounds{};GetClientRect((HWND)parent,&parentBounds);uiDiagnostic("container pid=%lu tid=%lu size=%ldx%ld",parentPid,parentThread,parentBounds.right,parentBounds.bottom);
  removed();
  window=CreateWindowExA(0,editorClass(),"",WS_CHILD|WS_VISIBLE|WS_CLIPSIBLINGS|WS_CLIPCHILDREN,0,0,rect.getWidth(),rect.getHeight(),(HWND)parent,nullptr,ghInst,this);
  if(!window){uiDiagnostic("CreateWindowEx failed error=%lu",GetLastError());return Steinberg::kResultFalse;}
  uiDiagnostic("attached child=%p parent=%p",window,parent);firstPaint=true;
  systemWindow=parent;SetTimer(window,1,100,nullptr);
  RedrawWindow(window,nullptr,nullptr,RDW_INVALIDATE|RDW_UPDATENOW);
  return Steinberg::kResultOk;
 }
 Steinberg::tresult PLUGIN_API removed()override{if(drag>=0){int id=drag;drag=-1;if(gesture)gesture(id,false);}if(popup){DestroyWindow(popup);popup=nullptr;search=nullptr;}if(window){uiDiagnostic("removed hwnd=%p",window);KillTimer(window,1);DestroyWindow(window);window=nullptr;}systemWindow=nullptr;return Steinberg::kResultOk;}
};
#endif

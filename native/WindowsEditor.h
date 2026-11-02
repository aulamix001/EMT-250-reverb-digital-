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
extern HINSTANCE ghInst;
class WindowsEditor:public Steinberg::CPluginView,public Steinberg::IPlugViewContentScaleSupport {
 HWND window=nullptr,popup=nullptr,search=nullptr;std::function<double(int)> read;std::function<void(int,double)> write;std::function<void()> done;
 std::function<void(int,int)> resizeHost;std::string formatName;bool firstPaint=true;float scale=1.f;bool scaleRequest=false;
 inline static std::mutex classMutex;inline static int classUsers=0;
 static const char* editorClass(){static std::string s="EMT250Editor-"+std::to_string((uintptr_t)ghInst);return s.c_str();}
 static const char* presetClass(){static std::string s="EMT250Presets-"+std::to_string((uintptr_t)ghInst);return s.c_str();}
 int drag=-1,scroll=0;bool favOnly=false;std::string filter,query;std::set<int> favorites;std::vector<int> visible;std::array<double,13> clipboard{},slot{};bool copied=false,hasSlot=false;
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
 void rebuild(){visible.clear();for(int i=0;i<int(std::size(presets));++i){std::string n=name(i),q=query;std::transform(n.begin(),n.end(),n.begin(),::tolower);std::transform(q.begin(),q.end(),q.begin(),::tolower);if((filter.empty()||std::string(presets[i].name).find(filter)!=std::string::npos)&&n.find(q)!=std::string::npos&&(!favOnly||favorites.count(i)))visible.push_back(i);}scroll=std::clamp(scroll,0,std::max(0,int(visible.size())-10));}
 void paintPopup(HDC d){box(d,0,0,390,710,rgb(35));box(d,1,1,388,34,rgb(55));{Gdiplus::Graphics lettering(d);artwork->text(lettering,50,6,290,25,"EMT 250 PRESETS",10.5f,Gdiplus::Color(255,223,224,218),true,true);}text(d,366,9,"X",16);text(d,14,90,"TAGS",11);text(d,270,90,favOnly?"FAVORITES *":"FAVORITES",11);const char* ts[]={"Reverb","Best Of","Long","Short","Guitar","Vocal","Keys","Drums","Delay","Space","Echo","Chorus","Phase"};for(int i=0;i<13;++i)button(d,12+(i%3)*122,115+(i/3)*32,116,28,ts[i],filter==ts[i],2);text(d,16,289,name(current()),12);text(d,356,286,favorites.count(current())?"*":"+",20);button(d,12,314,116,28,"DELETE");button(d,136,314,116,28,"RENAME");button(d,260,314,116,28,"SAVE");box(d,8,348,374,347,rgb(21));rebuild();for(int j=scroll;j<int(visible.size())&&j<scroll+10;++j){int i=visible[j],y=352+(j-scroll)*33;if(i==current())box(d,12,y,366,32,rgb(60));text(d,22,y+10,name(i),12);if(favorites.count(i))text(d,355,y+8,"*",16);}text(d,12,698,"Roda do mouse: percorrer presets",10,rgb(135));}
 void floating(){
  if(popup){ShowWindow(popup,SW_SHOW);SetForegroundWindow(popup);return;}
  RECT r;GetWindowRect(window,&r);RECT client;GetClientRect(window,&client);
  float popScale=std::min({scale,client.right/float(W),client.bottom/float(H)});
  DWORD style=WS_POPUP|WS_BORDER|WS_CAPTION|WS_SYSMENU;
  RECT decoration{0,0,390,710};AdjustWindowRectEx(&decoration,style,FALSE,WS_EX_TOOLWINDOW);
  MONITORINFO monitor{};monitor.cbSize=sizeof(monitor);
  bool work=GetMonitorInfoA(MonitorFromWindow(window,MONITOR_DEFAULTTONEAREST),&monitor)!=FALSE;
  if(work){float width=float(monitor.rcWork.right-monitor.rcWork.left-(decoration.right-decoration.left-390)-16);float height=float(monitor.rcWork.bottom-monitor.rcWork.top-(decoration.bottom-decoration.top-710)-16);popScale=std::min({popScale,width/390,height/710});}
  if(popScale<=0)return;
  RECT size{0,0,int(std::lround(390*popScale)),int(std::lround(710*popScale))};AdjustWindowRectEx(&size,style,FALSE,WS_EX_TOOLWINDOW);
  int width=size.right-size.left,height=size.bottom-size.top,x=r.left+20,y=r.top+40;
  if(work){x=std::clamp(x,int(monitor.rcWork.left),int(monitor.rcWork.right-width));y=std::clamp(y,int(monitor.rcWork.top),int(monitor.rcWork.bottom-height));}
  popup=CreateWindowExA(WS_EX_TOOLWINDOW,presetClass(),"Presets",style,x,y,width,height,window,nullptr,ghInst,this);
  if(!popup){uiDiagnostic("preset CreateWindowEx failed error=%lu",GetLastError());return;}
  search=CreateWindowExA(WS_EX_CLIENTEDGE,"EDIT","",WS_CHILD|WS_VISIBLE|ES_AUTOHSCROLL,int(std::lround(10*popScale)),int(std::lround(42*popScale)),int(std::lround(370*popScale)),int(std::lround(34*popScale)),popup,(HMENU)1,ghInst,nullptr);ShowWindow(popup,SW_SHOW);
 }

 void move(int x,int y){
  if(drag>=2&&drag<=5){int i=drag-2,mode=int(std::round(read(1)*5));if((mode==5&&i<3)||(mode==3&&i<2)||(mode==2&&(i==1||i==2)))return;
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
 void information(){MessageBoxA(window,"EMT-250 Reverb 0.4.1 - VST3 / VST2\n\nDSP original aproximado e interface redesenhada a partir da referencia.\nSem cobranca, prazo demo ou algoritmo EMT/UAD proprietario.\nUse as funcoes de automacao/MIDI da DAW para mapear os controles.\n\nBotao direito: presets, A/B, COPY/PASTE e arquivos .a250.","EMT-250 Reverb",MB_OK|MB_ICONINFORMATION);}
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
  if(y>0&&y<286){int xs[]={240,385,531,675};for(int i=0;i<4;++i)if(std::abs(x-xs[i])<24){drag=2+i;SetCapture(window);move(x,y);}}
  if(x>525&&x<680&&y>320&&y<343){drag=7;SetCapture(window);move(x,y);}
 }
 void render(HWND h,HDC target,bool pop){
  RECT client{};GetClientRect(h,&client);if(client.right<=0||client.bottom<=0)return;
  if(!pop){std::array<double,14> values{};for(int i=0;i<14;++i)values[i]=read(i);Gdiplus::Graphics graphics(target);graphics.SetPageUnit(Gdiplus::UnitPixel);graphics.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAliasGridFit);UiArtwork::fill(graphics,0,0,float(client.right),float(client.bottom),Gdiplus::Color(255,3,3,3));float zoom=std::min(client.right/float(W),client.bottom/float(H));float x=(client.right-W*zoom)/2,y=(client.bottom-H*zoom)/2;graphics.TranslateTransform(x,y);graphics.ScaleTransform(zoom,zoom);UiArtwork::fill(graphics,0,0,958,32,Gdiplus::Color(255,36,36,36));
   auto label=[&](float x,float w,const std::string&value){artwork->text(graphics,x,3,w,25,value,11,Gdiplus::Color(255,224,224,215));};
   label(0,28,"<");label(28,28,">");label(56,344,name(current())+"  v");label(400,45,values[10]<.5?"IN":"OFF");label(445,35,aSelected?"[A]":"A");label(480,35,aSelected?"B":"[B]");label(515,65,"COPY");label(580,70,"PASTE");label(680,190,"0.4.1  "+formatName);label(880,78,"INFO");
   graphics.TranslateTransform(0,32);artwork->prepareBackground(int(std::lround(W*zoom)),int(std::lround(371*zoom)));skin.draw(*artwork,graphics,values);lastPaint=values;hasPaint=true;return;}
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
 static LRESULT CALLBACK proc(HWND h,UINT m,WPARAM w,LPARAM l){auto*self=(WindowsEditor*)GetWindowLongPtrA(h,0);if(m==WM_NCCREATE){auto*create=(CREATESTRUCTA*)l;self=(WindowsEditor*)create->lpCreateParams;if(!self)return FALSE;SetWindowLongPtrA(h,0,(LONG_PTR)self);SetWindowLongPtrA(h,sizeof(LONG_PTR),strcmp(create->lpszClass,presetClass())==0);uiDiagnostic("WM_NCCREATE hwnd=%p parent=%p instance=%p",h,create->hwndParent,create->hInstance);return TRUE;}if(!self)return DefWindowProcA(h,m,w,l);bool pop=GetWindowLongPtrA(h,sizeof(LONG_PTR))!=0;if(m==WM_NCDESTROY){SetWindowLongPtrA(h,0,0);if(h==self->window)self->window=nullptr;if(h==self->popup){self->popup=nullptr;self->search=nullptr;}return DefWindowProcA(h,m,w,l);}if(m==WM_COMMAND&&!pop&&LOWORD(w)>=1&&LOWORD(w)<=11){self->command(LOWORD(w));return 0;}if(m==WM_CONTEXTMENU&&!pop){self->context(GET_X_LPARAM(l),GET_Y_LPARAM(l));return 0;}if(m==WM_PRINTCLIENT){self->render(h,(HDC)w,pop);return 0;}if(m==WM_ERASEBKGND)return 1;if(m==WM_PAINT){if(!pop&&self->firstPaint){uiDiagnostic("first WM_PAINT hwnd=%p",h);self->firstPaint=false;}PAINTSTRUCT ps;HDC d=BeginPaint(h,&ps);self->render(h,d,pop);EndPaint(h,&ps);return 0;}if(m==0x02E3&&!pop&&self->formatName=="VST2"){self->setContentScaleFactor(nativeDpiScale(h));return 0;}if(m==WM_TIMER){bool dirty=!self->hasPaint;for(int i=0;i<13;++i)if(self->read(i)!=self->lastPaint[i])dirty=true;auto level=[](double meter){return meter>=1?4:meter>=.5?3:meter>=.25?2:meter>=.125?1:0;};if(level(self->read(13))!=level(self->lastPaint[13]))dirty=true;if(dirty)InvalidateRect(h,nullptr,FALSE);return 0;}if(pop){if(m==WM_CLOSE){ShowWindow(h,SW_HIDE);return 0;}if(m==WM_COMMAND&&HIWORD(w)==EN_CHANGE){char b[256]{};GetWindowTextA((HWND)l,b,256);self->query=b;self->scroll=0;InvalidateRect(h,nullptr,FALSE);return 0;}if(m==WM_MOUSEWHEEL){self->scroll+=GET_WHEEL_DELTA_WPARAM(w)>0?-2:2;InvalidateRect(h,nullptr,FALSE);return 0;}if(m==WM_LBUTTONDOWN){POINT point=logicalPoint(h,l,true);int x=point.x,y=point.y;if(y<35&&x>350)ShowWindow(h,SW_HIDE);else if(y<35)SendMessage(h,WM_NCLBUTTONDOWN,HTCAPTION,0);else if(y>80&&y<108){self->favOnly=!self->favOnly;}else if(y>=115&&y<275){int i=(y-115)/32*3+(x-12)/122;const char*ts[]={"Reverb","Best Of","Long","Short","Guitar","Vocal","Keys","Drums","Delay","Space","Echo","Chorus","Phase"};if(i>=0&&i<13){self->filter=self->filter==ts[i]?"":ts[i];self->scroll=0;}}else if(y>280&&y<315&&x>340){int i=self->current();if(self->favorites.count(i))self->favorites.erase(i);else self->favorites.insert(i);}else if(y>=314&&y<343){if(x>260)self->saveFile();else MessageBoxA(h,"Presets de fabrica sao protegidos. Use SAVE para gravar uma copia .a250.","Presets",MB_OK);}else if(y>=352&&y<683){self->rebuild();int i=(y-352)/33+self->scroll;if(i<int(self->visible.size()))self->preset(self->visible[i]);}InvalidateRect(h,nullptr,FALSE);return 0;}}else{if(m==WM_LBUTTONDOWN){POINT point=logicalPoint(h,l,false);self->click(point.x,point.y);InvalidateRect(h,nullptr,FALSE);return 0;}if(m==WM_MOUSEMOVE&&self->drag>=0){POINT point=logicalPoint(h,l,false);self->move(point.x,point.y);return 0;}if(m==WM_LBUTTONUP){self->drag=-1;ReleaseCapture();return 0;}}return DefWindowProcA(h,m,w,l);}
public:
 WindowsEditor(std::function<double(int)>r,std::function<void(int,double)>w,std::function<void()>finish,std::string format="VST3",std::function<void(int,int)>resize={}):read(r),write(w),done(finish),resizeHost(std::move(resize)),formatName(std::move(format)){
  rect={0,0,W,H};artwork=std::make_unique<UiArtwork>();
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
  scale=factor;if(popup){DestroyWindow(popup);popup=nullptr;search=nullptr;}
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
 Steinberg::tresult PLUGIN_API removed()override{if(popup){DestroyWindow(popup);popup=nullptr;search=nullptr;}if(window){uiDiagnostic("removed hwnd=%p",window);KillTimer(window,1);DestroyWindow(window);window=nullptr;}systemWindow=nullptr;return Steinberg::kResultOk;}
};
#endif

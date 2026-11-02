#pragma once
#ifdef _WIN32
#include <windows.h>
#include <gdiplus.h>
#include <objidl.h>
#include <memory>
#include <mutex>
#include <vector>
#include <string>
extern HINSTANCE ghInst;
class UiArtwork {
 struct Image {
  IStream* stream=nullptr;std::unique_ptr<Gdiplus::Bitmap> bitmap;
  void load(int id){auto resource=FindResourceA(ghInst,MAKEINTRESOURCEA(id),MAKEINTRESOURCEA(10));if(!resource)return;DWORD size=SizeofResource(ghInst,resource);auto source=LoadResource(ghInst,resource);HGLOBAL data=GlobalAlloc(GMEM_MOVEABLE,size);if(!data)return;void* bytes=GlobalLock(data);if(!bytes){GlobalFree(data);return;}memcpy(bytes,LockResource(source),size);GlobalUnlock(data);if(CreateStreamOnHGlobal(data,TRUE,&stream)!=S_OK){GlobalFree(data);return;}bitmap.reset(Gdiplus::Bitmap::FromStream(stream));if(bitmap->GetLastStatus()!=Gdiplus::Ok)bitmap.reset();}
  ~Image(){bitmap.reset();if(stream)stream->Release();}
 };
 inline static std::mutex mutex;
 inline static int users=0;
 inline static ULONG_PTR token=0;
 struct Session {
  Session(){std::lock_guard<std::mutex> lock(mutex);if(users++==0){Gdiplus::GdiplusStartupInput input;Gdiplus::GdiplusStartup(&token,&input,nullptr);}}
  ~Session(){std::lock_guard<std::mutex> lock(mutex);if(--users==0&&token){Gdiplus::GdiplusShutdown(token);token=0;}}
 } session;
public:
 Image chassis,caps,bellows;
 Gdiplus::PrivateFontCollection fonts;
 std::unique_ptr<Gdiplus::FontFamily> lettering;
 std::unique_ptr<Gdiplus::Bitmap> frame;
 std::unique_ptr<Gdiplus::Bitmap> scaledBackground;int cachedWidth=0,cachedHeight=0;
 UiArtwork(){
  chassis.load(1001);caps.load(1002);bellows.load(1004);
  auto resource=FindResourceA(ghInst,MAKEINTRESOURCEA(1003),MAKEINTRESOURCEA(10));if(resource){auto data=LoadResource(ghInst,resource);fonts.AddMemoryFont(LockResource(data),SizeofResource(ghInst,resource));lettering=std::make_unique<Gdiplus::FontFamily>(L"Michroma",&fonts);if(lettering->GetLastStatus()!=Gdiplus::Ok)lettering.reset();}
 }
 ~UiArtwork()=default;
 Gdiplus::Bitmap* frameBuffer(int width,int height){
  if(!frame||int(frame->GetWidth())!=width||int(frame->GetHeight())!=height)frame=std::make_unique<Gdiplus::Bitmap>(width,height,PixelFormat32bppPARGB);
  if(frame->GetLastStatus()!=Gdiplus::Ok)return nullptr;frame->SetResolution(96.f,96.f);return frame.get();
 }
 void prepareBackground(int width,int height){
  if(width==cachedWidth&&height==cachedHeight)return;
  scaledBackground.reset();cachedWidth=width;cachedHeight=height;
  if(!chassis.bitmap)return;
  auto image=std::make_unique<Gdiplus::Bitmap>(width,height,PixelFormat32bppPARGB);
  if(image->GetLastStatus()!=Gdiplus::Ok)return;
  {Gdiplus::Graphics graphics(image.get());graphics.SetPageUnit(Gdiplus::UnitPixel);graphics.ScaleTransform(width/958.f,height/371.f);graphics.Clear(Gdiplus::Color(255,3,3,3));graphics.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
   // Measured registration to the latest 958 x 371 reference, rather than a free stretch.
   graphics.SetClip(Gdiplus::RectF(0,0,844,371));graphics.DrawImage(chassis.bitmap.get(),Gdiplus::RectF(-5.7f,-6.8f,958*1.008f,371*1.015f));graphics.ResetClip();
   float sx=chassis.bitmap->GetWidth()/958.f,sy=chassis.bitmap->GetHeight()/371.f;
   graphics.DrawImage(chassis.bitmap.get(),Gdiplus::RectF(849,0,109,371),844*sx,0.f,114*sx,371*sy,Gdiplus::UnitPixel);
   graphics.DrawImage(chassis.bitmap.get(),Gdiplus::RectF(37.6f,212.4f,104.4f,125),45.34f*sx,213.6f*sy,115.1f*sx,123.15f*sy,Gdiplus::UnitPixel);
  }
  scaledBackground=std::move(image);
 }
 Gdiplus::Bitmap* background(){return scaledBackground?scaledBackground.get():chassis.bitmap.get();}
 Gdiplus::Bitmap* handles(){return caps.bitmap.get();}
 Gdiplus::Bitmap* rubber(){return bellows.bitmap.get();}
 static Gdiplus::Color color(int r,int g,int b,int a=255){return Gdiplus::Color(BYTE(a),BYTE(r),BYTE(g),BYTE(b));}
 static void fill(Gdiplus::Graphics&g,float x,float y,float w,float h,Gdiplus::Color c){Gdiplus::SolidBrush brush(c);g.FillRectangle(&brush,x,y,w,h);}
 static std::wstring wide(const std::string&s){if(s.empty())return {};int n=MultiByteToWideChar(CP_UTF8,0,s.data(),int(s.size()),nullptr,0);std::wstring v(n,0);MultiByteToWideChar(CP_UTF8,0,s.data(),int(s.size()),v.data(),n);return v;}
 void text(Gdiplus::Graphics&g,float x,float y,float w,float h,const std::string&s,float size,Gdiplus::Color c,bool center=true,bool panel=false){
  auto value=wide(s);Gdiplus::FontFamily system(L"Arial");auto*family=panel&&lettering?lettering.get():system.GetLastStatus()==Gdiplus::Ok?&system:lettering?lettering.get():Gdiplus::FontFamily::GenericSansSerif();
  Gdiplus::Font font(family,size,(family==lettering.get()||!family->IsStyleAvailable(Gdiplus::FontStyleBold))?Gdiplus::FontStyleRegular:Gdiplus::FontStyleBold,Gdiplus::UnitPixel);
  Gdiplus::StringFormat format;format.SetAlignment(center?Gdiplus::StringAlignmentCenter:Gdiplus::StringAlignmentNear);format.SetLineAlignment(Gdiplus::StringAlignmentCenter);format.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
  Gdiplus::SolidBrush brush(c);g.DrawString(value.c_str(),int(value.size()),&font,Gdiplus::RectF(x,y,w,h),&format,&brush);
 }
};
#endif

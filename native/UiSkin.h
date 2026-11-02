#pragma once
#include "UiArtwork.h"
#include <array>
#include <cmath>
struct UiSkin {
 using G=Gdiplus::Graphics;using C=Gdiplus::Color;using R=Gdiplus::RectF;
 static constexpr float W=958,H=371;
 static void line(G&g,float x,float y,float xx,float yy,C color,float width=1){Gdiplus::Pen p(color,width);g.DrawLine(&p,x,y,xx,yy);}
 static void gradient(G&g,R r,C a,C b,bool vertical=true){Gdiplus::LinearGradientBrush brush(Gdiplus::PointF(r.X,r.Y),Gdiplus::PointF(r.X+(vertical?0:r.Width),r.Y+(vertical?r.Height:0)),a,b);g.FillRectangle(&brush,r);}
 static void ellipse(G&g,R r,C color){Gdiplus::SolidBrush brush(color);g.FillEllipse(&brush,r);}
 static void bulb(G&g,float x,float y,int color,bool on){
  C hue=color==0?C(255,255,25,20):color==1?C(255,24,255,50):C(255,255,245,12);
  if(on){for(int radius=9;radius>=5;--radius)ellipse(g,R(x-radius,y-radius,radius*2.f,radius*2.f),C(BYTE((10-radius)*3),hue.GetR(),hue.GetG(),hue.GetB()));}
  ellipse(g,R(x-5.4f,y-5.4f,10.8f,10.8f),C(255,12,13,12));
  Gdiplus::Pen rim(C(255,65,67,60),.75f);g.DrawEllipse(&rim,R(x-4.8f,y-4.8f,9.6f,9.6f));
  ellipse(g,R(x-3.7f,y-3.7f,7.4f,7.4f),on?hue:color==0?C(255,33,12,10):color==1?C(255,9,80,18):C(255,36,34,9));
  ellipse(g,R(x-2.1f,y-2.5f,2.3f,1.6f),on?C(255,255,255,207):C(150,173,181,166));
  if(on)ellipse(g,R(x-1.6f,y-1.5f,3.5f,3.1f),color==0?C(255,255,117,75):color==1?C(255,178,255,165):C(255,255,255,177));
 }
 static void led(G&g,float x,float y,int color,bool on,float zoom=.8f){auto state=g.Save();g.TranslateTransform(x,y);g.ScaleTransform(zoom,zoom);bulb(g,0,0,color,on);g.Restore(state);}
 static void stem(G&g,float x,float y,float anchor=248){
  float from=y<anchor?y+18:y-18,to=anchor;
  float top=std::min(from,to),bottom=std::max(from,to);
  gradient(g,R(x-13,top,26,bottom-top),C(255,5,5,5),C(255,21,22,20),false);
  for(float yy=top;yy<bottom;yy+=3.2f){ellipse(g,R(x-14,yy-2,28,6),C(255,7,8,7));Gdiplus::Pen edge(C(200,57,59,51),.9f);g.DrawArc(&edge,R(x-14,yy-2,28,6),8,166);}
 }
 static void thumb(G&g,float mix){
  float x=647+std::clamp(mix,0.f,1.f)*135;
  ellipse(g,R(x-4,455,62,10),C(150,0,0,0));
  gradient(g,R(x,449,54,15),C(255,68,70,67),C(255,7,7,7));
  gradient(g,R(x+3,450,48,3),C(255,126,127,118),C(255,43,44,41),false);
  line(g,x+3,451,x+3,460,C(255,163,164,153),1);line(g,x+51,451,x+51,461,C(255,5,6,5),2);
 }
 static void toggle(G&g,bool rear){
  ellipse(g,R(594,443,27,28),C(150,0,0,0));
  gradient(g,R(597,445,22,22),C(255,205,209,196),C(255,60,62,57));
  ellipse(g,R(598,446,20,20),C(255,224,224,207));ellipse(g,R(600,448,16,16),C(255,31,33,31));ellipse(g,R(602,450,12,12),C(255,211,163,46));
  float endY=rear?468:445;
  Gdiplus::Pen dark(C(255,25,27,24),8);dark.SetStartCap(Gdiplus::LineCapRound);dark.SetEndCap(Gdiplus::LineCapRound);g.DrawLine(&dark,608.f,456.f,605.f,endY);
  Gdiplus::Pen chrome(C(255,196,200,188),5);chrome.SetStartCap(Gdiplus::LineCapRound);chrome.SetEndCap(Gdiplus::LineCapRound);g.DrawLine(&chrome,607.f,454.f,604.f,endY-1);
  line(g,605,453,602,endY,C(255,247,247,235),1.2f);
 }
 void drawCap(UiArtwork&a,G&g,float x,float y,bool ivory){
  ellipse(g,R(x-21,y+13,47,9),C(100,0,0,0));
  if(auto image=a.handles())g.DrawImage(image,R(x-24,y-20,48,40),ivory?72.f:616.f,104.f,336.f,304.f,Gdiplus::UnitPixel);
 }
 void draw(UiArtwork&a,G&g,const std::array<double,14>&v){
  g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHighQuality);
  if(auto image=a.background())g.DrawImage(image,R(0,0,958,371));
  else UiArtwork::fill(g,0,0,958,371,C(255,28,28,28));
  int mode=int(std::round(v[1]*5));int f[4];for(int i=0;i<4;++i)f[i]=int(std::round(v[i+2]*(i==0?15:3)));
  for(int i=0;i<16;++i){float y=249-i*14.8f;led(g,204,y,0,mode==0&&f[0]==i);led(g,274,y,1,(mode==1||mode==2||mode==4)&&f[0]==i);}
  for(int i=0;i<4;++i){float y=249-i*58.2f;led(g,349,y,0,mode==0&&f[1]==i);led(g,419,y,1,(mode==1||mode==4)&&f[1]==i);led(g,494,y,0,(mode==0||mode==4)&&f[2]==i);led(g,564,y,1,(mode==1||mode==3)&&f[2]==i);led(g,710,y,1,f[3]==i);}
  const float xs[]={240,385,531,675},bases[]={239,253,225,225},travel[]={222,174,174,174};
  for(int i=0;i<4;++i){float y=bases[i]-float(v[i+2])*travel[i];
   float edge=y<142?y+17:y-17,top=std::min(edge,142.f),height=std::abs(edge-142.f);
   if(auto rubber=a.rubber()){if(height>0)g.DrawImage(rubber,R(xs[i]-11,top,22,height),20.f,40.f,216.f,688.f,Gdiplus::UnitPixel);}
   else{auto transform=g.Save();g.TranslateTransform(xs[i],0);g.ScaleTransform(.8f,.8f);stem(g,0,y/.8f,142/.8f);g.Restore(transform);}
   drawCap(a,g,xs[i],y,i==0);
  }
  for(int i=0;i<4;++i)led(g,50,226+i*20,i==0?0:1,v[13]>=(i==0?1.:i==1?.5:i==2?.25:.125),1.f);
  led(g,50,322,2,v[9]>.5,1.f);
  // A compact chrome lever, laid over the stationary socket in the plate.
  ellipse(g,R(483,319,21,21),C(255,203,204,176));ellipse(g,R(486,322,15,15),C(255,59,65,38));ellipse(g,R(488,324,11,11),C(255,219,206,63));
  float tip=v[6]>.5?338:320;
  Gdiplus::Pen shaft(C(255,202,205,186),4.2f);shaft.SetStartCap(Gdiplus::LineCapRound);shaft.SetEndCap(Gdiplus::LineCapRound);g.DrawLine(&shaft,493.f,329.f,491.f,tip);line(g,491,327,490,tip,C(255,255,255,239),1.2f);
  float x=526+float(v[7])*107;ellipse(g,R(x-2,331,49,7),C(140,0,0,0));gradient(g,R(x,326,44,12),C(255,85,87,83),C(255,8,9,8));line(g,x+3,327,x+40,327,C(255,114,117,110),1);line(g,x+3,328,x+3,335,C(255,169,169,155),1);
  float bx[]={192,297,329,361,393,425};
  for(int i=0;i<6;++i)if(mode==i){Gdiplus::Pen rim(i==0?C(170,255,64,33):C(160,88,255,59),.9f);g.DrawRectangle(&rim,R(bx[i]+2,309,26,29));}
  if(v[8]<.5)UiArtwork::fill(g,702,312,24,24,C(155,29,24,4));
  if(v[10]>.5)UiArtwork::fill(g,42,20,24,30,C(150,0,0,0));
 }
};

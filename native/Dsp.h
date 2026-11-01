#pragma once
#include <array>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>
struct Controls { int mode=0; std::array<int,4> f{10,1,2,2}; bool rear=false,solo=true,noise=true,bypass=false; double mix=.5,left=200,right=200; };
class Engine {
 struct Line {std::vector<double> b;size_t pos=0;double low=0;void init(int n){b.assign(n,0);pos=0;low=0;} double tap(double samples)const{double index=double(pos)-std::max(1.,samples);while(index<0)index+=b.size();auto i=size_t(index)%b.size();double t=index-std::floor(index);return b[i]*(1-t)+b[(i+1)%b.size()]*t;} void put(double x){b[pos]=x;pos=(pos+1)%b.size();}};
 std::array<Line,8> lines;Line pre,echo,source;double sr=48000,phase=0;uint32_t random=1234567;std::array<double,4> drift{},target{};int driftCount=0;
 double rnd(){random^=random<<13;random^=random>>17;random^=random<<5;return double(random)/4294967295.*2-1;}
public:
 void prepare(double rate){sr=rate;for(auto&l:lines)l.init(int(sr*1.1));pre.init(int(sr));echo.init(int(sr));source.init(int(sr));phase=0;drift={};target={};driftCount=0;}
 std::array<double,2> tick(double l,double r,const Controls& c){
  if(c.bypass)return {l,r};
  double mono=(l+r)*.5;if(c.mode==4&&c.rear)mono=0;

  const double pd=c.f[3]*.02*sr;double x=pd>0?pre.tap(pd):mono;pre.put(mono);double a=0,b=0;
  if(c.mode==1){source.put(x);a=source.tap(std::max(1.,c.left*sr/1000));b=source.tap(std::max(1.,c.right*sr/1000)); // replaced below with milliseconds
  }else if(c.mode==4){double ms=c.f[0]*20+c.f[1]*5;double y=echo.tap(std::max(1.,ms*sr/1000));double hf[]={.25,.33,.5,1};echo.low+=hf[c.f[2]]*(y-echo.low);echo.put(x+.9*echo.low);a=b=y;
  }else if(c.mode==2){double shift=c.f[0]*sr/1000.;double fixed=source.tap(.015*sr),variable=shift>0?source.tap(shift):x;source.put(x);if(c.rear){a=(fixed-variable)*.5;b=(fixed+variable)*.5;if(c.f[0]==0)a=0;if(c.f[0]==15)b=0;}else{a=(fixed+variable)*.5;b=(fixed-variable)*.5;}
  }else if(c.mode==3){
   if(--driftCount<=0){for(auto&t:target)t=rnd();driftCount=int(sr*.4);}double v[4];for(int i=0;i<4;++i){drift[i]+=(target[i]-drift[i])*.00005;v[i]=source.tap(sr*(.014+i*.003+drift[i]*.003));}source.put(x);switch(c.f[2]){case 0:a=v[0];b=v[1];break;case 1:a=v[2];b=v[3];break;case 2:a=(v[0]+v[2])*.5;b=(v[1]+v[3])*.5;break;default:a=(v[0]+v[2]+v[3])/3;b=(v[2]+v[1]-v[3])/3;}
  }else{
   static const double times[]={.0297,.0371,.0411,.0437,.0531,.0617,.0719,.0793};static const double decays[]={.4,.6,.8,1,1.2,1.5,1.8,2,2.2,2.5,2.8,3,3.3,3.6,4,4.5};double lf[]={.5,1,1.5,2},hf[]={.25,.33,.5,1};double decay=c.mode==5?10:decays[c.f[0]];for(int i=0;i<8;++i){auto&line=lines[i];double y=line.tap(times[i]*sr);line.low+=.06*(y-line.low);double loGain=std::pow(.001,times[i]/(decay*(c.mode==5?1:lf[c.f[1]])));double hiGain=std::pow(.001,times[i]/(decay*(c.mode==5?1:hf[c.f[2]])));line.put(x*.22+line.low*loGain+(y-line.low)*hiGain);if(c.rear){a+=y*((i%3==0)?-.25:.25);b+=y*((i%3==1)?-.25:.25);}else{if(i%2)a+=y*.35;else b+=y*.35;}}
  }
  if((c.mode==1||c.mode==3)&&c.rear)a=b=(a+b)*.5;
  if(c.noise){double n=rnd()*1e-5*std::min(1.,std::abs(a)+std::abs(b));a+=n;b-=n;}
  double wet=c.solo?1:std::expm1(2*std::log(17./3)*c.mix)/(std::pow(17./3,2)-1);return {l*(1-wet)+a*wet,r*(1-wet)+b*wet};
 }
};

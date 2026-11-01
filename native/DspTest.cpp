#include "Dsp.h"
#include <iostream>
#include <stdexcept>
int main(){for(int mode=0;mode<6;++mode){Engine engine;engine.prepare(48000);Controls c;c.mode=mode;c.noise=false;c.left=20;c.right=40;double energy=0;for(int i=0;i<48000*2;++i){auto y=engine.tick(i==0?1:0,i==0?1:0,c);for(auto v:y){if(!std::isfinite(v)||std::abs(v)>10)throw std::runtime_error("unstable");energy+=v*v;}}if(energy<=1e-9)throw std::runtime_error("no wet output");}Engine e;e.prepare(48000);Controls c;c.bypass=true;auto y=e.tick(.25,-.5,c);if(y[0]!=.25||y[1]!=-.5)throw std::runtime_error("bypass");std::cout<<"6 modes: impulse output finite and nonzero; stereo bypass exact\n";}

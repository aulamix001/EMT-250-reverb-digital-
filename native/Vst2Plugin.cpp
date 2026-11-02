#include "Vst2Abi.h"
#include "Dsp.h"
#include "Presets.h"
#include "WindowsEditor.h"
#include <atomic>
#include <cstring>
#include <cstdio>
#include <memory>
#include <string>
HINSTANCE ghInst = nullptr;
BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, LPVOID) {
    if(reason==DLL_PROCESS_ATTACH) ghInst=instance;
    return TRUE;
}
namespace {
constexpr int count = 13;
class Vst2Plugin {
public:
    vst2::Effect effect{};
    vst2::Callback host;
    std::array<std::atomic<double>,count> parameters{};
    std::atomic<double> meter{0};
    Engine engine;
    double sampleRate=48000;float hostScale=0;
    std::unique_ptr<WindowsEditor> editor;
    vst2::Rect bounds{0,0,403,958};
    std::array<double,count> chunk{};
    static Vst2Plugin& self(vst2::Effect* e) {return *static_cast<Vst2Plugin*>(e->object);}
    explicit Vst2Plugin(vst2::Callback callback):host(callback) {
        effect.magic=vst2::magic;effect.dispatcher=dispatch;effect.process=processAccumulating;
        effect.setParameter=setParameter;effect.getParameter=getParameter;
        effect.numPrograms=int(std::size(presets));effect.numParams=count;
        effect.numInputs=effect.numOutputs=2;
        effect.flags=1|16|32; // editor, replacing processor, state chunks
        effect.object=this;effect.uniqueID=0x41323530;effect.version=402;
        effect.processReplacing=process;
        parameters[8]=1;parameters[9]=1;
        int index=0;
        for(int i=0;i<int(std::size(presets));++i)if(std::string(presets[i].name).find("Default [")==0)index=i;
        preset(index);engine.prepare(sampleRate);setBounds(nativeDpiScale());
    }
    void setBounds(float factor){bounds.right=std::int16_t(std::lround(958*factor));bounds.bottom=std::int16_t(std::lround(403*factor));}
    int program()const {return std::clamp(int(std::round(parameters[0].load()*(std::size(presets)-1))),0,int(std::size(presets))-1);}
    void preset(int index) {
        index=std::clamp(index,0,int(std::size(presets))-1);
        const auto& p=presets[index];
        parameters[0]=index/double(std::size(presets)-1);parameters[1]=p.mode/5.;
        for(int i=0;i<4;++i)parameters[2+i]=p.f[i]/double(i==0?15:3);
        parameters[6]=p.rear;parameters[7]=p.mix;parameters[11]=p.left/315.;parameters[12]=p.right/315.;
    }
    void set(int id,double value) {
        if(id<0||id>=count||!std::isfinite(value))return;
        parameters[id]=std::clamp(value,0.,1.);
        if(id==0){preset(program());return;}
        if(int(std::round(parameters[1].load()*5))==1) {
            int channel=parameters[4].load()<.5?11:12;
            if(id==2||id==3)parameters[channel]=(std::round(parameters[2].load()*15)*20+std::round(parameters[3].load()*3)*5)/315.;
            if(id==4){int ms=int(std::round(parameters[channel].load()*63))*5;parameters[2]=(ms/20)/15.;parameters[3]=((ms%20)/5)/3.;}
        }
    }
    void edit(int id,double value) {
        if(host)host(&effect,vst2::BeginEdit,id,0,nullptr,0);
        set(id,value);
        if(host){host(&effect,vst2::Automate,id,0,nullptr,float(parameters[id].load()));host(&effect,vst2::EndEdit,id,0,nullptr,0);}
    }
    Controls controls()const {
        Controls c;c.mode=int(std::round(parameters[1].load()*5));
        for(int i=0;i<4;++i)c.f[i]=int(std::round(parameters[i+2].load()*(i==0?15:3)));
        c.rear=parameters[6].load()>.5;c.mix=parameters[7].load();c.solo=parameters[8].load()>.5;
        c.noise=parameters[9].load()>.5;c.bypass=parameters[10].load()>.5;
        c.left=std::round(parameters[11].load()*63)*5;c.right=std::round(parameters[12].load()*63)*5;return c;
    }
    static void setParameter(vst2::Effect*e,vst2::Int id,float value){self(e).set(id,value);}
    static float getParameter(vst2::Effect*e,vst2::Int id){return id>=0&&id<count?float(self(e).parameters[id].load()):0;}
    static void processBlock(vst2::Effect*e,float**inputs,float**outputs,vst2::Int samples,bool accumulate) {
        if(!inputs||!outputs||!inputs[0]||!inputs[1]||!outputs[0]||!outputs[1]||samples<0)return;
        auto& p=self(e);auto c=p.controls();double peak=0;
        for(int i=0;i<samples;++i){double l=inputs[0][i],r=inputs[1][i];peak=std::max(peak,std::max(std::abs(l),std::abs(r)));auto y=p.engine.tick(l,r,c);if(accumulate){outputs[0][i]+=float(y[0]);outputs[1][i]+=float(y[1]);}else{outputs[0][i]=float(y[0]);outputs[1][i]=float(y[1]);}}
        p.meter=peak;
    }
    static void process(vst2::Effect*e,float**inputs,float**outputs,vst2::Int samples){processBlock(e,inputs,outputs,samples,false);}
    static void processAccumulating(vst2::Effect*e,float**inputs,float**outputs,vst2::Int samples){processBlock(e,inputs,outputs,samples,true);}
    static vst2::Ptr dispatch(vst2::Effect*e,vst2::Int op,vst2::Int index,vst2::Ptr value,void*ptr,float opt) {
        auto& p=self(e);
        auto text=[&](const char*s,int capacity){if(!ptr)return vst2::Ptr(0);std::snprintf(static_cast<char*>(ptr),capacity,"%s",s);return vst2::Ptr(1);};
        switch(op) {
        case vst2::Open:return 1;
        case vst2::Close:delete &p;return 1;
        case vst2::SetProgram:p.preset(int(value));return 1;
        case vst2::GetProgram:return p.program();
        case vst2::GetProgramName:return text(presets[p.program()].name,24);
        case vst2::GetProgramNameIndexed:return index>=0&&index<int(std::size(presets))?text(presets[index].name,24):0;
        case vst2::GetParamName:{const char*names[]={"Preset","Mode","Fader 1","Fader 2","Fader 3","Predelay","Rear","Dry Wet","Wet Solo","Noise","Bypass","Delay L","Delay R"};return index>=0&&index<count?text(names[index],8):0;}
        case vst2::GetParamLabel:return text("",8);
        case vst2::GetParamDisplay:if(index>=0&&index<count&&ptr){std::snprintf(static_cast<char*>(ptr),8,"%.3f",p.parameters[index].load());return 1;}return 0;
        case vst2::SetSampleRate:if(std::isfinite(opt)&&opt>=8000&&opt<=384000){p.sampleRate=opt;p.engine.prepare(opt);return 1;}return 0;
        case vst2::SetBlockSize:return 1;
        case vst2::MainsChanged:if(value)p.engine.prepare(p.sampleRate);return 1;
        case vst2::StartProcess:case vst2::StopProcess:return 1;
        case vst2::SetProcessPrecision:return value==0;
        case vst2::SetBypass:p.parameters[10]=value!=0;return 1;
        case vst2::EditGetRect:if(ptr){*static_cast<vst2::Rect**>(ptr)=&p.bounds;return 1;}return 0;
        case vst2::EditOpen:{
            p.editor.reset();
            auto view=std::make_unique<WindowsEditor>([&p](int id){return id==13?p.meter.load():p.parameters[id].load();},[&p](int id,double v){p.edit(id,v);},[](){},"VST2",[&p](int width,int height){p.bounds.right=std::int16_t(width);p.bounds.bottom=std::int16_t(height);if(p.host)p.host(&p.effect,vst2::SizeWindow,width,height,nullptr,0);});
            float factor=p.hostScale>0?p.hostScale:nativeDpiScale((HWND)ptr);view->setContentScaleFactor(factor);p.setBounds(factor);
            if(view->attached(ptr,Steinberg::kPlatformTypeHWND)!=Steinberg::kResultOk)return 0;
            p.editor=std::move(view);if(p.host)p.host(e,vst2::SizeWindow,p.bounds.right,p.bounds.bottom,nullptr,0);return 1;}
        case vst2::EditClose:p.editor.reset();return 1;
        case vst2::EditIdle:return 1;
        case vst2::GetChunk:if(ptr){for(int i=0;i<count;++i)p.chunk[i]=p.parameters[i].load();*static_cast<void**>(ptr)=p.chunk.data();return sizeof(p.chunk);}return 0;
        case vst2::SetChunk:if(ptr&&value==sizeof(p.chunk)){std::array<double,count> next;std::memcpy(next.data(),ptr,sizeof(next));for(double v:next)if(!std::isfinite(v)||v<0||v>1)return 0;for(int i=0;i<count;++i)p.parameters[i]=next[i];return 1;}return 0;
        case vst2::GetCategory:return 1;
        case vst2::GetEffectName:case vst2::GetProductString:return text("EMT-250 Reverb",64);
        case vst2::GetVendorString:return text("Aulamix",64);
        case vst2::GetVendorVersion:return 402;
        case vst2::GetVstVersion:return 2400;
        case vst2::GetTailSize:return vst2::Ptr(p.sampleRate*30);
        case 50:if(index==0x50726553&&value==0x41654373&&std::isfinite(opt)&&opt>0&&opt<32767.f/958){p.hostScale=opt;p.setBounds(opt);if(p.editor)return p.editor->setContentScaleFactor(opt)==Steinberg::kResultOk;return 1;}return 0;
        case vst2::CanDo:if(ptr&&(std::strcmp(static_cast<const char*>(ptr),"bypass")==0||std::strcmp(static_cast<const char*>(ptr),"supportsViewDpiScaling")==0))return 1;return 0;
        default:return 0;
        }
    }
};
}
extern "C" __declspec(dllexport) vst2::Effect* VSTPluginMain(vst2::Callback host) {
    if(!host||!host(nullptr,vst2::HostVersion,0,0,nullptr,0))return nullptr;
    try{return &(new Vst2Plugin(host))->effect;}catch(...){return nullptr;}
}

#include "public.sdk/source/vst/vstsinglecomponenteffect.h"
#include "public.sdk/source/main/pluginfactory.h"
#include "pluginterfaces/vst/ivstparameterchanges.h"
#include "pluginterfaces/base/ibstream.h"
#include "base/source/fstreamer.h"
#include "Dsp.h"
#include "Presets.h"
#include "pluginterfaces/base/ustring.h"
#include <string>
#include <atomic>
#include "WindowsEditor.h"
using namespace Steinberg;using namespace Steinberg::Vst;
static const FUID uid(0x8D1B47E3,0xA2204751,0x9F441FE8,0x25000001);
class Plugin:public SingleComponentEffect {
 std::atomic<double> meter{0};std::array<std::atomic<double>,13> ui{};std::array<std::atomic<bool>,13> pending{},gestures{};
 Engine engine;Controls controls;std::array<double,13> values{0,0,10./15,1./3,2./3,2./3,0,.5,1,1,0,200./315,200./315};
 void load(int index){auto&p=presets[std::clamp(index,0,int(std::size(presets))-1)];values[1]=p.mode/5.;for(int i=0;i<4;++i)values[2+i]=p.f[i]/double(i==0?15:3);values[6]=p.rear;values[7]=p.mix;values[11]=p.left/315;values[12]=p.right/315;}
 void sync(){controls.mode=int(std::round(values[1]*5));for(int i=0;i<4;++i)controls.f[i]=int(std::round(values[i+2]*(i==0?15:3)));controls.rear=values[6]>.5;controls.mix=values[7];controls.solo=values[8]>.5;controls.noise=values[9]>.5;controls.bypass=values[10]>.5;controls.left=std::round(values[11]*63)*5;controls.right=std::round(values[12]*63)*5;}
public:
 IPlugView* PLUGIN_API createView(FIDString type)override{
#ifdef _WIN32
 uiDiagnostic("createView type=%s",type?type:"(null)");
 if(type&&strcmp(type,ViewType::kEditor)==0){addRef();return new WindowsEditor([this](int i){return i==13?meter.load():ui[i].load();},[this](int i,double v){v=std::clamp(v,0.,1.);bool discrete=!gestures[i].load();if(discrete)beginEdit(i);ui[i].store(v);pending[i].store(true);setParamNormalized(i,v);performEdit(i,v);if(discrete)endEdit(i);},[this](){release();},"VST3",{},[this](int id,bool begin){gestures[id].store(begin);if(begin)beginEdit(id);else endEdit(id);});}
#endif
 return nullptr;
 }
 static FUnknown* create(void*){return static_cast<IEditController*>(new Plugin);}
 tresult PLUGIN_API initialize(FUnknown* ctx)override{auto result=SingleComponentEffect::initialize(ctx);if(result!=kResultOk)return result;addAudioInput(STR16("Stereo Input"),SpeakerArr::kStereo);addAudioOutput(STR16("Stereo Output"),SpeakerArr::kStereo);
 auto* list=new StringListParameter(STR16("Preset / Tags"),0);for(auto&p:presets)list->appendString(UString128(p.name));parameters.addParameter(list);
 auto* modes=new StringListParameter(STR16("Mode"),1);for(auto*name:{"Reverb","Delay","Phase","Chorus","Echo","Space"})modes->appendString(UString128(name));parameters.addParameter(modes);
 const TChar* names[]={STR16("Fader 1 (16 steps)"),STR16("Fader 2 (4 steps)"),STR16("Fader 3 (4 steps)"),STR16("Predelay 0/20/40/60 ms"),STR16("Rear / Mono / Input mute"),STR16("Dry Wet (center = 15%)"),STR16("Wet Solo"),STR16("Noise"),STR16("Bypass"),STR16("Delay L ms"),STR16("Delay R ms")};for(int i=2;i<13;++i)parameters.addParameter(names[i-2],nullptr,i==2?15:i<6?3:(i==6||i==8||i==9||i==10?1:i>10?63:0),values[i],i==10?ParameterInfo::kIsBypass|ParameterInfo::kCanAutomate:ParameterInfo::kCanAutomate,i);int def=0;for(int i=0;i<int(std::size(presets));++i)if(std::string(presets[i].name).find("Default [")==0)def=i;values[0]=def/double(std::size(presets)-1);load(def);for(int i=0;i<13;++i){setParamNormalized(i,values[i]);ui[i].store(values[i]);}return result;}
 tresult PLUGIN_API setupProcessing(ProcessSetup& setup)override{engine.prepare(setup.sampleRate);return SingleComponentEffect::setupProcessing(setup);}
 tresult PLUGIN_API setProcessing(TBool)override{return kResultOk;}
 tresult PLUGIN_API canProcessSampleSize(int32 size)override{return size==kSample32?kResultTrue:kResultFalse;}
 uint32 PLUGIN_API getTailSamples()override{return uint32(processSetup.sampleRate*30);}
 tresult PLUGIN_API setBusArrangements(SpeakerArrangement* in,int32 ni,SpeakerArrangement* out,int32 no)override{if(ni!=1||no!=1||in[0]!=SpeakerArr::kStereo||out[0]!=SpeakerArr::kStereo)return kResultFalse;return SingleComponentEffect::setBusArrangements(in,ni,out,no);}
 tresult PLUGIN_API process(ProcessData& data)override{
  auto before=values;std::array<double,13> observed{};std::array<bool,13> edited{};bool editing=false;
  for(int i=0;i<13;++i){observed[i]=ui[i].load();edited[i]=pending[i].exchange(false);editing=editing||edited[i]||gestures[i].load();if(edited[i]){double v=ui[i].load();observed[i]=v;bool changed=i==0?std::round(v*(std::size(presets)-1))!=std::round(values[i]*(std::size(presets)-1)):v!=values[i];values[i]=v;if(i==0&&changed)load(int(std::round(v*(std::size(presets)-1))));}}
  if(auto*changes=data.inputParameterChanges)for(int i=0;i<changes->getParameterCount();++i){
   auto*q=changes->getParameterData(i);if(!q||q->getParameterId()>=13||q->getPointCount()==0)continue;
   int id=int(q->getParameterId());int32 offset=0;double v=0;
   if(q->getPoint(q->getPointCount()-1,offset,v)!=kResultTrue||!std::isfinite(v))continue;
   if(edited[id]||pending[id].load()||gestures[id].load()||(id==0&&editing))continue;
   v=std::clamp(v,0.,1.);if(v==values[id]||(id==0&&std::round(v*(std::size(presets)-1))==std::round(values[0]*(std::size(presets)-1))))continue;values[id]=v;
   if(id==0)load(int(std::round(v*(std::size(presets)-1))));
   else if((id==2||id==3)&&int(std::round(values[1]*5))==1){int channel=values[4]<.5?11:12;values[channel]=(std::round(values[2]*15)*20+std::round(values[3]*3)*5)/315.;}
   else if(id==4&&int(std::round(values[1]*5))==1){int t=int(std::round(values[values[4]<.5?11:12]*63))*5;values[2]=(t/20)/15.;values[3]=((t%20)/5)/3.;}
  }
  // Publish only genuine host changes; unchanged preset echoes must never reset the editor.
  for(int i=0;i<13;++i)if(values[i]!=before[i]&&!edited[i]&&!pending[i].load()&&!gestures[i].load())ui[i].compare_exchange_strong(observed[i],values[i]);
  sync();if(data.numInputs<1||data.numOutputs<1||data.numSamples==0)return kResultOk;if(data.symbolicSampleSize!=kSample32)return kResultFalse;auto&in=data.inputs[0];auto&out=data.outputs[0];if(in.numChannels<2||out.numChannels<2)return kResultFalse;out.silenceFlags=0;double peak=0;for(int i=0;i<data.numSamples;++i){peak=std::max(peak,double(std::max(std::abs(in.channelBuffers32[0][i]),std::abs(in.channelBuffers32[1][i]))));auto y=engine.tick(in.channelBuffers32[0][i],in.channelBuffers32[1][i],controls);out.channelBuffers32[0][i]=float(y[0]);out.channelBuffers32[1][i]=float(y[1]);}meter.store(peak);return kResultOk;}
 tresult PLUGIN_API getState(IBStream* stream)override{IBStreamer s(stream,kLittleEndian);for(auto&parameter:ui)if(!s.writeDouble(parameter.load()))return kResultFalse;return kResultOk;}
 tresult PLUGIN_API setState(IBStream* stream)override{IBStreamer s(stream,kLittleEndian);auto next=values;for(auto&v:next)if(!s.readDouble(v)||!std::isfinite(v)||v<0||v>1)return kResultFalse;values=next;for(int i=0;i<13;++i){setParamNormalized(i,values[i]);ui[i].store(values[i]);}return kResultOk;}
};
bool InitModule(){
#ifdef _WIN32
 uiDiagnostic("VST3 InitModule");
#endif
 return true;
}bool DeinitModule(){return true;}
BEGIN_FACTORY_DEF("Aulamix", "https://github.com/aulamix001/EMT-250-reverb-digital-", "")
DEF_CLASS2(INLINE_UID_FROM_FUID(uid),PClassInfo::kManyInstances,kVstAudioEffectClass,"EMT-250 Reverb",0,PlugType::kFxReverb,"0.4.2",kVstVersionString,Plugin::create)
END_FACTORY

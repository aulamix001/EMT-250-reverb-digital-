#include "Vst2Abi.h"
#include <windows.h>
#include <array>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
namespace {
int edits=0;
vst2::Ptr host(vst2::Effect*,vst2::Int op,vst2::Int,vst2::Ptr,void*,float) {
    if(op==vst2::HostVersion)return 2400;
    if(op==vst2::Automate)++edits;
    return 0;
}
void check(bool ok,const char* s) {if(!ok)throw std::runtime_error(s);std::printf("PASS %s\n",s);std::fflush(stdout);}
void pump(){MSG m;for(int i=0;i<5;++i){while(PeekMessage(&m,nullptr,0,0,PM_REMOVE)){TranslateMessage(&m);DispatchMessage(&m);}Sleep(20);}}
void click(HWND h,int x,int y){SendMessage(h,WM_LBUTTONDOWN,MK_LBUTTON,MAKELPARAM(x,y));SendMessage(h,WM_LBUTTONUP,0,MAKELPARAM(x,y));pump();}
}
int main(int argc,char**argv) {try{
    check(argc==2,"VST2 DLL argument");
    std::string path=argv[1];if(path[0]=='/'){path="Z:"+path;std::replace(path.begin(),path.end(),'/','\\');}
    HMODULE module=LoadLibraryA(path.c_str());check(module,"load VST2 DLL");
    auto entry=reinterpret_cast<vst2::Effect*(*)(vst2::Callback)>(GetProcAddress(module,"VSTPluginMain"));check(entry,"VSTPluginMain export");
    auto effect=entry(host);check(effect&&effect->magic==vst2::magic,"VST2 magic");
    check(effect->numInputs==2&&effect->numOutputs==2&&effect->numParams==13&&effect->numPrograms==30,"stereo parameters and 30 factory programs");
    check((effect->flags&1)&&(effect->flags&16)&&(effect->flags&32),"editor replacing and chunk flags");
    auto dispatch=[&](int op,int index=0,vst2::Ptr value=0,void*ptr=nullptr,float opt=0){return effect->dispatcher(effect,op,index,value,ptr,opt);};
    check(dispatch(vst2::Open)==1,"open VST2 instance");
    check(dispatch(vst2::GetVstVersion)==2400,"VST 2.4 protocol");
    check(dispatch(vst2::SetSampleRate,0,0,nullptr,48000)==1,"sample rate");
    check(dispatch(vst2::MainsChanged,0,1)==1,"activate audio");
    float l[256]{},r[256]{},ol[256]{},orr[256]{};float*in[]{l,r};float*out[]{ol,orr};
    for(int mode=0;mode<6;++mode){
        effect->setParameter(effect,1,mode/5.f);effect->setParameter(effect,8,1);effect->setParameter(effect,9,0);
        dispatch(vst2::MainsChanged,0,1);double energy=0;bool finite=true;
        for(int block=0;block<375;++block){
            std::fill_n(l,256,0);std::fill_n(r,256,0);if(block==0)l[0]=r[0]=1;
            effect->processReplacing(effect,in,out,256);
            for(int i=0;i<256;++i){finite=finite&&std::isfinite(ol[i])&&std::isfinite(orr[i]);energy+=ol[i]*ol[i]+orr[i]*orr[i];}
        }
        check(finite&&energy>1e-9,"mode has finite nonzero impulse output");
    }
    dispatch(vst2::SetBypass,0,1);l[0]=.25f;r[0]=-.5f;effect->processReplacing(effect,in,out,256);
    check(ol[0]==.25f&&orr[0]==-.5f,"exact stereo bypass");
    ol[0]=1;orr[0]=1;effect->process(effect,in,out,256);check(ol[0]==1.25f&&orr[0]==.5f,"legacy accumulating process");
    dispatch(vst2::SetBypass,0,0);
    effect->setParameter(effect,7,.375f);void* chunk=nullptr;auto bytes=dispatch(vst2::GetChunk,0,0,&chunk);
    check(bytes==13*sizeof(double)&&chunk,"state chunk");std::vector<unsigned char> saved(static_cast<unsigned char*>(chunk),static_cast<unsigned char*>(chunk)+bytes);
    effect->setParameter(effect,7,.9f);check(dispatch(vst2::SetChunk,0,bytes,saved.data())==1,"restore state");
    check(effect->getParameter(effect,7)==.375f,"state round trip");
    check(dispatch(vst2::SetChunk,0,bytes-1,saved.data())==0,"reject truncated state");
    for(int program=0;program<30;++program){dispatch(vst2::SetProgram,0,program);check(dispatch(vst2::GetProgram)==program,"select factory program");}
    vst2::Rect* size=nullptr;check(dispatch(vst2::EditGetRect,0,0,&size)==1&&size&&size->right==1186&&size->bottom==570,"editor dimensions");
    HWND parent=CreateWindowExA(0,"STATIC","VST2 host test",WS_OVERLAPPEDWINDOW|WS_VISIBLE,282,125,1300,750,nullptr,nullptr,GetModuleHandle(nullptr),nullptr);check(parent,"host HWND");
    for(int pass=0;pass<3;++pass){
        check(dispatch(vst2::EditOpen,0,0,parent)==1,"open VST2 editor");pump();HWND editor=GetWindow(parent,GW_CHILD);check(editor&&IsWindowVisible(editor),"visible native VST2 editor");
        SetWindowLongPtrA(editor,GWLP_USERDATA,12345);click(editor,380,450);check(std::abs(effect->getParameter(effect,1)-.2f)<.0001,"mode from UI");
        click(editor,260,450);click(editor,310,90);check(effect->getParameter(effect,2)==1,"fader from UI");check(edits>0,"UI sends host automation");
        click(editor,200,20);HWND popup=nullptr;
        for(HWND w=GetTopWindow(nullptr);w;w=GetWindow(w,GW_HWNDNEXT)){char name[128]{};GetClassNameA(w,name,sizeof(name));HWND owner=GetWindow(w,GW_OWNER);if((owner==editor||owner==parent)&&std::string(name).find("EMT250Presets-")==0){popup=w;break;}}
        check(popup&&IsWindowVisible(popup),"VST2 preset browser");
        check(dispatch(vst2::EditClose)==1&&!IsWindow(editor)&&!IsWindow(popup),"close VST2 editor and browser");
    }
    DestroyWindow(parent);dispatch(vst2::MainsChanged,0,0);dispatch(vst2::Close);FreeLibrary(module);std::puts("VST2 HOST TEST PASSED");return 0;
}catch(const std::exception&e){std::fprintf(stderr,"FAIL %s\n",e.what());return 1;}}

// Windows VST3 host regression test. Run natively, or with Wine for cross builds.
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <stdexcept>
#include <string>
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/gui/iplugview.h"
#include "public.sdk/source/vst/hosting/parameterchanges.h"
#include "pluginterfaces/gui/iplugviewcontentscalesupport.h"
using namespace Steinberg;
using namespace Steinberg::Vst;

static void require(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
    std::printf("PASS %s\n", message);
    std::fflush(stdout);
}
static void pump() {
    MSG message;
    for (int i = 0; i < 5; ++i) {
        while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
        Sleep(20);
    }
}
static void click(HWND window, int x, int y) {
    RECT client{};GetClientRect(window,&client);char name[128]{};GetClassNameA(window,name,sizeof(name));
    bool popup=std::string(name).find("EMT250Presets-")==0;
    if(popup){x=int(std::lround(double(x)*client.right/390));y=int(std::lround(double(y)*client.bottom/710));}
    else{double zoom=std::min(client.right/958.,client.bottom/403.);x=int(std::lround((client.right-958*zoom)/2+x*zoom));y=int(std::lround((client.bottom-403*zoom)/2+(y+32)*zoom));}
    SendMessage(window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
    SendMessage(window, WM_LBUTTONUP, 0, MAKELPARAM(x, y));
    pump();
}
static void expectRect(HWND window, HWND parent, const ViewRect& expected) {
    RECT actual{};
    require(GetWindowRect(window, &actual), "read native editor bounds");
    MapWindowPoints(nullptr, parent, reinterpret_cast<POINT*>(&actual), 2);
    require(actual.left == 0 && actual.top == 0 &&
            actual.right == expected.getWidth() && actual.bottom == expected.getHeight(),
            "native editor stays at the host container local origin");
}
static void expectPaint(HWND window) {
    HDC dc = GetDC(window), memory = CreateCompatibleDC(dc);
    RECT bounds{};GetClientRect(window,&bounds);
    HBITMAP bitmap = CreateCompatibleBitmap(dc, bounds.right, bounds.bottom);
    auto previous = SelectObject(memory, bitmap);
    FillRect(memory, &bounds, reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
    SendMessage(window, WM_PRINTCLIENT, reinterpret_cast<WPARAM>(memory), PRF_CLIENT);
    double zoom=std::min(bounds.right/958.,bounds.bottom/403.);double ox=(bounds.right-958*zoom)/2,oy=(bounds.bottom-403*zoom)/2;
    COLORREF panel=GetPixel(memory,int(ox+800*zoom),int(oy+72*zoom));
    COLORREF rack=GetPixel(memory,int(ox+853*zoom),int(oy+52*zoom));
    bool painted=panel!=CLR_INVALID&&rack!=CLR_INVALID&&GetRValue(panel)<170&&GetRValue(rack)>45&&GetRValue(rack)>GetGValue(rack)*1.5;
    SelectObject(memory, previous);
    static bool captured=false;
    if(!captured&&bounds.right==1198&&bounds.bottom==504){
        if(const char*path=std::getenv("EMT250_TEST_IMAGE")){
            BITMAPINFO info{};info.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);info.bmiHeader.biWidth=bounds.right;info.bmiHeader.biHeight=bounds.bottom;info.bmiHeader.biPlanes=1;info.bmiHeader.biBitCount=32;
            std::vector<unsigned char> pixels(size_t(bounds.right)*bounds.bottom*4);
            require(GetDIBits(dc,bitmap,0,bounds.bottom,pixels.data(),&info,DIB_RGB_COLORS)!=0,"capture scaled panel");
            BITMAPFILEHEADER header{};header.bfType=0x4d42;header.bfOffBits=sizeof(header)+sizeof(BITMAPINFOHEADER);header.bfSize=DWORD(header.bfOffBits+pixels.size());
            FILE*file=std::fopen(path,"wb");require(file!=nullptr,"snapshot file");
            bool written=std::fwrite(&header,sizeof(header),1,file)==1&&std::fwrite(&info.bmiHeader,sizeof(BITMAPINFOHEADER),1,file)==1&&std::fwrite(pixels.data(),pixels.size(),1,file)==1;
            std::fclose(file);require(written,"snapshot written");captured=true;
        }
    }
    DeleteObject(bitmap);
    DeleteDC(memory);
    ReleaseDC(window, dc);
    require(painted, "host print request renders the panel rather than a blank surface");
}
class HostFrame:public IPlugFrame {
    uint32 refs=1;
public:
    int requests=0;
    tresult PLUGIN_API queryInterface(const TUID id,void**object)override{
        if(!object)return kInvalidArgument;
        if(FUnknownPrivate::iidEqual(id,IPlugFrame::iid)||FUnknownPrivate::iidEqual(id,FUnknown::iid)){*object=static_cast<IPlugFrame*>(this);addRef();return kResultOk;}
        *object=nullptr;return kNoInterface;
    }
    uint32 PLUGIN_API addRef()override{return ++refs;}
    uint32 PLUGIN_API release()override{auto n=--refs;if(!n)delete this;return n;}
    tresult PLUGIN_API resizeView(IPlugView*view,ViewRect*size)override{++requests;return view->onSize(size);}
};
int main(int argc, char** argv) {
    try {
        require(argc == 2, "VST3 module argument");
        std::string path = argv[1];
        if (path[0] == '/') { // CTest on a Linux cross-build running under Wine.
            path = "Z:" + path;
            std::replace(path.begin(), path.end(), '/', '\\');
        }
        HMODULE module = LoadLibraryA(path.c_str());
        require(module != nullptr, "load VST3 module");
        auto init = reinterpret_cast<bool(*)()>(GetProcAddress(module, "InitDll"));
        require(init && init(), "InitDll");
        auto factoryFunction = reinterpret_cast<IPluginFactory*(*)()>(GetProcAddress(module, "GetPluginFactory"));
        require(factoryFunction != nullptr, "factory export");
        auto factory = factoryFunction();
        PClassInfo info{};
        require(factory->getClassInfo(0, &info) == kResultOk, "class info");
        IComponent* component = nullptr;
        require(factory->createInstance(info.cid, IComponent::iid, reinterpret_cast<void**>(&component)) == kResultOk, "instantiate component");
        require(component->initialize(nullptr) == kResultOk, "initialize component");
        IEditController* controller = nullptr;
        IAudioProcessor* processor = nullptr;
        require(component->queryInterface(IEditController::iid, reinterpret_cast<void**>(&controller)) == kResultOk, "controller interface");
        require(component->queryInterface(IAudioProcessor::iid, reinterpret_cast<void**>(&processor)) == kResultOk, "processor interface");
        ProcessSetup setup{kRealtime, kSample32, 256, 48000};
        require(processor->setupProcessing(setup) == kResultOk, "setup processor");
        component->setActive(true);
        processor->setProcessing(true);
        HWND host = CreateWindowExA(0, "STATIC", "VST3 host test", WS_OVERLAPPEDWINDOW,
                                   282, 125, 1500, 800, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
        require(host != nullptr, "create initially hidden host");
        for (int pass = 0; pass < 3; ++pass) {
            auto view = controller->createView(ViewType::kEditor);
            require(view != nullptr, "create editor");
            IPlugViewContentScaleSupport* contentScale=nullptr;
            require(view->queryInterface(IPlugViewContentScaleSupport::iid,reinterpret_cast<void**>(&contentScale))==kResultOk,"content scale interface");
            require(contentScale->setContentScaleFactor(1.25f)==kResultOk,"125% before frame and attachment");
            ViewRect dpiSize;view->getSize(&dpiSize);
            require(dpiSize.getWidth()==1198&&dpiSize.getHeight()==504,"125% physical dimensions before frame");
            auto frame=new HostFrame;
            require(view->setFrame(frame)==kResultOk,"host frame");
            require(contentScale->setContentScaleFactor(1.f)==kResultOk&&frame->requests==1,"resize negotiation with host");
            require(view->isPlatformTypeSupported(nullptr) == kResultFalse, "reject null platform");
            require(view->attached(nullptr, kPlatformTypeHWND) != kResultOk, "reject missing parent");
            require(view->onSize(nullptr) != kResultOk, "reject missing rectangle");
            ViewRect empty;
            require(view->onSize(&empty) != kResultOk, "reject empty rectangle");
            // The host container is already positioned in its FX panel. ViewRect origins
            // must not be applied a second time to the native child.
            ViewRect requested{282, 125, 1240, 528};
            require(view->onSize(&requested) == kResultOk, "position before attachment");
            require(view->attached(host, kPlatformTypeHWND) == kResultOk, "attach editor");
            HWND editor = GetWindow(host, GW_CHILD);
            require(editor != nullptr, "native editor exists");
            char registeredClass[128]{};GetClassNameA(editor,registeredClass,sizeof(registeredClass));
            expectRect(editor, host, requested);
            ShowWindow(host, SW_SHOW);
            pump();
            require(IsWindowVisible(editor), "editor becomes visible with host");
            expectPaint(editor);
            // Hosts may use GWLP_USERDATA when subclassing their embedded windows.
            SetWindowLongPtrA(editor, GWLP_USERDATA, 12345);
            expectPaint(editor);
            SetWindowLongPtrA(editor, GWLP_USERDATA, 0);
            ViewRect moved{20, 30, 978, 433};
            require(view->onSize(&moved) == kResultOk, "position after attachment");
            expectRect(editor, host, moved);
            ViewRect actual;
            require(view->getSize(&actual) == kResultOk && actual.left == moved.left &&
                    actual.top == moved.top && actual.getWidth() == 958 && actual.getHeight() == 403,
                    "reported and native rectangles agree");
            if(pass==0){
                require(view->canResize()==kResultTrue,"host may resize the panel to fit available space");
                for(float factor:{1.f,1.25f,1.5f,1.75f,2.f,2.5f,3.f,4.f,5.f,1.37f}){
                    click(editor,208,325);
                    require(contentScale->setContentScaleFactor(factor)==kResultOk,"change display scale");
                    ViewRect scaledSize;view->getSize(&scaledSize);
                    require(scaledSize.getWidth()==std::lround(958*factor)&&scaledSize.getHeight()==std::lround(403*factor),"physical dimensions at requested scale");
                    expectRect(editor,host,scaledSize);expectPaint(editor);
                    click(editor,208,325);
                    require(controller->getParamNormalized(1)==0.,"reset mode through scaled control");
                    click(editor,311,325);
                    require(std::abs(controller->getParamNormalized(1)-.2)<.0001,"scaled pointer reaches the correct control");
                    std::printf("PASS scale %.0f%% paint and input\n",factor*100);
                }
            }
            ViewRect fitted{0,0,900,450};
            require(view->checkSizeConstraint(&fitted)==kResultTrue&&view->onSize(&fitted)==kResultOk,"host can fit the whole panel into a smaller area");
            expectPaint(editor);click(editor,208,325);require(controller->getParamNormalized(1)==0.,"input in resized panel");
            require(contentScale->setContentScaleFactor(1.25f)==kResultOk,"125% while attached");
            ViewRect scaled;view->getSize(&scaled);expectRect(editor,host,scaled);expectPaint(editor);
            require(scaled.getWidth()==1198&&scaled.getHeight()==504,"125% attached dimensions");
            float left[256]{}, right[256]{}, outLeft[256]{}, outRight[256]{};
            float* input[]{left, right};
            float* output[]{outLeft, outRight};
            AudioBusBuffers inBus, outBus;
            inBus.numChannels = outBus.numChannels = 2;
            inBus.channelBuffers32 = input;
            outBus.channelBuffers32 = output;
            ProcessData data{};
            data.symbolicSampleSize = kSample32;
            data.numSamples = 256;
            data.numInputs = data.numOutputs = 1;
            data.inputs = &inBus;
            data.outputs = &outBus;
            const int modeX[]{208,311,343,375,407,439};
            for (int mode = 0; mode < 6; ++mode) {
                click(editor, modeX[mode], 325);
                require(std::abs(controller->getParamNormalized(1) - mode / 5.) < .0001, "mode button updates host parameter");
                left[0] = right[0] = 1;
                require(processor->process(data) == kResultOk, "process UI-selected mode");
                bool finite = true;
                for (int i = 0; i < 256; ++i) finite = finite && std::isfinite(outLeft[i]) && std::isfinite(outRight[i]);
                require(finite, "finite stereo output");
            }
            click(editor,208,325); // Reverb: fader 1 is active.
            click(editor,244,10);
            require(controller->getParamNormalized(2) == 1., "fader updates host parameter");
            ParameterChanges echoes(13);int32 queueIndex=0,pointIndex=0;
            auto* presetEcho=echoes.addParameterData(0,queueIndex);presetEcho->addPoint(0,double(float(controller->getParamNormalized(0))),pointIndex);
            data.inputParameterChanges=&echoes;
            auto mouse=[&](UINT message,int x,int y){RECT c{};GetClientRect(editor,&c);double z=std::min(c.right/958.,c.bottom/403.);int px=int(std::lround((c.right-958*z)/2+x*z)),py=int(std::lround((c.bottom-403*z)/2+(y+32)*z));SendMessage(editor,message,message==WM_LBUTTONUP?0:MK_LBUTTON,MAKELPARAM(px,py));};
            mouse(WM_LBUTTONDOWN,244,239);
            for(int y:{210,160,110,60,10}){mouse(WM_MOUSEMOVE,244,y);require(processor->process(data)==kResultOk,"audio processes during mouse drag");}
            mouse(WM_LBUTTONUP,244,10);
            require(controller->getParamNormalized(2)==1.,"continuous drag reaches top with audio running");
            for(int block=0;block<8;++block)require(processor->process(data)==kResultOk,"process repeated unchanged host preset");
            SendMessage(editor,WM_COMMAND,6,0);click(editor,244,242);SendMessage(editor,WM_COMMAND,7,0);
            require(controller->getParamNormalized(2)==1.,"fader survives host preset echo and audio blocks");
            echoes.clearQueue();auto* automation=echoes.addParameterData(2,queueIndex);automation->addPoint(0,.25,pointIndex);
            controller->setParamNormalized(2,.25);
            for(int block=0;block<3;++block)require(processor->process(data)==kResultOk,"genuine host automation processes");
            SendMessage(editor,WM_COMMAND,6,0);click(editor,244,242);SendMessage(editor,WM_COMMAND,7,0);
            require(controller->getParamNormalized(2)==.25,"genuine automation still reaches editor");
            data.inputParameterChanges=nullptr;click(editor,244,10);
            SendMessage(editor,WM_COMMAND,5,0);click(editor,244,242);require(controller->getParamNormalized(2)==0.,"edit comparison B");
            SendMessage(editor,WM_COMMAND,4,0);require(controller->getParamNormalized(2)==1.,"restore comparison A");
            SendMessage(editor,WM_COMMAND,5,0);require(controller->getParamNormalized(2)==0.,"restore comparison B");
            SendMessage(editor,WM_COMMAND,6,0);click(editor,244,10);SendMessage(editor,WM_COMMAND,7,0);require(controller->getParamNormalized(2)==0.,"copy and paste complete state");
            click(editor,200,-16);
            // The preset browser is a top-level owned window, not a child.
            HWND found = nullptr;
            for (HWND candidate = GetTopWindow(nullptr); candidate; candidate = GetWindow(candidate, GW_HWNDNEXT)) {
                HWND owner = GetWindow(candidate, GW_OWNER);
                if (owner != editor && owner != host) continue;
                char name[128]{};
                GetClassNameA(candidate, name, sizeof(name));
                if (std::string(name).find("EMT250Presets-") == 0) { found = candidate; break; }
            }
            HWND popup = found;
            require(popup && IsWindowVisible(popup), "owned preset browser opens");
            require(contentScale->setContentScaleFactor(1.5f)==kResultOk&&IsWindow(popup)&&IsWindowVisible(popup),"floating browser survives host scale renegotiation");
            require(contentScale->setContentScaleFactor(1.25f)==kResultOk,"restore scale with browser open");
            HWND search = FindWindowExA(popup, nullptr, "EDIT", nullptr);
            require(search != nullptr, "preset search control");
            SetWindowTextA(search, "Default");
            pump();
            click(popup, 50, 365);
            require(controller->getParamNormalized(1) == 0., "filtered preset selection");
            require(processor->process(data) == kResultOk, "process preset state");
            require(view->removed() == kResultOk, "detach editor");
            require(!IsWindow(editor) && !IsWindow(popup), "destroy editor and owned browser");
            require(view->attached(host, kPlatformTypeHWND) == kResultOk, "reattach same view");
            editor = GetWindow(host, GW_CHILD);
            expectRect(editor, host, scaled);
            expectPaint(editor);
            require(view->removed() == kResultOk, "detach reattached view");
            require(view->setFrame(nullptr)==kResultOk,"clear host frame");frame->release();contentScale->release();
            view->release();
            WNDCLASSA obsolete{};
            require(!GetClassInfoA(module,registeredClass,&obsolete), "unregister class after last view closes");
            ShowWindow(host, SW_HIDE);
        }
        processor->setProcessing(false);
        component->setActive(false);
        processor->release();
        controller->release();
        component->terminate();
        component->release();
        factory->release();
        DestroyWindow(host);
        auto exit = reinterpret_cast<bool(*)()>(GetProcAddress(module, "ExitDll"));
        require(exit && exit(), "ExitDll");
        FreeLibrary(module);
        std::puts("WINDOWS HOST TEST PASSED");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "FAIL %s\n", error.what());
        return 1;
    }
}

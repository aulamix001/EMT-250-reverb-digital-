// Windows VST3 host regression test. Run natively, or with Wine for cross builds.
#include <windows.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>
#include "pluginterfaces/base/ipluginbase.h"
#include "pluginterfaces/vst/ivstcomponent.h"
#include "pluginterfaces/vst/ivsteditcontroller.h"
#include "pluginterfaces/vst/ivstaudioprocessor.h"
#include "pluginterfaces/gui/iplugview.h"
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
    SendMessage(window, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(x, y));
    SendMessage(window, WM_LBUTTONUP, 0, MAKELPARAM(x, y));
    pump();
}
static void expectRect(HWND window, HWND parent, const ViewRect& expected) {
    RECT actual{};
    require(GetWindowRect(window, &actual), "read native editor bounds");
    MapWindowPoints(nullptr, parent, reinterpret_cast<POINT*>(&actual), 2);
    require(actual.left == expected.left && actual.top == expected.top &&
            actual.right == expected.right && actual.bottom == expected.bottom,
            "native editor follows the host rectangle");
}
static void expectPaint(HWND window) {
    HDC dc = GetDC(window), memory = CreateCompatibleDC(dc);
    HBITMAP bitmap = CreateCompatibleBitmap(dc, 1186, 570);
    auto previous = SelectObject(memory, bitmap);
    RECT bounds{0, 0, 1186, 570};
    FillRect(memory, &bounds, reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH)));
    SendMessage(window, WM_PRINTCLIENT, reinterpret_cast<WPARAM>(memory), PRF_CLIENT);
    bool painted = GetPixel(memory, 1000, 20) == RGB(58, 58, 58) &&
                   GetPixel(memory, 1000, 20) != GetPixel(memory, 1100, 300);
    SelectObject(memory, previous);
    DeleteObject(bitmap);
    DeleteDC(memory);
    ReleaseDC(window, dc);
    require(painted, "host print request renders the panel rather than a blank surface");
}
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
                                   0, 0, 1500, 800, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
        require(host != nullptr, "create initially hidden host");
        for (int pass = 0; pass < 3; ++pass) {
            auto view = controller->createView(ViewType::kEditor);
            require(view != nullptr, "create editor");
            require(view->isPlatformTypeSupported(nullptr) == kResultFalse, "reject null platform");
            require(view->attached(nullptr, kPlatformTypeHWND) != kResultOk, "reject missing parent");
            require(view->onSize(nullptr) != kResultOk, "reject missing rectangle");
            ViewRect empty;
            require(view->onSize(&empty) != kResultOk, "reject empty rectangle");
            // Model an FX panel with controls to the left and above the plugin.
            ViewRect requested{282, 125, 1468, 695};
            require(view->onSize(&requested) == kResultOk, "position before attachment");
            require(view->attached(host, kPlatformTypeHWND) == kResultOk, "attach editor");
            HWND editor = FindWindowExA(host, nullptr, "Aulamix250Editor", nullptr);
            require(editor != nullptr, "native editor exists");
            expectRect(editor, host, requested);
            ShowWindow(host, SW_SHOW);
            pump();
            require(IsWindowVisible(editor), "editor becomes visible with host");
            expectPaint(editor);
            ViewRect moved{20, 30, 1206, 600};
            require(view->onSize(&moved) == kResultOk, "position after attachment");
            expectRect(editor, host, moved);
            ViewRect actual;
            require(view->getSize(&actual) == kResultOk && actual.left == moved.left &&
                    actual.top == moved.top && actual.getWidth() == 1186 && actual.getHeight() == 570,
                    "reported and native rectangles agree");
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
            const int modeX[]{260, 380, 420, 460, 500, 540};
            for (int mode = 0; mode < 6; ++mode) {
                click(editor, modeX[mode], 450);
                require(std::abs(controller->getParamNormalized(1) - mode / 5.) < .0001, "mode button updates host parameter");
                left[0] = right[0] = 1;
                require(processor->process(data) == kResultOk, "process UI-selected mode");
                bool finite = true;
                for (int i = 0; i < 256; ++i) finite = finite && std::isfinite(outLeft[i]) && std::isfinite(outRight[i]);
                require(finite, "finite stereo output");
            }
            click(editor, 260, 450); // Reverb: fader 1 is active.
            click(editor, 310, 90);
            require(controller->getParamNormalized(2) == 1., "fader updates host parameter");
            click(editor, 200, 20);
            // The preset browser is a top-level owned window, not a child.
            HWND found = nullptr;
            for (HWND candidate = GetTopWindow(nullptr); candidate; candidate = GetWindow(candidate, GW_HWNDNEXT)) {
                HWND owner = GetWindow(candidate, GW_OWNER);
                if (owner != editor && owner != host) continue;
                char name[128]{};
                GetClassNameA(candidate, name, sizeof(name));
                if (std::string(name) == "Aulamix250Presets") { found = candidate; break; }
            }
            HWND popup = found;
            require(popup && IsWindowVisible(popup), "owned preset browser opens");
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
            editor = FindWindowExA(host, nullptr, "Aulamix250Editor", nullptr);
            expectRect(editor, host, moved);
            expectPaint(editor);
            require(view->removed() == kResultOk, "detach reattached view");
            view->release();
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

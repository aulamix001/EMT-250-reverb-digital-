#pragma once
#ifdef _WIN32
#include <windows.h>
inline float nativeDpiScale(HWND parent=nullptr){
    HMODULE user=GetModuleHandleA("user32.dll");
    using WindowDpi=UINT(WINAPI*)(HWND);using SystemDpi=UINT(WINAPI*)();
    auto windowDpi=reinterpret_cast<WindowDpi>(GetProcAddress(user,"GetDpiForWindow"));
    auto systemDpi=reinterpret_cast<SystemDpi>(GetProcAddress(user,"GetDpiForSystem"));
    UINT dpi=parent&&windowDpi?windowDpi(parent):systemDpi?systemDpi():96;
    return dpi?dpi/96.f:1.f;
}
#endif

#pragma once
#ifdef _WIN32
#include <windows.h>
#include <cstdio>
#include <cstdarg>
inline void uiDiagnostic(const char* format, ...) {
    char line[1024];
    SYSTEMTIME time{};GetLocalTime(&time);
    int prefix=std::snprintf(line,sizeof(line),"0.4.2 %02u:%02u:%02u pid=%lu tid=%lu ",time.wHour,time.wMinute,time.wSecond,GetCurrentProcessId(),GetCurrentThreadId());
    va_list args;va_start(args,format);std::vsnprintf(line+prefix,sizeof(line)-prefix,format,args);va_end(args);
    OutputDebugStringA(line);
    char directory[MAX_PATH]{};
    DWORD length=GetTempPathA(MAX_PATH,directory);
    if(!length||length>=MAX_PATH)return;
    char path[MAX_PATH+32];std::snprintf(path,sizeof(path),"%sEMT250-ui.log",directory);
    if(FILE* file=std::fopen(path,"ab")){std::fprintf(file,"%s\n",line);std::fclose(file);}
}
#endif

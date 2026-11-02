#pragma once
// Minimal VST 2.4 binary interface declarations used by this adapter.
// No Steinberg VST2 SDK implementation is included.
#include <cstdint>
#include <cstddef>
namespace vst2 {
using Int = std::int32_t;
using Ptr = std::intptr_t;
struct Effect;
using Callback = Ptr (*)(Effect*, Int, Int, Ptr, void*, float);
using Process = void (*)(Effect*, float**, float**, Int);
using SetParameter = void (*)(Effect*, Int, float);
using GetParameter = float (*)(Effect*, Int);
struct Effect {
    Int magic;
    Callback dispatcher;
    Process process;
    SetParameter setParameter;
    GetParameter getParameter;
    Int numPrograms, numParams, numInputs, numOutputs, flags;
    Ptr reserved1, reserved2;
    Int initialDelay, realQualities, offQualities;
    float ioRatio;
    void* object;
    void* user;
    Int uniqueID, version;
    Process processReplacing;
    void (*processDoubleReplacing)(Effect*, double**, double**, Int);
    char future[56];
};
struct Rect { std::int16_t top, left, bottom, right; };
constexpr Int magic = 0x56737450;
enum Opcode { Open=0, Close=1, SetProgram=2, GetProgram=3, SetProgramName=4,
    GetProgramName=5, GetParamLabel=6, GetParamDisplay=7, GetParamName=8,
    SetSampleRate=10, SetBlockSize=11, MainsChanged=12, EditGetRect=13,
    EditOpen=14, EditClose=15, EditIdle=19, GetChunk=23, SetChunk=24,
    GetProgramNameIndexed=29, GetCategory=35, SetBypass=44, GetEffectName=45,
    GetVendorString=47, GetProductString=48, GetVendorVersion=49, CanDo=51,
    GetTailSize=52, GetVstVersion=58, StartProcess=71, StopProcess=72,
    SetProcessPrecision=77 };
enum HostOpcode { Automate=0, HostVersion=1, HostIdle=3, SizeWindow=15,
    UpdateDisplay=42, BeginEdit=43, EndEdit=44 };
static_assert(sizeof(void*) != 8 || (sizeof(Effect)==192 && offsetof(Effect,object)==96 && offsetof(Effect,processReplacing)==120), "VST2 x64 ABI layout");
}

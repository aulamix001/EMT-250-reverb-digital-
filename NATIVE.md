# 0.4.2 — stable mouse edits and black floating browser

Fixed a reproduced VST3 parameter feedback defect: repeated host messages for the already-selected preset used to reload the factory values and reset user edits. Equality is now based on the discrete preset index, including float-rounded host values. Pending editor changes are consumed after acquiring the dirty flag and take priority over stale host events; processor state is no longer blindly copied back over the editor every block. Only genuine host changes are published with compare/exchange. Continuous fader/Dry-Wet gestures send one beginEdit at mouse down and one endEdit at release/capture loss, and protect edited parameters while held. Session state reads the latest editor mailbox, including edits made while transport is stopped. Actual automation is still accepted after editing.

The regression test failed on 0.4.1 and now covers held mouse movement with audio processing, repeated unchanged preset echoes, float-rounded preset values, copy/paste after audio blocks, and genuine host automation. All faders can move visually; controls unused by a DSP mode continue to have no effect on that mode's audio.

The floating browser is explicitly owned by the host root window and activated on open. DPI renegotiation no longer destroys it. Its body and search field are black, with a custom draggable header. Tests verify it remains visible through host scale changes and still searches/selects presets. The main panel artwork, sprites, colors and layout are unchanged; only the version label changes. Main painting now composes into a reusable native-resolution offscreen bitmap and presents one complete frame, avoiding visible clear/background/control phases during mouse updates.

# 0.4.1 — preset toolbar and VST2 host DPI protocol

Restored the visible preset toolbar with previous/next, preset name/browser, IN, A/B, COPY/PASTE and INFO. Version and format are visible to distinguish older cached/duplicate installations. The panel stays in a 958 x 371 area below the 32-pixel toolbar; total native size 958 x 403, 125% 1198 x 504. Rendering and pointer transforms share the same fitted viewport, including the toolbar offset. VST2 advertises supportsViewDpiScaling and handles the PreSonus-compatible PreS/AeCs content-scale callback, retaining the requested scale across editor reopen. Tests exercise the preset toolbar in both formats and VST2 scale callbacks at 125%, plus the existing scale/input/state suites. These changes are validated with Wine; native REAPER behavior still needs confirmation. The screenshot supplied with this report depicts the earlier procedural artwork, so duplicate/older installations should be removed.

# EMT-250 Reverb — 0.4.0 — reference-style panel

The Windows VST3 and VST2 share the new panel-only layout based on the user's latest reference. Native size is 958 x 371; 125% is 1198 x 464. There is no internal top toolbar or footer in this reference. The host's own controls remain outside the plugin. Presets, A/B, COPY/PASTE, save/import and INFO are available by right-click; clicking the EMT 250 name opens the preset browser. The LEARN menu explains host MIDI/automation mapping; native MIDI learn is not implemented.

The chassis uses an embedded high-resolution reconstructed plate. The moving caps and rubber bellows are original orthographic 3D renders, reproducible with `blender --background --python scripts/render-native-materials.py`. LEDs, toggle and Dry/Wet thumb are drawn live. Fonts and PNG assets are embedded into each DLL using Windows resources; no loose artwork or font installation is required. The Michroma font is distributed under its included OFL license. Main rendering uses GDI+ at native physical resolution with a cached static plate, and avoids idle repainting when controls and meter LED levels do not change.

The panel keeps its aspect ratio at arbitrary host window sizes; unused space is letterboxed rather than stretching knobs. Click coordinates use the same fitted viewport. Tests cover scales 100–500%, custom 137%, state, UI automation, A/B and COPY/PASTE. DSP, plugin IDs and parameter IDs are unchanged. The user confirmed version 0.3.0 worked in REAPER 7.84 x64, Windows 11, 125%; version 0.4.0 is validated locally with Wine. Native DAW validation of this new graphical revision is not claimed. The artwork is a reconstruction from the reference; pixel-for-pixel identity to the commercial original has not been verified.

# EMT-250 Reverb — 0.3.0 — Windows VST3 + VST2

The reported target is REAPER 7.84 x64 on Windows 11 at 125%. Version 0.3.0 implements IPlugViewContentScaleSupport, negotiates physical dimensions with IPlugFrame, scales the full panel and inverse-maps pointer coordinates. It supports positive custom scale factors whose pixel dimensions fit the Windows window limits, and allows the host to resize the panel to fit the available area. The preset browser also scales and is bounded to the monitor work area. VST2 reads the native window DPI and handles DPI changes after the parent changes monitor. These changes are tested through protocol calls under Wine, not on that native Windows/REAPER installation.

The Win32 panel is shared by both formats. The new VST2.4 adapter exports VSTPluginMain, stereo replacing and legacy accumulating float processing, 13 normalized parameters, 30 factory programs, state chunks, host automation notifications, bypass and an editor. It uses original minimal ABI declarations in Vst2Abi.h rather than the discontinued proprietary VST2 SDK implementation. The VST3 class and parameter IDs are preserved; the VST2 plugin has its own ID and cannot automatically replace existing VST3 instances.

The 0.2.2 coordinate change is superseded: the native child now stays at (0,0) inside the supplied host container, as the SDK VSTGUI editor does. Host ViewRect dimensions resize it. UI state uses private class window-extra bytes instead of GWLP_USERDATA. Classes are unique per loaded module and unregistered after the last view closes, avoiding stale class procedures after module unload. Creation explicitly handles WM_NCCREATE and clears pointers on WM_NCDESTROY. The panel still handles WM_PRINTCLIENT and immediate repainting.

Diagnostics are appended to `%TEMP%\EMT250-ui.log` during module/UI startup, registration, attachment, sizing and the first WM_PAINT. They include thread/process IDs, HWNDs and Win32 error codes; no audio, secrets or document paths are recorded. Use ABRIR-DIAGNOSTICO.cmd if the editor is blank.

Build Windows targets `Aulamix250 Aulamix250Vst2 dsp-test windows-host-test vst2-host-test`; CTest runs DSP, WindowsHost and VST2Host. For cross builds configure CMAKE_CROSSCOMPILING_EMULATOR to Wine. The VST2 DLL is under build/VST2 (Release subdirectory for multi-config builds). Package both formats with `python scripts/package-windows.py /path/to/flat-vst3-binary /path/to/EMT-250\ Reverb.dll`.

Scale tests cover 100%, 125%, 150%, 175%, 200%, 250%, 300%, 400%, 500% and custom 137%, checking pixel output and resetting/selecting controls at each scale. They also cover fitting the panel into a smaller host area. Tests cover hidden host/show, nonzero container location, local child bounds, host user-data changes, host-requested painting, UI modes/faders, preset search, editor reattachment and class cleanup. VST2 tests also exercise all six impulse responses, exact bypass, accumulating processing, all 30 programs, state round-trip, truncated-state rejection and automation callbacks. An independent host compiled with Clang's Microsoft C++ ABI verifies VST3 factory/controller/view and IPlugFrame interoperability. These are Wine checks, not a REAPER session. REAPER 7.84 x64 was identified by the user, but downloading that host is blocked by the environment's egress policy. The specific reported failure is not yet confirmed resolved in that host.

# EMT-250 Reverb — 0.2.2

Windows editor host compatibility fix: apply the host's IPlugView::onSize rectangle to the HWND, honor positions received before attachment, redraw immediately, and destroy previous child windows on reattachment. Handle WM_PRINTCLIENT so host-requested rendering produces the panel. Reject null platform/invalid parent and size arguments. The power arrow is drawn with GDI to avoid ANSI/UTF-8 corruption. Plugin class and parameter IDs remain unchanged.

The new `WindowsHost` regression test loads the built VST3, attaches to a hidden host at a nonzero FX-panel offset, shows and moves the editor, checks rendered pixels, exercises six modes, a fader and preset search/selection, and detaches/reattaches across three editor lifetimes. On Windows, build target `windows-host-test` before running CTest. Cross-build tests run with `CMAKE_CROSSCOMPILING_EMULATOR` set to Wine. Version 0.2.1 fails the offset regression; 0.2.2 is validated through Wine and the Linux SDK validator. The specific reported REAPER session has not been reproduced in native Windows; these checks do not establish complete REAPER compatibility.

# EMT-250 Reverb — 0.2.1

Renamed the DAW factory name, Windows bundle, panel and preset browser to EMT-250 Reverb. Class ID and parameter IDs unchanged to preserve existing sessions and presets. Remove the old Aulamix250.vst3 installation before rescanning. Current bundle: EMT-250 Reverb.vst3.

# Aulamix 250 VST3 — experimental 0.2.0

Native stereo VST3 with 30 mapped presets and six independent, original DSP approximations. Not the EMT/UAD DSP, not an exact sonic emulation. Reverb/Space use a damped feedback delay bank; Chorus uses four slowly random-modulated taps; Phase combines fixed and variable delays; Delay has independent L/R taps without feedback; Echo uses mono feedback at 0.9 with damping and predelay outside the loop.

**Windows 0.2.0 includes a native graphical editor**, fixed at 1186×570: textured dark panel, four shaded levers, LED scales, red rack ear, toolbar, mode buttons, output switch and Dry/Wet. It is a procedural recreation based on the supplied reference, **not a verified pixel-identical clone**. Linux still uses the generic host editor.

Click the preset name for a separate movable floating window with search, 13 tag filters, preset list and session favorites. Selecting a factory preset updates the visible levers and the audio parameter mailbox, and sends VST3 edit gestures to the host. Save exports a `.a250` settings file; the toolbar `...` imports one. Factory delete/rename are protected. Copy/Paste and A/B compare parameter snapshots. LEARN directs users to host automation; no MIDI-learn implementation. Favorites are retained for the lifetime of the open editor, not persisted across closing it.

All photographed factory values are generated from `model.js` by `node scripts/native-presets.js`. `presets.json` and `MAPEAMENTO.csv` are reviewable exports. Delay R values remain provisional for Dual Delay, Hands Down Double and Jimmy Jam Delay. Large Courtyard has no supplied control photo and is not included. Dry/Wet positions are approximate. Wet Solo and Noise are instance controls enabled by default.

## Build

Use CMake >=3.25 and C++17. Clone Steinberg VST3 SDK tag `v3.8.0_build_66` recursively outside this checkout.

```
node scripts/native-presets.js
cmake -S . -B build -DVST3_SDK_ROOT=/absolute/path/to/vst3sdk -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target Aulamix250 dsp-test --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

Linux output: `build/VST3/Release/Aulamix250.vst3`. Copy the entire bundle to `~/.vst3/` and rescan your host. Windows builds are generated by the GitHub workflow and should be copied to `C:\Program Files\Common Files\VST3`. A Linux VST3 cannot be installed as a Windows VST3.

## Current limits

Experimental first build: no DAW listening test in this environment, no exact converter/saturation emulation, no MIDI learn, no mode-specific state memory. Parameters are stepped/updated at block boundaries; sample-accurate automation and smoothing remain future work. Native L/R delay times are normalized parameters with 64 positions (0–315 ms in steps of 5). The first three generic fader names remain static; refer to the mapping table for their active functions. Inactive faders are ignored by DSP.

The GitHub workflow compiles Linux and Windows artifacts on pushes to main. Tagged pushes also upload release assets using the workflow token. A remote workflow has not been validated until its actual run succeeds. Do not infer Windows validation from the local Linux build.

## Validation of Linux build 0.1.0

SDK validator: 47 passed, 0 failed. DSP impulse tests: six modes emit finite, nonzero output; stereo bypass exact. Catalog: 24 tests passed. No listening/DAW GUI test or Windows build was performed locally. See `releases/validation-linux.txt`.

## Windows x64 cross-build (0.1.0)

`releases/Aulamix250-0.1.0-Windows-x64.zip` contains the standard VST3 Windows bundle and installation notes. Built with Debian MinGW-w64 GCC 14.2 (posix). C/C++ and winpthread runtimes linked statically; imports limited to Windows KERNEL32, msvcrt, ole32, USER32. PE32+ AMD64 and GetPluginFactory/InitDll/ExitDll exports verified. No Windows execution or DAW loading test was performed. Linux validator results do not establish Windows runtime compatibility.

To reproduce on Linux, extract the MinGW packages to a prefix and set MINGW_ROOT, then configure with `-DCMAKE_TOOLCHAIN_FILE=cmake/windows-mingw.cmake`. Cross builds omit the unused SDK dataexchange helper (MinGW lacks std::aligned_alloc) and cosmetic Windows folder-icon commands. Run `python scripts/package-windows.py /path/to/build/VST3/Aulamix250.vst3` to package and inspect the PE headers.

## Windows graphical build 0.2.0

New native Win32/GDI view returned by IEditController::createView for HWND. Host parameters and UI exchange values through atomic mailboxes; repaint uses a timer. Float preset window is owned by the plugin view and closes on view removal. Inactive faders are ignored in Phase/Chorus/Space. Delay channel selection updates the coarse/fine display. UI-to-host parameter edits use beginEdit/performEdit/endEdit.

Cross-compiled Windows x64, PE format and required exports checked. Windows GUI appearance, mouse interaction and DAW loading **have not been tested on a running Windows system**. Linux SDK validator and DSP tests verify the processor, not the Windows GUI. No claim of pixel-identical or complete GUI runtime validation is made.

"""Package verified Windows x64 VST3, optionally together with the VST2 DLL."""
import pathlib
import re
import struct
import sys
import zipfile

root = pathlib.Path(__file__).resolve().parent.parent
version = re.search(r'project\(Aulamix250 VERSION ([0-9.]+)', (root / 'CMakeLists.txt').read_text()).group(1)
vst3 = pathlib.Path(sys.argv[1])
vst2 = pathlib.Path(sys.argv[2]) if len(sys.argv) > 2 else None
suffix = '-VST3-VST2' if vst2 else ''
output = root / f'releases/EMT-250-Reverb-{version}-Windows-x64{suffix}.zip'

def verify(binary, exports):
    data = binary.read_bytes()
    assert data[:2] == b'MZ', f'Not a PE file: {binary}'
    pe = struct.unpack_from('<I', data, 0x3c)[0]
    assert data[pe:pe+4] == b'PE\0\0', 'Invalid PE header'
    assert struct.unpack_from('<H', data, pe+4)[0] == 0x8664, 'Not Windows x64'
    assert struct.unpack_from('<H', data, pe+24)[0] == 0x20b, 'Not PE32+'
    for name in exports:
        assert name.encode() + b'\0' in data, f'Missing export {name}'

verify(vst3, ['GetPluginFactory', 'InitDll', 'ExitDll'])
if vst2:
    verify(vst2, ['VSTPluginMain'])
with zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED) as archive:
    archive.write(vst3, 'EMT-250 Reverb.vst3/Contents/x86_64-win/EMT-250 Reverb.vst3')
    if vst2:
        archive.write(vst2, 'VST2/EMT-250 Reverb.dll')
    for name in ['NATIVE.md', 'INSTALAR-WINDOWS.txt', 'MAPEAMENTO.csv', 'presets.json', 'licenses/Steinberg-VST3-SDK.txt', 'licenses/Michroma-OFL.txt', 'scripts/ABRIR-DIAGNOSTICO.cmd']:
        archive.write(root / name, pathlib.Path(name).name if name.endswith('.cmd') else name)
    validation = root / f'releases/validation-windows-{version}.txt'
    if validation.exists():
        archive.write(validation, 'VALIDACAO.txt')
    snapshot = root / f'releases/interface-{version}-125.png'
    if snapshot.exists():
        archive.write(snapshot, 'interface-125-Wine.png')
    browser_snapshot = root / f'releases/presets-{version}-125.png'
    if browser_snapshot.exists():
        archive.write(browser_snapshot, 'presets-125-Wine.png')
with zipfile.ZipFile(output) as archive:
    assert archive.testzip() is None
print(f'Windows x64 PE and ZIP integrity verified: {output.name}')

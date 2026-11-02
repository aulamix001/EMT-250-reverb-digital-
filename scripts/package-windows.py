"""Package a cross-built Windows VST3 into its standard bundle layout."""
import pathlib, re, struct, sys, zipfile
binary = pathlib.Path(sys.argv[1])
root = pathlib.Path(__file__).resolve().parent.parent
version = re.search(r'project\(Aulamix250 VERSION ([0-9.]+)', (root / 'CMakeLists.txt').read_text()).group(1)
output = root / f'releases/EMT-250-Reverb-{version}-Windows-x64.zip'
data = binary.read_bytes()
assert data[:2] == b'MZ', 'Not a PE file'
pe = struct.unpack_from('<I', data, 0x3c)[0]
assert data[pe:pe+4] == b'PE\0\0', 'Invalid PE header'
assert struct.unpack_from('<H', data, pe+4)[0] == 0x8664, 'Not Windows x64'
assert struct.unpack_from('<H', data, pe+24)[0] == 0x20b, 'Not PE32+'
for entry in (b'GetPluginFactory\0', b'InitDll\0', b'ExitDll\0'):
    assert entry in data, f'Missing VST3 export {entry}'
with zipfile.ZipFile(output, 'w', zipfile.ZIP_DEFLATED) as archive:
    archive.write(binary, 'EMT-250 Reverb.vst3/Contents/x86_64-win/EMT-250 Reverb.vst3')
    for name in ['NATIVE.md', 'INSTALAR-WINDOWS.txt', 'licenses/Steinberg-VST3-SDK.txt']:
        archive.write(root / name, name)
with zipfile.ZipFile(output) as archive:
    assert archive.testzip() is None
print(f'Windows x64 PE verified; ZIP integrity verified: {output.name}')

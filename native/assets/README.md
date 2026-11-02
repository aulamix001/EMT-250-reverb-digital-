# Embedded panel artwork

- `chassis.png`: high-resolution reconstructed static plate following the user's final panel-only reference; the measured registration in `UiArtwork.h` aligns it to the logical 958 × 371 canvas.
- `fader-caps.png`: transparent orthographic render atlas. Left cream, right red. The lower cylinder face is visible, matching the reference camera angle.
- `bellows.png`: transparent corrugated rubber sleeve render, stretched along each moving stem.
- `Michroma-Regular.ttf`: geometric lettering used by the preset browser. See `licenses/Michroma-OFL.txt`.

Regenerate the 3D assets with Blender 4.3+ using `blender --background --python scripts/render-native-materials.py`. CPU Cycles is sufficient. Assets are embedded by `UiAssets.rc.in` in VST3 and VST2. The runtime does not require Blender, image files or an installed font.

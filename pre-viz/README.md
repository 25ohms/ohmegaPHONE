# ohmegaPHONE pre-viz

A small SDL2 desktop app for previewing the firmware's rotating Ω wireframe before flashing the ESP32. The preview surface is rendered at the display's native resolution (320×240 landscape or 240×320 portrait) and shown at 1:1 scale.

## Build and run

Install CMake, a C++17 compiler, and SDL2, then from this directory run:

```sh
cmake -S . -B build
cmake --build build
./build/ohmega_previz
```

On macOS with Homebrew, install SDL2 with `brew install sdl2` if needed. The app uses `pkg-config` to locate SDL2.

## Controls

`main.cpp` constructs and owns the `CsvMeshLoader`, `Screen`, and five `Slider` components, then passes references to `PreVizApp`. The loader reads the CSV name from the shared `include/OhmConfig.h` (resolved from the executable or project directory). `MeshRenderer` draws the mesh, `GraphicsMath` handles projection and UI coordinate mapping, and `UiDrawing` provides SDL text, rectangle, and color helpers. Shared render defaults and the LittleFS path also come from `OhmConfig.h`, so the pre-viz and firmware have one source for those values. These are composed collaborators rather than subclasses: inheritance is reserved for a real “is-a” relationship or shared polymorphic behavior.

Drag the sliders to change scale, Z scale, Y rotation speed, viewing angle, and wireframe spacing. Choose a wireframe color, switch between landscape and portrait, pause or resume animation, or reset to the firmware's current defaults. Both the ESP32 firmware and this visualizer load their mesh from `data/ohmCOORDs.csv` at runtime. Edit that file and relaunch the visualizer to preview the same coordinates used by the firmware; upload the filesystem with `pio run -t uploadfs` to update the board.

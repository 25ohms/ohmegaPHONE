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

Drag the sliders to change scale, extrusion depth, Y rotation speed, viewing angle, and wireframe spacing. Choose a wireframe color, switch between landscape and portrait, pause or resume animation, or reset to the firmware's current defaults. The outline is shared with `include/OhmGeometry.h`, so edits to the glyph appear in both the desktop preview and ESP32 firmware.

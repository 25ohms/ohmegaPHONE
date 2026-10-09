# ohmegaPHONE

This repository serves to hold the firmware using PlatformIO to compile and serve binaries for the ESP32 devkit board (and later on, onto a Daisy Seed).

Hardware specs:
- ESP32
- TFT Screen 2.4in 240x320 RGB LCD Display


### Compilation:
```
pio run             # compile binaries
pio run -t upload   # flash onto firmware
pio run -t uploadfs # upload data/ files to the ESP32 LittleFS filesystem


pio device list     # run this if device is not showing up at all
pio run -t upload --upload-port /dev/cu.usbserial-0001
# example device

pio device monitor  # use this to montior serial outputs

```

The Ω mesh is loaded from `data/ohmCOORDs.csv` at startup. After changing the
CSV, run `pio run -t uploadfs` to update the board's LittleFS data.

### Pre-viz

Preview the Ω before flashing. From the repo root, run `cd pre-viz && cmake -S . -B build && cmake --build build && ./build/ohmega_previz` to build and open it. Requires CMake and SDL2.

The pre-viz is split into small modules. `main.cpp` constructs the
`CsvMeshLoader`, `Screen`, and slider components, then passes them to
`PreVizApp`. Mesh rendering, graphics calculations, and SDL drawing helpers
are separate modules. The loader reads the shared CSV directly, so changing
coordinates does not require editing pre-viz code. The pre-viz uses composition
between these collaborators; it has no artificial class inheritance because
the screen and sliders are not specializations of one another.

Shared compile-time defaults and the coordinate filename live in
[`include/OhmConfig.h`](include/OhmConfig.h). Both the pre-viz and firmware use
that header, so common startup values have one source and add no runtime file
I/O or settings storage. Set `OhmConfig::kDisplayRotation` to `1` or `3` to
choose the ILI9341's landscape direction. Rotation `1` is the reverse landscape
orientation from `3`; it rotates the whole display output by 180 degrees. The
pre-viz screen applies the same rotation to its preview.

### Pre-viz UML

```mermaid
classDiagram
  direction LR
  class main_cpp {
    +main(argc, argv)
  }
  class CsvMeshLoader {
    +loadProjectCsv(executablePath)
    +mesh() Mesh
    +error() string
  }
  class OhmConfig {
    <<static configuration>>
    +kRenderDefaults
    +kCoordinatesFileName
    +kDisplayRotation
  }
  class Mesh {
    +coordinates Point3[]
    +baseVertexCount
    +center Point3
    +size
    +triangleCount()
  }
  class PreVizApp {
    +run()
    -renderMesh()
    -drawInterface()
    -handleMouseDown(x, y)
  }
  class MeshRenderer {
    +draw(renderer, mesh, size, settings)
  }
  class Screen {
    +open(title, size)
    +resizePreview(width, height)
    +drawPreview(destination)
    +renderer()
    +previewTexture()
  }
  class Slider {
    +hitTest(x, y, track)
    +setFromX(mouseX, track)
    +value()
    +draw(renderer, layout)
  }
  class GraphicsMath {
    +toScreen(point, mesh, viewport, settings)
    +sliderValueAt(mouseX, track)
    +sliderPosition(value, track)
    +contains(point, rectangle)
  }
  class UiDrawing {
    +drawText(renderer, text, position)
    +drawRect(renderer, rectangle)
    +from565(color)
  }
  class SDL_Window
  class SDL_Renderer
  class SDL_Texture
  class ohmCOORDs_csv

  main_cpp *-- CsvMeshLoader : owns
  main_cpp *-- Screen : owns
  main_cpp "1" *-- "5" Slider : owns controls
  main_cpp --> PreVizApp : starts with references
  main_cpp ..> OhmConfig : observes shared defaults
  CsvMeshLoader ..> OhmConfig : observes shared filename
  CsvMeshLoader --> ohmCOORDs_csv : reads
  CsvMeshLoader --> Mesh : creates
  PreVizApp --> Mesh : displays
  PreVizApp --> Screen : draws through
  PreVizApp "1" --> "5" Slider : updates and displays
  PreVizApp *-- MeshRenderer : delegates mesh drawing
  PreVizApp --> UiDrawing : draws controls
  PreVizApp --> GraphicsMath : maps UI input
  MeshRenderer --> GraphicsMath : projects vertices
  MeshRenderer --> UiDrawing : converts color
  Screen *-- SDL_Window : owns
  Screen *-- SDL_Renderer : owns
  Screen *-- SDL_Texture : owns preview target
```

### ESP32 firmware UML

```mermaid
classDiagram
  direction LR
  class setup_loop {
    +setup()
    +loop()
  }
  class LittleFS {
    +begin()
    +open(path, mode) File
  }
  class File {
    +readStringUntil(newline)
    +close()
  }
  class Stream {
    +available()
    +readStringUntil(newline)
  }
  class OhmWireframe {
    +loadGeometry(stream) bool
    +draw(elapsedMillis)
    +settings()
    -project(x, y, z, angles) Point3
    -drawMeshTriangle(points)
    -drawSegment(a, b)
  }
  class OhmConfig {
    <<static configuration>>
    +kRenderDefaults
    +kCoordinatesFileName
    +kDisplayRotation
  }
  class Adafruit_GFX {
    +drawLine(x1, y1, x2, y2, color)
  }
  class Adafruit_ILI9341 {
    +begin()
    +fillScreen(color)
  }
  class ESP32_TFT

  setup_loop --> LittleFS : mounts and opens CSV
  LittleFS --> File : returns
  File --|> Stream : implements
  setup_loop --> OhmWireframe : loads mesh and draws frames
  setup_loop ..> OhmConfig : observes shared filename
  OhmWireframe ..> OhmConfig : observes shared defaults
  OhmWireframe ..> Stream : parses coordinates
  OhmWireframe --> Adafruit_GFX : draws projected mesh
  Adafruit_ILI9341 --|> Adafruit_GFX : implements display API
  Adafruit_ILI9341 --> ESP32_TFT : drives
```


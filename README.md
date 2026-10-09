# ohmegaPHONE

This repository serves to hold the firmware using PlatformIO to compile and serve binaries for the ESP32 devkit board (and later on, onto a Daisy Seed).

Hardware specs:
- ESP32
- TFT Screen 2.4in 240x320 RGB LCD Display


### Compilation:
```
pio run             # compile binaries
pio run -t upload   # flash onto firmware


pio device list     # run this if device is not showing up at all
pio run -t upload --upload-port /dev/cu.usbserial-0001
# example device

pio device monitor  # use this to montior serial outputs

```

### Pre-viz

Preview the Ω before flashing. From the repo root, run `cd pre-viz && cmake -S . -B build && cmake --build build && ./build/ohmega_previz` to build and open it. Requires CMake and SDL2.


#pragma once

#include <stdint.h>

// Compile-time defaults and shared resource names for the ESP32 and pre-viz.
// Keep preview-only control ranges in the pre-viz UI; these are the values
// both renderers use when they start.
namespace OhmConfig {

struct RenderDefaults {
  float scale;
  float previewZScale;
  float rotationSpeedX;
  float rotationSpeedY;
  float angleDegrees;
  uint8_t meshStep;
  uint16_t color565;
};

constexpr RenderDefaults kRenderDefaults = {1.25f, 1.0f, 0.0f,  0.8f,
                                            0.0f,  1,    0x07FF};

// ILI9341 landscape orientation: use 3 normally or 1 for the reverse view.
// The rotation applies to the whole display, not only the Ω mesh.
constexpr uint8_t kDisplayRotation = 1;
static_assert(kDisplayRotation == 1 || kDisplayRotation == 3,
              "The configured display rotation must be landscape (1 or 3).");

constexpr char kCoordinatesFileName[] = "ohmCOORDs.csv";

} // namespace OhmConfig

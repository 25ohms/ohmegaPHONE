#pragma once

#include <Adafruit_GFX.h>
#include <Arduino.h>

// Small software renderer for a rotating, extruded capital Omega (Ω).
// The glyph is a sampled outline inspired by the proportions of Verdana's
// capital Omega. No font file, framebuffer, or 3D library is needed.
class OhmWireframe {
public:
  struct Settings {
    float scale;             // Overall glyph size in display pixels.
    float depth;             // Extrusion distance in model units.
    float rotationSpeedX;    // Rotation speed, radians per second.
    float rotationSpeedY;
    uint8_t meshStep;        // Lower values draw a denser wireframe.
    uint16_t color;          // Adafruit_GFX 16-bit RGB565 color.
  };

  explicit OhmWireframe(Adafruit_GFX &display);

  Settings &settings() { return _settings; }
  const Settings &settings() const { return _settings; }

  // Draw one frame centered on the display. Call repeatedly from loop().
  void draw(uint32_t elapsedMillis);

private:
  struct Point3 {
    float x;
    float y;
    float z;
  };

  Point3 project(float x, float y, float z, float angleX, float angleY) const;
  void drawContour(const Point3 *contour, size_t count, float angleX,
                   float angleY, bool closed);

  Adafruit_GFX &_display;
  Settings _settings;
};

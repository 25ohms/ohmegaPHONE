#pragma once

#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <Stream.h>

// Small software renderer for a rotating, extruded capital Omega (Ω).
// Mesh coordinates are loaded from a filesystem stream at startup.
class OhmWireframe {
public:
  struct Settings {
    float scale;             // Overall glyph size in display pixels.
    float rotationSpeedX;    // Rotation speed, radians per second.
    float rotationSpeedY;
    uint8_t meshStep;        // Lower values draw a denser wireframe.
    uint16_t color;          // Adafruit_GFX 16-bit RGB565 color.
  };

  explicit OhmWireframe(Adafruit_GFX &display);

  // Load tab-separated x/y/z mesh coordinates from a filesystem stream.
  bool loadGeometry(Stream &input);

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
  void drawSegment(const Point3 &a, const Point3 &b, float angleX,
                   float angleY);
  void drawMeshTriangle(const Point3 *triangle, float angleX, float angleY);

  Adafruit_GFX &_display;
  Settings _settings;
  static constexpr size_t kMaxMeshPoints = 200;
  Point3 _mesh[kMaxMeshPoints];
  size_t _meshPointCount = 0;
  size_t _baseVertexCount = 0;
  float _centerX = 0.0f;
  float _centerY = 0.0f;
  float _centerZ = 0.0f;
  float _fitScale = 1.0f;
};

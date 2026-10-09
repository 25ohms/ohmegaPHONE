#include "OhmWireframe.h"
#include "OhmGeometry.h"

#include <math.h>

namespace {
// Apply a small perspective projection after rotating around X and Y.
constexpr float kCameraDistance = 230.0f;
} // namespace

OhmWireframe::OhmWireframe(Adafruit_GFX &display)
    : _display(display),
      _settings{1.35f, 14.0f, 0.0f, 0.9f, 3, 0x07FF} {}

OhmWireframe::Point3 OhmWireframe::project(float x, float y, float z,
                                          float angleX,
                                          float angleY) const {
  const float cosY = cosf(angleY);
  const float sinY = sinf(angleY);
  const float rotatedX = x * cosY + z * sinY;
  const float rotatedZ = -x * sinY + z * cosY;

  const float cosX = cosf(angleX);
  const float sinX = sinf(angleX);
  const float rotatedY = y * cosX - rotatedZ * sinX;
  const float finalZ = y * sinX + rotatedZ * cosX;

  const float perspective = kCameraDistance / (kCameraDistance - finalZ);
  return {rotatedX * perspective, rotatedY * perspective, finalZ};
}

void OhmWireframe::drawContour(const Point3 *contour, size_t count,
                               float angleX, float angleY, bool closed) {
  const size_t edgeCount = closed ? count : count - 1;
  for (size_t i = 0; i < edgeCount; ++i) {
    const size_t next = (i + 1) % count;
    const Point3 a = project(contour[i].x, contour[i].y, contour[i].z,
                             angleX, angleY);
    const Point3 b = project(contour[next].x, contour[next].y, contour[next].z,
                             angleX, angleY);
    const int centerX = _display.width() / 2;
    const int centerY = _display.height() / 2;
    _display.drawLine(centerX + (int)(a.x * _settings.scale),
                      centerY + (int)(a.y * _settings.scale),
                      centerX + (int)(b.x * _settings.scale),
                      centerY + (int)(b.y * _settings.scale), _settings.color);
  }
}

void OhmWireframe::draw(uint32_t elapsedMillis) {
  const float seconds = elapsedMillis / 1000.0f;
  const float angleX = seconds * _settings.rotationSpeedX;
  const float angleY = seconds * _settings.rotationSpeedY;
  const float frontZ = _settings.depth * 0.5f;
  const float backZ = -frontZ;

  Point3 front[kOhmOutlinePointCount];
  Point3 back[kOhmOutlinePointCount];

  for (size_t i = 0; i < kOhmOutlinePointCount; ++i) {
    front[i] = {kOhmOutline[i].x, kOhmOutline[i].y, frontZ};
    back[i] = {kOhmOutline[i].x, kOhmOutline[i].y, backZ};
  }

  // Outline the front and rear faces of the extruded letter.
  drawContour(front, kOhmOutlinePointCount, angleX, angleY, true);
  drawContour(back, kOhmOutlinePointCount, angleX, angleY, true);

  // Crossbars connect front and back samples, making the extrusion visible.
  const size_t step = _settings.meshStep < 1 ? 1 : _settings.meshStep;
  for (size_t i = 0; i < kOhmOutlinePointCount; i += step) {
    const Point3 f = project(front[i].x, front[i].y, front[i].z,
                             angleX, angleY);
    const Point3 b = project(back[i].x, back[i].y, back[i].z,
                             angleX, angleY);
    const int cx = _display.width() / 2;
    const int cy = _display.height() / 2;
    _display.drawLine(cx + (int)(f.x * _settings.scale),
                      cy + (int)(f.y * _settings.scale),
                      cx + (int)(b.x * _settings.scale),
                      cy + (int)(b.y * _settings.scale), _settings.color);
  }
}

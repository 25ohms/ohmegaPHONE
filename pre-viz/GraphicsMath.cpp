#include "ohmega/GraphicsMath.h"

#include <algorithm>
#include <cmath>

namespace ohmega::graphics {
namespace {
constexpr float kCameraDistance = 230.0f;
}

// rotating about Y axis, we are using R_y(\theta) \vec{v}
Point3 project(float x, float y, float z, float angleY) {
  const float rotatedX = x * cosf(angleY) + z * sinf(angleY);
  const float rotatedZ = -x * sinf(angleY) + z * cosf(angleY);
  const float perspective = kCameraDistance / (kCameraDistance - rotatedZ);
  return {rotatedX * perspective, y * perspective, rotatedZ};
}

ScreenPoint toScreen(const Point3 &point, const Mesh &mesh, int width,
                     int height, float scale, float zScale, float angleY) {
  const Point3 projected = project(
      (point.x - mesh.center.x) * 2.0f, (point.y - mesh.center.y) * 2.0f,
      (point.z - mesh.center.z) * 2.0f * zScale, angleY);
  const float fitScale = (height * 0.36f) / mesh.size;
  return {width / 2 + static_cast<int>(projected.x * fitScale * scale),
          height / 2 + static_cast<int>(projected.y * fitScale * scale)};
}

float sliderValueAt(int mouseX, int trackX, int trackWidth, float minimum,
                    float maximum) {
  const float amount = std::clamp(
      (mouseX - trackX) / static_cast<float>(trackWidth), 0.0f, 1.0f);
  return minimum + amount * (maximum - minimum);
}

int sliderPosition(float value, int trackX, int trackWidth, float minimum,
                   float maximum) {
  const float amount =
      std::clamp((value - minimum) / (maximum - minimum), 0.0f, 1.0f);
  return trackX + static_cast<int>(amount * trackWidth);
}

bool contains(int x, int y, int left, int top, int width, int height) {
  return x >= left && x < left + width && y >= top && y < top + height;
}

} // namespace ohmega::graphics

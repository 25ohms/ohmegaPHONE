#include "OhmWireframe.h"
#include "OhmConfig.h"

#include <math.h>
#include <stdlib.h>

namespace {
// Apply a small perspective projection after rotating around X and Y.
constexpr float kCameraDistance = 230.0f;
} // namespace

OhmWireframe::OhmWireframe(Adafruit_GFX &display)
    : _display(display),
      _settings{OhmConfig::kRenderDefaults.scale,
                OhmConfig::kRenderDefaults.rotationSpeedX,
                OhmConfig::kRenderDefaults.rotationSpeedY,
                OhmConfig::kRenderDefaults.meshStep,
                OhmConfig::kRenderDefaults.color565} {}

bool OhmWireframe::loadGeometry(Stream &input) {
  _meshPointCount = 0;
  _baseVertexCount = 0;
  bool readingTriangles = false;
  float minX = 0.0f, maxX = 0.0f;
  float minY = 0.0f, maxY = 0.0f;
  float minZ = 0.0f, maxZ = 0.0f;

  while (input.available()) {
    String line = input.readStringUntil('\n');
    line.trim();
    if (!line.length()) continue;
    if (line[0] == 'x' || line[0] == 'X') continue;

    char row[96];
    line.toCharArray(row, sizeof(row));
    char *cursor = row;
    char *xText = strsep(&cursor, "\t,");
    char *yText = cursor ? strsep(&cursor, "\t,") : nullptr;
    char *zText = cursor ? strsep(&cursor, "\t,") : nullptr;
    if (!xText || !yText || !zText || _meshPointCount >= kMaxMeshPoints)
      return false;

    Point3 point = {atof(xText), atof(yText), atof(zText)};
    if (_meshPointCount == 0) {
      minX = maxX = point.x;
      minY = maxY = point.y;
      minZ = maxZ = point.z;
    } else {
      if (point.x < minX) minX = point.x;
      if (point.x > maxX) maxX = point.x;
      if (point.y < minY) minY = point.y;
      if (point.y > maxY) maxY = point.y;
      if (point.z < minZ) minZ = point.z;
      if (point.z > maxZ) maxZ = point.z;
    }
    if (point.z > 0.0001f) readingTriangles = true;
    if (!readingTriangles) ++_baseVertexCount;
    _mesh[_meshPointCount++] = point;
  }

  const bool valid = _baseVertexCount > 0 &&
                     (_meshPointCount - _baseVertexCount) >= 3 &&
                     (_meshPointCount - _baseVertexCount) % 3 == 0;
  if (valid) {
    _centerX = (minX + maxX) * 0.5f;
    _centerY = (minY + maxY) * 0.5f;
    _centerZ = (minZ + maxZ) * 0.5f;
    const float modelSize = fmaxf(maxX - minX, maxY - minY);
    if (modelSize <= 0.0f) return false;
    _fitScale = (_display.height() * 0.36f) / modelSize;
  }
  return valid;
}

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

void OhmWireframe::drawSegment(const Point3 &sourceA, const Point3 &sourceB,
                               float angleX, float angleY) {
  const float multiplier = _fitScale * _settings.scale;
  const int screenCenterX = _display.width() / 2;
  const int screenCenterY = _display.height() / 2;

  const Point3 a = project((sourceA.x - _centerX) * 2.0f,
                           (sourceA.y - _centerY) * 2.0f,
                           (sourceA.z - _centerZ) * 2.0f, angleX, angleY);
  const Point3 b = project((sourceB.x - _centerX) * 2.0f,
                           (sourceB.y - _centerY) * 2.0f,
                           (sourceB.z - _centerZ) * 2.0f, angleX, angleY);
  _display.drawLine(screenCenterX + (int)(a.x * multiplier),
                    screenCenterY + (int)(a.y * multiplier),
                    screenCenterX + (int)(b.x * multiplier),
                    screenCenterY + (int)(b.y * multiplier), _settings.color);
}

void OhmWireframe::drawMeshTriangle(const Point3 *triangle, float angleX,
                                    float angleY) {
  for (size_t edge = 0; edge < 3; ++edge)
    drawSegment(triangle[edge], triangle[(edge + 1) % 3], angleX, angleY);
}

void OhmWireframe::draw(uint32_t elapsedMillis) {
  const float seconds = elapsedMillis / 1000.0f;
  const float angleX = seconds * _settings.rotationSpeedX;
  const float angleY = seconds * _settings.rotationSpeedY;
  if (_meshPointCount <= _baseVertexCount) return;
  const size_t triangleCount = (_meshPointCount - _baseVertexCount) / 3;
  const size_t step = _settings.meshStep < 1 ? 1 : _settings.meshStep;
  for (size_t triangleIndex = 0; triangleIndex < triangleCount;
       triangleIndex += step) {
    const Point3 *triangle = &_mesh[_baseVertexCount + triangleIndex * 3];
    drawMeshTriangle(triangle, angleX, angleY);

    Point3 rearTriangle[3] = {triangle[0], triangle[1], triangle[2]};
    for (size_t vertex = 0; vertex < 3; ++vertex) {
      rearTriangle[vertex].z = 0.0f;
    }
    drawMeshTriangle(rearTriangle, angleX, angleY);
    for (size_t vertex = 0; vertex < 3; ++vertex) {
      drawSegment(triangle[vertex], rearTriangle[vertex], angleX, angleY);
    }
  }
}

#pragma once

#include <cstddef>
#include <vector>

namespace ohmega {

struct Point3 {
  float x;
  float y;
  float z;
};

struct Mesh {
  std::vector<Point3> coordinates;
  size_t baseVertexCount = 0;
  Point3 center{0.0f, 0.0f, 0.0f};
  float size = 0.0f;

  size_t triangleCount() const {
    if (baseVertexCount > coordinates.size()) return 0;
    return (coordinates.size() - baseVertexCount) / 3;
  }
  bool valid() const {
    if (baseVertexCount > coordinates.size()) return false;
    const size_t points = coordinates.size() - baseVertexCount;
    return baseVertexCount > 0 && points >= 3 && points % 3 == 0 && size > 0;
  }
};

} // namespace ohmega

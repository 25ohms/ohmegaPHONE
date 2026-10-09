#include "ohmega/CsvMeshLoader.h"
#include "OhmConfig.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

namespace ohmega {
namespace {
bool parseCsv(const std::string &path, Mesh &mesh) {
  std::ifstream input(path);
  if (!input) return false;

  Mesh parsed;
  bool readingTriangles = false;
  float minX = 0.0f, maxX = 0.0f;
  float minY = 0.0f, maxY = 0.0f;
  float minZ = 0.0f, maxZ = 0.0f;
  std::string line;
  while (std::getline(input, line)) {
    std::replace(line.begin(), line.end(), '\t', ' ');
    std::replace(line.begin(), line.end(), ',', ' ');
    std::istringstream row(line);
    Point3 point{};
    if (!(row >> point.x >> point.y >> point.z)) continue;

    if (parsed.coordinates.empty()) {
      minX = maxX = point.x;
      minY = maxY = point.y;
      minZ = maxZ = point.z;
    } else {
      minX = std::min(minX, point.x);
      maxX = std::max(maxX, point.x);
      minY = std::min(minY, point.y);
      maxY = std::max(maxY, point.y);
      minZ = std::min(minZ, point.z);
      maxZ = std::max(maxZ, point.z);
    }
    if (point.z > 0.0001f) readingTriangles = true;
    if (!readingTriangles) ++parsed.baseVertexCount;
    parsed.coordinates.push_back(point);
  }

  parsed.center = {(minX + maxX) * 0.5f, (minY + maxY) * 0.5f,
                   (minZ + maxZ) * 0.5f};
  parsed.size = std::max(maxX - minX, maxY - minY);
  if (!parsed.valid()) return false;
  mesh = std::move(parsed);
  return true;
}
} // namespace

bool CsvMeshLoader::load(const std::string &path) {
  _mesh = Mesh{};
  _loadedPath.clear();
  _error.clear();
  if (!parseCsv(path, _mesh)) {
    _mesh = Mesh{};
    _error = "Could not read a valid coordinate mesh from " + path;
    return false;
  }
  _loadedPath = path;
  return true;
}

bool CsvMeshLoader::loadProjectCsv(const std::string &executablePath) {
  namespace fs = std::filesystem;
  std::vector<fs::path> candidates;
  const fs::path executable = fs::absolute(executablePath);
  const fs::path executableDirectory = executable.parent_path();
  candidates.push_back(executableDirectory / "../../data" /
                        OhmConfig::kCoordinatesFileName);
  candidates.push_back(executableDirectory / "../data" /
                        OhmConfig::kCoordinatesFileName);
  candidates.push_back(fs::current_path() / "data" /
                        OhmConfig::kCoordinatesFileName);
  candidates.push_back(fs::current_path() / "../data" /
                        OhmConfig::kCoordinatesFileName);

  for (const fs::path &candidate : candidates) {
    if (load(candidate.lexically_normal().string())) return true;
  }
  _error = "Could not find a valid data/" +
           std::string(OhmConfig::kCoordinatesFileName) +
           " beside the project or visualizer executable";
  return false;
}

} // namespace ohmega

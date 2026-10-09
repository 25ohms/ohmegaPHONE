#pragma once

#include "ohmega/Mesh.h"

#include <string>

namespace ohmega {

class CsvMeshLoader {
public:
  bool load(const std::string &path);
  bool loadProjectCsv(const std::string &executablePath);

  const Mesh &mesh() const { return _mesh; }
  const std::string &loadedPath() const { return _loadedPath; }
  const std::string &error() const { return _error; }

private:
  Mesh _mesh;
  std::string _loadedPath;
  std::string _error;
};

} // namespace ohmega

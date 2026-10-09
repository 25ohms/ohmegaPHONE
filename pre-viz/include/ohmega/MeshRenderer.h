#pragma once

#include "ohmega/Mesh.h"

#include <SDL.h>

#include <cstdint>

namespace ohmega {

struct MeshViewSettings {
  float scale;
  float zScale;
  float angleY;
  int meshStep;
  uint16_t color565;
};

class MeshRenderer {
public:
  void draw(SDL_Renderer *renderer, const Mesh &mesh, int width, int height,
            const MeshViewSettings &settings) const;
};

} // namespace ohmega

#include "ohmega/MeshRenderer.h"

#include "ohmega/GraphicsMath.h"
#include "ohmega/UiDrawing.h"

#include <algorithm>

namespace ohmega {

void MeshRenderer::draw(SDL_Renderer *renderer, const Mesh &mesh, int width,
                        int height,
                        const MeshViewSettings &settings) const {
  const SDL_Color color = ui::from565(settings.color565);
  ui::setColor(renderer, color.r, color.g, color.b);
  const auto drawLine = [&](const Point3 &a, const Point3 &b) {
    const graphics::ScreenPoint first = graphics::toScreen(
        a, mesh, width, height, settings.scale, settings.zScale,
        settings.angleY);
    const graphics::ScreenPoint second = graphics::toScreen(
        b, mesh, width, height, settings.scale, settings.zScale,
        settings.angleY);
    SDL_RenderDrawLine(renderer, first.x, first.y, second.x, second.y);
  };

  const size_t step = static_cast<size_t>(std::max(1, settings.meshStep));
  for (size_t index = 0; index < mesh.triangleCount(); index += step) {
    const Point3 *triangle = &mesh.coordinates[mesh.baseVertexCount + index * 3];
    Point3 rear[3] = {triangle[0], triangle[1], triangle[2]};
    for (size_t edge = 0; edge < 3; ++edge) {
      drawLine(triangle[edge], triangle[(edge + 1) % 3]);
      rear[edge].z = 0.0f;
    }
    for (size_t edge = 0; edge < 3; ++edge)
      drawLine(rear[edge], rear[(edge + 1) % 3]);
    for (size_t vertex = 0; vertex < 3; ++vertex)
      drawLine(triangle[vertex], rear[vertex]);
  }
}

} // namespace ohmega

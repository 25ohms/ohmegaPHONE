#pragma once

#include "ohmega/Mesh.h"
#include "ohmega/MeshRenderer.h"
#include "ohmega/Screen.h"
#include "ohmega/Slider.h"
#include "OhmConfig.h"

#include <array>
#include <cstdint>

namespace ohmega {

class PreVizApp {
public:
  PreVizApp(const Mesh &mesh, Screen &screen,
            std::array<Slider, 5> &sliders);
  int run();

private:
  struct Settings {
    uint16_t color565 = OhmConfig::kRenderDefaults.color565;
    bool landscape = true;
    bool animate = true;
  };

  void renderMesh();
  void drawInterface();
  void handleMouseDown(int x, int y);
  void updateSlider(int index, int mouseX);
  void resetSettings();
  void drawButton(const char *label, int x, int y, int width,
                  bool selected = false);

  const Mesh &_mesh;
  Screen &_screen;
  std::array<Slider, 5> &_sliders;
  Settings _settings;
  MeshRenderer _meshRenderer;
  bool _dragging = false;
  int _draggedSlider = -1;
  float _angleRadians = 0.0f;
  uint32_t _previousFrameTicks = 0;
};

} // namespace ohmega

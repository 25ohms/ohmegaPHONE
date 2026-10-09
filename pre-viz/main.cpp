#include "ohmega/CsvMeshLoader.h"
#include "ohmega/PreVizApp.h"
#include "ohmega/Screen.h"
#include "ohmega/Slider.h"
#include "OhmConfig.h"

#include <array>
#include <cstdio>

int main(int argc, char **argv) {
  ohmega::CsvMeshLoader csvLoader;
  const char *executablePath = (argc > 0 && argv[0]) ? argv[0] : ".";
  if (!csvLoader.loadProjectCsv(executablePath)) {
    std::fprintf(stderr, "%s\n", csvLoader.error().c_str());
    return 1;
  }

  ohmega::Screen screen;
  std::array<ohmega::Slider, 5> sliders = {{
      {"SCALE", 80, 0.60f, 2.00f, OhmConfig::kRenderDefaults.scale, 2},
      {"Z SCALE", 120, 0.0f, 2.0f,
       OhmConfig::kRenderDefaults.previewZScale, 1},
      {"Y SPEED", 160, 0.0f, 2.5f,
       OhmConfig::kRenderDefaults.rotationSpeedY, 2},
      {"ANGLE", 200, 0.0f, 360.0f,
       OhmConfig::kRenderDefaults.angleDegrees, 0},
      {"MESH STEP", 240, 1.0f, 8.0f,
       static_cast<float>(OhmConfig::kRenderDefaults.meshStep), 0, true},
  }};
  ohmega::PreVizApp app(csvLoader.mesh(), screen, sliders);
  return app.run();
}

#include "ohmega/PreVizApp.h"

#include "ohmega/GraphicsMath.h"
#include "ohmega/UiDrawing.h"
#include "OhmConfig.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace ohmega {
namespace {
constexpr int kWindowWidth = 960;
constexpr int kWindowHeight = 400;
constexpr int kPreviewX = 24;
constexpr int kPreviewY = 48;
constexpr int kPanelX = 370;
constexpr int kTrackX = 480;
constexpr int kTrackWidth = 250;
constexpr float kPi = 3.14159265358979323846f;

struct ColorChoice {
  const char *name;
  uint16_t rgb565;
};

constexpr ColorChoice kColors[] = {
    {"CYAN", OhmConfig::kRenderDefaults.color565},
    {"GREEN", 0x07E0}, {"WHITE", 0xFFFF},
    {"AMBER", 0xFD20}};

} // namespace

PreVizApp::PreVizApp(const Mesh &mesh, Screen &screen,
                     std::array<Slider, 5> &sliders)
    : _mesh(mesh), _screen(screen), _sliders(sliders) {}

int PreVizApp::run() {
  if (!_screen.open("ohmegaPHONE pre-viz", kWindowWidth, kWindowHeight))
    return 1;

  bool running = true;
  _previousFrameTicks = SDL_GetTicks();
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      } else if (event.type == SDL_MOUSEBUTTONDOWN &&
                 event.button.button == SDL_BUTTON_LEFT) {
        handleMouseDown(event.button.x, event.button.y);
      } else if (event.type == SDL_MOUSEBUTTONUP &&
                 event.button.button == SDL_BUTTON_LEFT) {
        _dragging = false;
        _draggedSlider = -1;
      } else if (event.type == SDL_MOUSEMOTION && _dragging) {
        updateSlider(_draggedSlider, event.motion.x);
      } else if (event.type == SDL_KEYDOWN &&
                 event.key.keysym.sym == SDLK_ESCAPE) {
        running = false;
      }
    }

    const uint32_t now = SDL_GetTicks();
    const float deltaSeconds = (now - _previousFrameTicks) / 1000.0f;
    _previousFrameTicks = now;
    if (_settings.animate)
      _angleRadians += _sliders[2].value() * deltaSeconds;
    float angleDegrees = std::fmod(_angleRadians * 180.0f / kPi, 360.0f);
    if (angleDegrees < 0) angleDegrees += 360.0f;
    _sliders[3].setValue(angleDegrees);

    renderMesh();
    drawInterface();
    SDL_RenderPresent(_screen.renderer());
    SDL_Delay(16);
  }

  return 0;
}

void PreVizApp::renderMesh() {
  const int requestedWidth = _settings.landscape ? 320 : 240;
  const int requestedHeight = _settings.landscape ? 240 : 320;
  SDL_Texture *preview = _screen.resizePreview(requestedWidth, requestedHeight);
  if (!preview) return;

  SDL_Renderer *renderer = _screen.renderer();
  SDL_SetRenderTarget(renderer, preview);
  ui::setColor(renderer, 0, 0, 0);
  SDL_RenderClear(renderer);
  _meshRenderer.draw(renderer, _mesh, requestedWidth, requestedHeight,
                     {_sliders[0].value(), _sliders[1].value(), _angleRadians,
                      _sliders[4].integer(), _settings.color565});
  SDL_SetRenderTarget(renderer, nullptr);
}

void PreVizApp::drawButton(const char *label, int x, int y, int width,
                           bool selected) {
  const SDL_Color fill = selected ? SDL_Color{0, 105, 113, 255}
                                  : SDL_Color{31, 43, 51, 255};
  const SDL_Color edge = selected ? SDL_Color{0, 215, 225, 255}
                                  : SDL_Color{67, 84, 93, 255};
  SDL_Renderer *renderer = _screen.renderer();
  ui::drawRect(renderer, x, y, width, 22, fill);
  ui::drawRect(renderer, x, y, width, 22, edge, false);
  ui::drawText(renderer, label, x + 5, y + 8, {225, 235, 240, 255});
}

void PreVizApp::drawInterface() {
  SDL_Renderer *renderer = _screen.renderer();
  const int displayWidth = _screen.previewWidth();
  const int displayHeight = _screen.previewHeight();
  ui::setColor(renderer, 15, 20, 24);
  SDL_RenderClear(renderer);

  ui::drawRect(renderer, kPreviewX - 2, kPreviewY - 2, displayWidth + 4,
               displayHeight + 4, {59, 76, 84, 255}, false);
  if (_screen.previewTexture()) {
    SDL_Rect target{kPreviewX, kPreviewY, displayWidth, displayHeight};
    _screen.drawPreview(target);
  }
  char resolution[48];
  std::snprintf(resolution, sizeof(resolution), "%d X %d PIXELS  1:1",
                displayWidth, displayHeight);
  ui::drawText(renderer, resolution, kPreviewX, 20, {130, 157, 166, 255});
  ui::drawText(renderer, "OHMEGA PRE-VIZ", kPanelX, 23, {0, 215, 225, 255}, 2);
  ui::drawRect(renderer, kPanelX, 38, 520, 1, {52, 72, 80, 255});

  for (const Slider &slider : _sliders)
    slider.draw(renderer, kPanelX, kTrackX, kTrackWidth);

  ui::drawText(renderer, "COLOR", kPanelX, 286, {160, 180, 188, 255});
  for (int i = 0; i < 4; ++i) {
    drawButton(kColors[i].name, 430 + i * 77, 280, 68,
               _settings.color565 == kColors[i].rgb565);
  }
  drawButton(_settings.landscape ? "LAND" : "PORT", 370, 330, 66,
             _settings.landscape);
  drawButton(_settings.animate ? "PAUSE" : "PLAY", 445, 330, 70,
             _settings.animate);
  drawButton("RESET", 524, 330, 64, false);
  ui::drawText(renderer, "Y ROTATION ONLY", 610, 338,
               {130, 157, 166, 255});
}

void PreVizApp::updateSlider(int index, int mouseX) {
  if (index < 0 || index >= static_cast<int>(_sliders.size())) return;
  _sliders[index].setFromX(mouseX, kTrackX, kTrackWidth);
  if (index == 3) _angleRadians = _sliders[index].value() * kPi / 180.0f;
}

void PreVizApp::resetSettings() {
  _settings = Settings{};
  _angleRadians = 0.0f;
  _sliders[0].setValue(OhmConfig::kRenderDefaults.scale);
  _sliders[1].setValue(OhmConfig::kRenderDefaults.previewZScale);
  _sliders[2].setValue(OhmConfig::kRenderDefaults.rotationSpeedY);
  _sliders[3].setValue(OhmConfig::kRenderDefaults.angleDegrees);
  _sliders[4].setValue(OhmConfig::kRenderDefaults.meshStep);
}

void PreVizApp::handleMouseDown(int x, int y) {
  for (size_t index = 0; index < _sliders.size(); ++index) {
    if (_sliders[index].hitTest(x, y, kTrackX, kTrackWidth)) {
      _draggedSlider = static_cast<int>(index);
      _dragging = true;
      updateSlider(_draggedSlider, x);
      return;
    }
  }
  for (int i = 0; i < 4; ++i) {
    if (graphics::contains(x, y, 430 + i * 77, 280, 68, 22)) {
      _settings.color565 = kColors[i].rgb565;
      return;
    }
  }
  if (graphics::contains(x, y, 370, 330, 66, 22)) {
    _settings.landscape = !_settings.landscape;
  } else if (graphics::contains(x, y, 445, 330, 70, 22)) {
    _settings.animate = !_settings.animate;
  } else if (graphics::contains(x, y, 524, 330, 64, 22)) {
    resetSettings();
  }
}

} // namespace ohmega

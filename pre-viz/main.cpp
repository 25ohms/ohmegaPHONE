#include <SDL.h>

#include "OhmGeometry.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>

namespace {
constexpr int kWindowWidth = 960;
constexpr int kWindowHeight = 400;
constexpr int kPreviewX = 24;
constexpr int kPreviewY = 48;
constexpr int kPanelX = 370;
constexpr int kTrackX = 480;
constexpr int kTrackWidth = 250;
constexpr float kCameraDistance = 230.0f;
constexpr float kPi = 3.14159265358979323846f;

struct Settings {
  float scale = 1.25f;
  float depth = 18.0f;
  float rotationSpeedY = 0.8f; // radians per second, matching the firmware
  float angleDegrees = 0.0f;
  int meshStep = 2;
  uint16_t color565 = 0x07FF;
  bool landscape = true;
  bool animate = true;
};

struct Point3 {
  float x;
  float y;
  float z;
};

struct ColorChoice {
  const char *name;
  uint16_t rgb565;
};

constexpr ColorChoice kColors[] = {
    {"CYAN", 0x07FF}, {"GREEN", 0x07E0}, {"WHITE", 0xFFFF},
    {"AMBER", 0xFD20}};

struct Glyph {
  char character;
  uint8_t rows[5]; // Three lit-pixel bits per row.
};

constexpr Glyph kFont[] = {
    {'A', {2, 5, 7, 5, 5}}, {'B', {6, 5, 6, 5, 6}},
    {'C', {3, 4, 4, 4, 3}}, {'D', {6, 5, 5, 5, 6}},
    {'E', {7, 4, 6, 4, 7}}, {'F', {7, 4, 6, 4, 4}},
    {'G', {3, 4, 5, 5, 3}}, {'H', {5, 5, 7, 5, 5}},
    {'I', {7, 2, 2, 2, 7}}, {'J', {1, 1, 1, 5, 2}},
    {'K', {5, 5, 6, 5, 5}}, {'L', {4, 4, 4, 4, 7}},
    {'M', {5, 7, 7, 5, 5}}, {'N', {5, 7, 7, 7, 5}},
    {'O', {2, 5, 5, 5, 2}}, {'P', {6, 5, 6, 4, 4}},
    {'Q', {2, 5, 5, 7, 3}}, {'R', {6, 5, 6, 5, 5}},
    {'S', {3, 4, 2, 1, 6}}, {'T', {7, 2, 2, 2, 2}},
    {'U', {5, 5, 5, 5, 7}}, {'V', {5, 5, 5, 5, 2}},
    {'W', {5, 5, 7, 7, 5}}, {'X', {5, 5, 2, 5, 5}},
    {'Y', {5, 5, 2, 2, 2}}, {'Z', {7, 1, 2, 4, 7}},
    {'0', {7, 5, 5, 5, 7}}, {'1', {2, 6, 2, 2, 7}},
    {'2', {6, 1, 2, 4, 7}}, {'3', {6, 1, 2, 1, 6}},
    {'4', {5, 5, 7, 1, 1}}, {'5', {7, 4, 6, 1, 6}},
    {'6', {3, 4, 6, 5, 2}}, {'7', {7, 1, 2, 2, 2}},
    {'8', {2, 5, 2, 5, 2}}, {'9', {2, 5, 3, 1, 6}},
    {'.', {0, 0, 0, 0, 2}}, {':', {0, 2, 0, 2, 0}},
    {'-', {0, 0, 7, 0, 0}}, {'/', {1, 1, 2, 4, 4}},
    {'+', {0, 2, 7, 2, 0}}, {' ', {0, 0, 0, 0, 0}}};

Settings settings;
SDL_Window *window = nullptr;
SDL_Renderer *renderer = nullptr;
SDL_Texture *displayTexture = nullptr;
int displayWidth = 320;
int displayHeight = 240;
int textureWidth = 0;
int textureHeight = 0;
bool dragging = false;
int draggedSlider = -1;
float angleRadians = 0.0f;
uint32_t previousFrameTicks = 0;

void setColor(uint8_t r, uint8_t g, uint8_t b) {
  SDL_SetRenderDrawColor(renderer, r, g, b, 255);
}

void drawRect(int x, int y, int w, int h, SDL_Color color, bool filled = true) {
  SDL_Rect rect{x, y, w, h};
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  if (filled) SDL_RenderFillRect(renderer, &rect);
  else SDL_RenderDrawRect(renderer, &rect);
}

void drawText(const char *text, int x, int y, SDL_Color color,
              int pixelScale = 1) {
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  for (const char *it = text; *it; ++it, x += 4 * pixelScale) {
    const char c = (*it >= 'a' && *it <= 'z') ? *it - 'a' + 'A' : *it;
    const Glyph *glyph = nullptr;
    for (const Glyph &candidate : kFont) {
      if (candidate.character == c) {
        glyph = &candidate;
        break;
      }
    }
    if (!glyph) continue;
    for (int row = 0; row < 5; ++row) {
      for (int column = 0; column < 3; ++column) {
        if (glyph->rows[row] & (1 << (2 - column))) {
          SDL_Rect pixel{x + column * pixelScale, y + row * pixelScale,
                         pixelScale, pixelScale};
          SDL_RenderFillRect(renderer, &pixel);
        }
      }
    }
  }
}

SDL_Color from565(uint16_t color) {
  const uint8_t r5 = (color >> 11) & 0x1F;
  const uint8_t g6 = (color >> 5) & 0x3F;
  const uint8_t b5 = color & 0x1F;
  return {static_cast<uint8_t>((r5 * 255) / 31),
          static_cast<uint8_t>((g6 * 255) / 63),
          static_cast<uint8_t>((b5 * 255) / 31), 255};
}

Point3 project(float x, float y, float z, float angleY) {
  // The firmware currently rotates only around Y.
  const float rotatedX = x * cosf(angleY) + z * sinf(angleY);
  const float rotatedZ = -x * sinf(angleY) + z * cosf(angleY);
  const float perspective = kCameraDistance / (kCameraDistance - rotatedZ);
  return {rotatedX * perspective, y * perspective, rotatedZ};
}

void drawProjectedLine(const Point3 &a, const Point3 &b, float angleY) {
  const Point3 pa = project(a.x, a.y, a.z, angleY);
  const Point3 pb = project(b.x, b.y, b.z, angleY);
  const int cx = displayWidth / 2;
  const int cy = displayHeight / 2;
  SDL_RenderDrawLine(renderer,
                     cx + static_cast<int>(pa.x * settings.scale),
                     cy + static_cast<int>(pa.y * settings.scale),
                     cx + static_cast<int>(pb.x * settings.scale),
                     cy + static_cast<int>(pb.y * settings.scale));
}

void renderOhm() {
  const int requestedWidth = settings.landscape ? 320 : 240;
  const int requestedHeight = settings.landscape ? 240 : 320;
  if (!displayTexture || textureWidth != requestedWidth ||
      textureHeight != requestedHeight) {
    if (displayTexture) SDL_DestroyTexture(displayTexture);
    displayWidth = textureWidth = requestedWidth;
    displayHeight = textureHeight = requestedHeight;
    displayTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                       SDL_TEXTUREACCESS_TARGET,
                                       displayWidth, displayHeight);
  }
  if (!displayTexture) return;

  SDL_SetRenderTarget(renderer, displayTexture);
  setColor(0, 0, 0);
  SDL_RenderClear(renderer);
  const SDL_Color color = from565(settings.color565);
  setColor(color.r, color.g, color.b);

  const float frontZ = settings.depth * 0.5f;
  const float backZ = -frontZ;
  std::array<Point3, kOhmOutlinePointCount> front{};
  std::array<Point3, kOhmOutlinePointCount> back{};
  for (size_t i = 0; i < kOhmOutlinePointCount; ++i) {
    front[i] = {kOhmOutline[i].x, kOhmOutline[i].y, frontZ};
    back[i] = {kOhmOutline[i].x, kOhmOutline[i].y, backZ};
  }

  const float angleY = angleRadians;
  for (size_t i = 0; i < kOhmOutlinePointCount; ++i) {
    const size_t next = (i + 1) % kOhmOutlinePointCount;
    drawProjectedLine(front[i], front[next], angleY);
    drawProjectedLine(back[i], back[next], angleY);
  }
  for (size_t i = 0; i < kOhmOutlinePointCount; i += settings.meshStep) {
    drawProjectedLine(front[i], back[i], angleY);
  }
  SDL_SetRenderTarget(renderer, nullptr);
}

struct Slider {
  const char *label;
  int y;
  float minimum;
  float maximum;
  float *value;
  int precision;
};

void setSlider(int index, int mouseX) {
  const std::array<Slider, 4> controls = {{{"SCALE", 80, 0.60f, 2.00f,
                                             &settings.scale, 2},
                                            {"DEPTH", 120, 0.0f, 40.0f,
                                             &settings.depth, 1},
                                            {"Y SPEED", 160, 0.0f, 2.5f,
                                             &settings.rotationSpeedY, 2},
                                            {"ANGLE", 200, 0.0f, 360.0f,
                                             &settings.angleDegrees, 0}}};
  if (index == 4) {
    const float amount = std::clamp((mouseX - kTrackX) /
                                        static_cast<float>(kTrackWidth),
                                    0.0f, 1.0f);
    settings.meshStep = 1 + static_cast<int>(std::round(amount * 7.0f));
    return;
  }
  if (index < 0 || index >= static_cast<int>(controls.size())) return;
  const Slider &slider = controls[index];
  const float amount = std::clamp((mouseX - kTrackX) /
                                      static_cast<float>(kTrackWidth),
                                  0.0f, 1.0f);
  *slider.value = slider.minimum + amount * (slider.maximum - slider.minimum);
  if (index == 3) angleRadians = settings.angleDegrees * kPi / 180.0f;
}

void drawSlider(const Slider &slider, int index) {
  drawText(slider.label, kPanelX, slider.y - 3, {160, 180, 188, 255}, 1);
  drawRect(kTrackX, slider.y, kTrackWidth, 4, {42, 55, 63, 255});
  float current = index == 4 ? static_cast<float>(settings.meshStep)
                             : *slider.value;
  float amount = (current - slider.minimum) / (slider.maximum - slider.minimum);
  amount = std::clamp(amount, 0.0f, 1.0f);
  const int knobX = kTrackX + static_cast<int>(amount * kTrackWidth);
  drawRect(kTrackX, slider.y, knobX - kTrackX, 4, {0, 215, 225, 255});
  drawRect(knobX - 4, slider.y - 4, 9, 12, {0, 215, 225, 255});

  char value[20];
  if (index == 4) std::snprintf(value, sizeof(value), "%d", settings.meshStep);
  else std::snprintf(value, sizeof(value), "%.*f", slider.precision, current);
  drawText(value, kTrackX + kTrackWidth + 12, slider.y - 3,
           {220, 230, 235, 255});
}

void drawButton(const char *label, int x, int y, int width, bool selected = false) {
  const SDL_Color fill = selected ? SDL_Color{0, 105, 113, 255}
                                  : SDL_Color{31, 43, 51, 255};
  const SDL_Color edge = selected ? SDL_Color{0, 215, 225, 255}
                                  : SDL_Color{67, 84, 93, 255};
  drawRect(x, y, width, 22, fill);
  drawRect(x, y, width, 22, edge, false);
  drawText(label, x + 5, y + 8, {225, 235, 240, 255});
}

void drawInterface() {
  setColor(15, 20, 24);
  SDL_RenderClear(renderer);

  drawRect(kPreviewX - 2, kPreviewY - 2, displayWidth + 4, displayHeight + 4,
           {59, 76, 84, 255}, false);
  if (displayTexture) {
    SDL_Rect target{kPreviewX, kPreviewY, displayWidth, displayHeight};
    SDL_RenderCopy(renderer, displayTexture, nullptr, &target);
  }
  char resolution[48];
  std::snprintf(resolution, sizeof(resolution), "%d X %d PIXELS  1:1",
                displayWidth, displayHeight);
  drawText(resolution, kPreviewX, 20, {130, 157, 166, 255});

  drawText("OHMEGA PRE-VIZ", kPanelX, 23, {0, 215, 225, 255}, 2);
  drawRect(kPanelX, 38, 520, 1, {52, 72, 80, 255});

  const std::array<Slider, 5> controls = {{{"SCALE", 80, 0.60f, 2.00f,
                                             &settings.scale, 2},
                                            {"DEPTH", 120, 0.0f, 40.0f,
                                             &settings.depth, 1},
                                            {"Y SPEED", 160, 0.0f, 2.5f,
                                             &settings.rotationSpeedY, 2},
                                            {"ANGLE", 200, 0.0f, 360.0f,
                                             &settings.angleDegrees, 0},
                                            {"MESH STEP", 240, 1.0f, 8.0f,
                                             nullptr, 0}}};
  for (int i = 0; i < static_cast<int>(controls.size()); ++i)
    drawSlider(controls[i], i);

  drawText("COLOR", kPanelX, 286, {160, 180, 188, 255});
  for (int i = 0; i < 4; ++i) {
    drawButton(kColors[i].name, 430 + i * 77, 280, 68,
               settings.color565 == kColors[i].rgb565);
  }

  drawButton(settings.landscape ? "LAND" : "PORT", 370, 330, 66,
             settings.landscape);
  drawButton(settings.animate ? "PAUSE" : "PLAY", 445, 330, 70,
             settings.animate);
  drawButton("RESET", 524, 330, 64, false);
  drawText("Y ROTATION ONLY", 610, 338, {130, 157, 166, 255});
}

bool pointIn(int x, int y, int left, int top, int width, int height) {
  return x >= left && x < left + width && y >= top && y < top + height;
}

void resetSettings() {
  settings = Settings{};
  angleRadians = 0.0f;
}

void handleMouseDown(int x, int y) {
  if (x >= kTrackX && x <= kTrackX + kTrackWidth) {
    const int row = (y - 74) / 40;
    if (row >= 0 && row < 5) {
      draggedSlider = row;
      dragging = true;
      setSlider(row, x);
      return;
    }
  }
  for (int i = 0; i < 4; ++i) {
    if (pointIn(x, y, 430 + i * 77, 280, 68, 22)) {
      settings.color565 = kColors[i].rgb565;
      return;
    }
  }
  if (pointIn(x, y, 370, 330, 66, 22)) {
    settings.landscape = !settings.landscape;
  } else if (pointIn(x, y, 445, 330, 70, 22)) {
    settings.animate = !settings.animate;
  } else if (pointIn(x, y, 524, 330, 64, 22)) {
    resetSettings();
  }
}

} // namespace

int main(int, char **) {
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
    return 1;
  }
  window = SDL_CreateWindow("ohmegaPHONE pre-viz", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, kWindowWidth,
                            kWindowHeight, SDL_WINDOW_SHOWN);
  if (!window) {
    std::fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED |
                                             SDL_RENDERER_PRESENTVSYNC |
                                             SDL_RENDERER_TARGETTEXTURE);
  if (!renderer) {
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE |
                                               SDL_RENDERER_TARGETTEXTURE);
  }
  if (!renderer) {
    std::fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  bool running = true;
  previousFrameTicks = SDL_GetTicks();
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) running = false;
      else if (event.type == SDL_MOUSEBUTTONDOWN &&
               event.button.button == SDL_BUTTON_LEFT)
        handleMouseDown(event.button.x, event.button.y);
      else if (event.type == SDL_MOUSEBUTTONUP &&
               event.button.button == SDL_BUTTON_LEFT) {
        dragging = false;
        draggedSlider = -1;
      } else if (event.type == SDL_MOUSEMOTION && dragging) {
        setSlider(draggedSlider, event.motion.x);
      } else if (event.type == SDL_KEYDOWN &&
                 event.key.keysym.sym == SDLK_ESCAPE)
        running = false;
    }

    const uint32_t now = SDL_GetTicks();
    const float deltaSeconds = (now - previousFrameTicks) / 1000.0f;
    previousFrameTicks = now;
    if (settings.animate)
      angleRadians += settings.rotationSpeedY * deltaSeconds;
    settings.angleDegrees = std::fmod(angleRadians * 180.0f / kPi, 360.0f);
    if (settings.angleDegrees < 0) settings.angleDegrees += 360.0f;

    renderOhm();
    drawInterface();
    SDL_RenderPresent(renderer);
    SDL_Delay(16);
  }

  if (displayTexture) SDL_DestroyTexture(displayTexture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}

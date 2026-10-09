#include "ohmega/Screen.h"
#include "OhmConfig.h"

#include <cstdio>

namespace ohmega {

Screen::~Screen() { close(); }

bool Screen::open(const char *title, int windowWidth, int windowHeight) {
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    std::fprintf(stderr, "SDL initialization failed: %s\n", SDL_GetError());
    return false;
  }
  _sdlInitialized = true;

  _window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, windowWidth,
                             windowHeight, SDL_WINDOW_SHOWN);
  if (!_window) {
    std::fprintf(stderr, "Window creation failed: %s\n", SDL_GetError());
    return false;
  }
  _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_ACCELERATED |
                                                SDL_RENDERER_PRESENTVSYNC |
                                                SDL_RENDERER_TARGETTEXTURE);
  if (!_renderer) {
    _renderer = SDL_CreateRenderer(_window, -1, SDL_RENDERER_SOFTWARE |
                                                  SDL_RENDERER_TARGETTEXTURE);
  }
  if (!_renderer) {
    std::fprintf(stderr, "Renderer creation failed: %s\n", SDL_GetError());
    return false;
  }
  return true;
}

SDL_Texture *Screen::resizePreview(int width, int height) {
  if (_previewTexture && _previewWidth == width && _previewHeight == height)
    return _previewTexture;
  if (_previewTexture) SDL_DestroyTexture(_previewTexture);
  _previewTexture = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGBA8888,
                                      SDL_TEXTUREACCESS_TARGET, width, height);
  if (_previewTexture) {
    _previewWidth = width;
    _previewHeight = height;
  }
  return _previewTexture;
}

void Screen::drawPreview(const SDL_Rect &destination) const {
  const double rotation = OhmConfig::kDisplayRotation == 1 ? 180.0 : 0.0;
  SDL_RenderCopyEx(_renderer, _previewTexture, nullptr, &destination, rotation,
                   nullptr, SDL_FLIP_NONE);
}

void Screen::close() {
  if (_previewTexture) SDL_DestroyTexture(_previewTexture);
  if (_renderer) SDL_DestroyRenderer(_renderer);
  if (_window) SDL_DestroyWindow(_window);
  _previewTexture = nullptr;
  _renderer = nullptr;
  _window = nullptr;
  if (_sdlInitialized) SDL_Quit();
  _sdlInitialized = false;
}

} // namespace ohmega

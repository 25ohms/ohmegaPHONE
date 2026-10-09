#pragma once

#include <SDL.h>

namespace ohmega {

class Screen {
public:
  Screen() = default;
  ~Screen();

  Screen(const Screen &) = delete;
  Screen &operator=(const Screen &) = delete;

  bool open(const char *title, int windowWidth, int windowHeight);
  SDL_Renderer *renderer() const { return _renderer; }
  SDL_Texture *resizePreview(int width, int height);
  SDL_Texture *previewTexture() const { return _previewTexture; }
  void drawPreview(const SDL_Rect &destination) const;
  int previewWidth() const { return _previewWidth; }
  int previewHeight() const { return _previewHeight; }

private:
  void close();

  SDL_Window *_window = nullptr;
  SDL_Renderer *_renderer = nullptr;
  SDL_Texture *_previewTexture = nullptr;
  int _previewWidth = 0;
  int _previewHeight = 0;
  bool _sdlInitialized = false;
};

} // namespace ohmega

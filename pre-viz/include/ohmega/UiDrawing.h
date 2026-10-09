#pragma once

#include <SDL.h>

#include <cstdint>

namespace ohmega::ui {

void setColor(SDL_Renderer *renderer, uint8_t red, uint8_t green, uint8_t blue);
void drawRect(SDL_Renderer *renderer, int x, int y, int width, int height,
              SDL_Color color, bool filled = true);
void drawText(SDL_Renderer *renderer, const char *text, int x, int y,
              SDL_Color color, int pixelScale = 1);
SDL_Color from565(uint16_t color);

} // namespace ohmega::ui

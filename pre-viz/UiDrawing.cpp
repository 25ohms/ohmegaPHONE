#include "ohmega/UiDrawing.h"

namespace ohmega::ui {
namespace {
struct Glyph {
  char character;
  uint8_t rows[5];
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
} // namespace

void setColor(SDL_Renderer *renderer, uint8_t red, uint8_t green,
              uint8_t blue) {
  SDL_SetRenderDrawColor(renderer, red, green, blue, 255);
}

void drawRect(SDL_Renderer *renderer, int x, int y, int width, int height,
              SDL_Color color, bool filled) {
  SDL_Rect rect{x, y, width, height};
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  if (filled) SDL_RenderFillRect(renderer, &rect);
  else SDL_RenderDrawRect(renderer, &rect);
}

void drawText(SDL_Renderer *renderer, const char *text, int x, int y,
              SDL_Color color, int pixelScale) {
  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  for (const char *it = text; *it; ++it, x += 4 * pixelScale) {
    const char character = (*it >= 'a' && *it <= 'z') ? *it - 'a' + 'A' : *it;
    const Glyph *glyph = nullptr;
    for (const Glyph &candidate : kFont) {
      if (candidate.character == character) {
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
  const uint8_t red5 = (color >> 11) & 0x1F;
  const uint8_t green6 = (color >> 5) & 0x3F;
  const uint8_t blue5 = color & 0x1F;
  return {static_cast<uint8_t>((red5 * 255) / 31),
          static_cast<uint8_t>((green6 * 255) / 63),
          static_cast<uint8_t>((blue5 * 255) / 31), 255};
}

} // namespace ohmega::ui

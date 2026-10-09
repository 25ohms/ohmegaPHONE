#include "ohmega/Slider.h"

#include "ohmega/GraphicsMath.h"
#include "ohmega/UiDrawing.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace ohmega {
namespace {
constexpr int kTrackYInset = 8;
constexpr int kTrackHeight = 12;
constexpr int kKnobWidth = 9;
constexpr int kKnobHeight = 12;
}

Slider::Slider(const char *label, int y, float minimum, float maximum,
               float initialValue, int precision, bool integerValue)
    : _label(label), _y(y), _minimum(minimum), _maximum(maximum),
      _value(initialValue), _precision(precision),
      _integerValue(integerValue) {
  setValue(initialValue);
}

bool Slider::hitTest(int x, int y, int trackX, int trackWidth) const {
  return graphics::contains(x, y, trackX, _y - kTrackYInset,
                            trackWidth, kTrackHeight);
}

void Slider::setFromX(int mouseX, int trackX, int trackWidth) {
  setValue(graphics::sliderValueAt(mouseX, trackX, trackWidth,
                                  _minimum, _maximum));
}

void Slider::setValue(float value) {
  _value = std::clamp(value, _minimum, _maximum);
  if (_integerValue) _value = std::round(_value);
}

int Slider::integer() const { return static_cast<int>(std::round(_value)); }

void Slider::draw(SDL_Renderer *renderer, int labelX, int trackX,
                  int trackWidth) const {
  ui::drawText(renderer, _label, labelX, _y - 3, {160, 180, 188, 255});
  ui::drawRect(renderer, trackX, _y, trackWidth, 4, {42, 55, 63, 255});
  const int knobX = graphics::sliderPosition(_value, trackX, trackWidth,
                                              _minimum, _maximum);
  ui::drawRect(renderer, trackX, _y, knobX - trackX, 4,
               {0, 215, 225, 255});
  ui::drawRect(renderer, knobX - kKnobWidth / 2, _y - 4, kKnobWidth,
               kKnobHeight, {0, 215, 225, 255});

  char valueText[20];
  if (_integerValue)
    std::snprintf(valueText, sizeof(valueText), "%d", integer());
  else
    std::snprintf(valueText, sizeof(valueText), "%.*f", _precision, _value);
  ui::drawText(renderer, valueText, trackX + trackWidth + 12, _y - 3,
               {220, 230, 235, 255});
}

} // namespace ohmega

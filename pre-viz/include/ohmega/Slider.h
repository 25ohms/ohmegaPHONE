#pragma once

#include <SDL.h>

namespace ohmega {

class Slider {
public:
  Slider(const char *label, int y, float minimum, float maximum,
         float initialValue, int precision, bool integerValue = false);

  bool hitTest(int x, int y, int trackX, int trackWidth) const;
  void setFromX(int mouseX, int trackX, int trackWidth);
  void setValue(float value);
  float value() const { return _value; }
  int integer() const;
  int y() const { return _y; }

  void draw(SDL_Renderer *renderer, int labelX, int trackX,
            int trackWidth) const;

private:
  const char *_label;
  int _y;
  float _minimum;
  float _maximum;
  float _value;
  int _precision;
  bool _integerValue;
};

} // namespace ohmega

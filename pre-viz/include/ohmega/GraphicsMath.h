#pragma once

#include "ohmega/Mesh.h"

namespace ohmega::graphics {

struct ScreenPoint {
  int x;
  int y;
};

Point3 project(float x, float y, float z, float angleY);
ScreenPoint toScreen(const Point3 &point, const Mesh &mesh, int width,
                     int height, float scale, float zScale, float angleY);
float sliderValueAt(int mouseX, int trackX, int trackWidth, float minimum,
                    float maximum);
int sliderPosition(float value, int trackX, int trackWidth, float minimum,
                   float maximum);
bool contains(int x, int y, int left, int top, int width, int height);

} // namespace ohmega::graphics

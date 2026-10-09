#pragma once

#include <cstddef>

// Shared Ω outline used by both the ESP32 renderer and the desktop pre-visualizer.
struct OhmPoint2 {
  float x;
  float y;
};

constexpr OhmPoint2 kOhmOutline[] = {
    {-50, 58}, {-34, 58}, {-38, 49}, {-44, 39}, {-48, 27}, {-50, 14},
    {-49, 1}, {-46, -12}, {-40, -24}, {-31, -34}, {-20, -41}, {-7, -45},
    {7, -45}, {20, -41}, {31, -34}, {40, -24}, {46, -12}, {49, 1},
    {50, 14}, {48, 27}, {44, 39}, {38, 49}, {34, 58}, {50, 58},
    {50, 48}, {29, 48}, {30, 38}, {32, 28}, {34, 17}, {34, 5},
    {31, -7}, {25, -18}, {18, -26}, {10, -31}, {4, -33}, {-4, -33},
    {-10, -31}, {-18, -26}, {-25, -18}, {-31, -7}, {-34, 5}, {-34, 17},
    {-32, 28}, {-30, 38}, {-29, 48}, {-50, 48}};

constexpr size_t kOhmOutlinePointCount =
    sizeof(kOhmOutline) / sizeof(kOhmOutline[0]);

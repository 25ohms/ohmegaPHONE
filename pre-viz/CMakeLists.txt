cmake_minimum_required(VERSION 3.16)
project(ohmega_previz LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED IMPORTED_TARGET sdl2)

add_executable(ohmega_previz
  main.cpp
  CsvMeshLoader.cpp
  GraphicsMath.cpp
  MeshRenderer.cpp
  PreVizApp.cpp
  Screen.cpp
  Slider.cpp
  UiDrawing.cpp
)
target_compile_features(ohmega_previz PRIVATE cxx_std_17)
target_include_directories(ohmega_previz PRIVATE include ../include)
target_link_libraries(ohmega_previz PRIVATE PkgConfig::SDL2)

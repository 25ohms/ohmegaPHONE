#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Arduino.h>
#include <SPI.h>

#include "OhmWireframe.h"

#define TFT_CS 5
#define TFT_DC 2
#define TFT_RST 4

Adafruit_ILI9341 tft(TFT_CS, TFT_DC, TFT_RST);
OhmWireframe ohm(tft);

uint32_t animationStartedAt = 0;

void setup() {
  Serial.begin(115200);

  tft.begin();
  tft.setRotation(0); // Native 240 x 320 portrait orientation.
  tft.fillScreen(ILI9341_BLACK);

  // Tweak scale, depth, rotation speeds, meshStep, and color here.
  ohm.settings().scale = 1.25f;
  ohm.settings().depth = 18.0f;
  ohm.settings().rotationSpeedX = 0.0f;
  ohm.settings().rotationSpeedY = 0.8f;
  ohm.settings().meshStep = 2;
  ohm.settings().color = ILI9341_CYAN;
  animationStartedAt = millis();
}

void loop() {
  tft.fillScreen(ILI9341_BLACK);
  ohm.draw(millis() - animationStartedAt);
  delay(33); // About 30 frames per second.
}

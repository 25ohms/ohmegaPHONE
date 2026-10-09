#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Arduino.h>
#include <LittleFS.h>
#include <SPI.h>

#include "OhmConfig.h"
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
  tft.setRotation(OhmConfig::kDisplayRotation);
  tft.fillScreen(ILI9341_BLACK);

  if (!LittleFS.begin(false)) {
    Serial.print("LittleFS mount failed; cannot load ");
    Serial.println(OhmConfig::kCoordinatesFileName);
  } else {
    File coordinates = LittleFS.open(OhmConfig::kCoordinatesFileName, "r");
    if (!coordinates || !ohm.loadGeometry(coordinates)) {
      Serial.print("Could not load mesh from ");
      Serial.println(OhmConfig::kCoordinatesFileName);
    }
    coordinates.close();
  }

  animationStartedAt = millis();
}

void loop() {
  tft.fillScreen(ILI9341_BLACK);
  ohm.draw(millis() - animationStartedAt);
  delay(33); // About 30 frames per second.
}

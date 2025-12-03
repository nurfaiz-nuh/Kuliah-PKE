#ifndef PLOTTER_H
#define PLOTTER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

#include "Fonts/TomThumb.h"

extern Adafruit_SSD1306 display;

// Draw RED graph
void drawRedPlot(long redHistory[], int size, long redValue) {
  long currentMin = redHistory[0];
  long currentMax = redHistory[0];
  for (int i = 1; i < size; ++i) {
    if (redHistory[i] < currentMin) currentMin = redHistory[i];
    if (redHistory[i] > currentMax) currentMax = redHistory[i];
  }
  currentMin -= 500;
  currentMax += 500;
  if (currentMax <= currentMin) currentMax = currentMin + 1;

  int *localPlot = new int[size];
  for (int i = 0; i < size; ++i) {
    localPlot[i] = map(redHistory[i], currentMin, currentMax, SCREEN_HEIGHT - 1, 0);
  }

  int activeIndex = (size - 1);
  long activeValue = redValue;

  display.clearDisplay();

  display.setFont(&TomThumb);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 5);
  display.print("RED Plot");

  display.setCursor(81, 5);
  display.print("Value: " + String(activeValue));

  display.setCursor(0, 64);
  display.print("Min:" + String(currentMin));
  display.setCursor(91, 64);
  display.print("Max:" + String(currentMax));

  for (int x = 1; x < size; x++) {
    int xPrev = (x - 1) * xSpacing;
    int xCurr = x * xSpacing;
    if (xCurr >= SCREEN_WIDTH) break;
    display.drawLine(xPrev, localPlot[x-1], xCurr, localPlot[x], SSD1306_WHITE);
  }

  display.display();
  delete[] localPlot;
}

// Draw IR graph
void drawIRPlot(long irHistory[], int size, long irValue) {
  long currentMin = irHistory[0];
  long currentMax = irHistory[0];
  for (int i = 1; i < size; ++i) {
    if (irHistory[i] < currentMin) currentMin = irHistory[i];
    if (irHistory[i] > currentMax) currentMax = irHistory[i];
  }
  currentMin -= 500;
  currentMax += 500;
  if (currentMax <= currentMin) currentMax = currentMin + 1;

  int *localPlot = new int[size];
  for (int i = 0; i < size; ++i) {
    localPlot[i] = map(irHistory[i], currentMin, currentMax, SCREEN_HEIGHT - 1, 0);
  }

  long activeValue = irValue;

  display.clearDisplay();

  display.setFont(&TomThumb);
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 5);
  display.print("IR Plot");
  display.setCursor(81, 5);
  display.print("Value: " + String(activeValue));

  display.setCursor(0, 64);
  display.print("Min:" + String(currentMin));
  display.setCursor(91, 64);
  display.print("Max:" + String(currentMax));

  for (int x = 1; x < size; x++) {
    int xPrev = (x - 1) * xSpacing;
    int xCurr = x * xSpacing;
    if (xCurr >= SCREEN_WIDTH) break;
    display.drawLine(xPrev, localPlot[x-1], xCurr, localPlot[x], SSD1306_WHITE);
  }

  display.display();
  delete[] localPlot;
}

#endif // PLOTTER_H

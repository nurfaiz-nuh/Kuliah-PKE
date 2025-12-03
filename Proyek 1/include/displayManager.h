#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Fonts/Picopixel.h"
#include "Fonts/Org_01.h"

extern Adafruit_SSD1306 display;

static char dm_bpmString[12];
static char dm_spoString[12];

static void dm_rightAlign(const char* text, int y)
{
    int16_t x1, y1;
    uint16_t width, height;

    display.getTextBounds(text, 0, 0, &x1, &y1, &width, &height);

    int startX = display.width() - width;
    display.setCursor(startX, y);
    display.println(text);
}

void updateDisplay(int bpm, int spo, bool validSpo, long irValue)
{
    // Formats
    if (irValue > 50000)
        snprintf(dm_bpmString, sizeof(dm_bpmString), "%d/min ", bpm);
    else
        snprintf(dm_bpmString, sizeof(dm_bpmString), "0/min ", bpm);

    if (validSpo)
        snprintf(dm_spoString, sizeof(dm_spoString), "%d%%.o2 ", spo);
    else
        snprintf(dm_spoString, sizeof(dm_spoString), "---.o2");

    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);

    // Labels
    display.setFont(&Picopixel);

    display.setCursor(2, 7);
    display.println("PULSE-BEAT");
    display.setCursor(2, 15);
    display.println("PER MINUTE");

    display.setCursor(2, 42);
    display.println("PERIPHERAL");
    display.setCursor(2, 50);
    display.println("SATURATION");

    display.drawLine(0, 32, 128, 32, 1);

    // Values
    display.setFont(&Org_01);
    display.setTextSize(2);

    dm_rightAlign(dm_bpmString, 22);
    dm_rightAlign(dm_spoString, 57);

    display.display();
}

#endif // DISPLAY_MANAGER_H

#ifndef DRAW_SCREENS_H
#define DRAW_SCREENS_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"

extern Adafruit_SSD1306 display;
#include "Fonts/Org_01.h"
#include "Fonts/Picopixel.h"

void drawSplashScreen() 
{
    int16_t x1, y1;
    uint16_t w, h;
    int centerX;

    display.clearDisplay();

    display.setFont(&Org_01);
    display.setTextSize(2);
    display.setTextColor(WHITE);

    const char* title = "OXIMETER";
    display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);

    centerX = (display.width() - w) / 2;
    display.setCursor(centerX, 18);
    display.println(title);

    display.setFont(&Picopixel);
    display.setTextSize(1);

    const char* sub = "MAX30105 + SSD1306";
    display.getTextBounds(sub, 0, 0, &x1, &y1, &w, &h);

    centerX = (display.width() - w) / 2;
    display.setCursor(centerX, 42);
    display.println(sub);

    display.drawLine(10, 55, 118, 55, 1);

    display.display();
}

void drawConfiguration()
{
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setFont(NULL);
    display.setCursor(24, 5);
    display.print("BUTTONS");

    display.setTextSize(1);
    display.setFont(&Picopixel);

    display.drawCircle(32, 40, 10, 1);
    display.setCursor(23, 60);
    display.print("GRAPH");

    display.fillCircle(64, 40, 10, 1);
    display.setCursor(55, 60);
    display.print("RESET");

    display.drawCircle(96, 40, 10, 1);
    display.setCursor(87, 60);
    display.print("VALUE");
    display.setTextSize(1);
    
    display.display();
}

void drawCountDown(int count)
{
    int16_t x1, y1;
    uint16_t w, h;
    int centerX;

	display.clearDisplay();
	
	display.setFont(NULL);
    display.setTextSize(1);
    display.setTextColor(WHITE);

    const char* text = "Turning off";
    display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);

    centerX = (display.width() - w) / 2;
    display.setCursor(centerX, 22);
    display.println(text);

    display.setCursor(52, 35);
    display.print("in");

    display.setCursor(71, 35);
    display.print(count);
    display.setCursor(70, 35);
    display.print(count);

	display.display();
}

#endif // DRAW_SCREENS_H

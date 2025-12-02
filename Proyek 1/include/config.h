#ifndef CONFIG_H
#define CONFIG_H

// Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Plotting
constexpr int maxDataPoints = 128;
constexpr int xSpacing = 4;
constexpr int calcDataPoints = maxDataPoints / xSpacing;

// Pins (adjust if needed)
#define RESET_PIN 33
#define VALUE_PIN 32
#define GRAPH_PIN 27
#define HEART_PIN 2

// Sensor threshold
static const long FINGER_THRESHOLD = 50000L;

#endif // CONFIG_H

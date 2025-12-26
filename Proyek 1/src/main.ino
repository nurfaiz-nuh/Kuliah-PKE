#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "MAX30105.h"

#include "config.h"

#include "pulseDetector.h"
#include "spo.h"
#include "bpm.h"
#include "displayManager.h"
#include "displayAverage.h"
#include "plotter.h"
#include "drawScreens.h"

// Global hardware objects (only defined once here)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
MAX30105 particleSensor;

// Screen mapping
// -1 = error, 0 = splash, 1 = buttons, 2 = avg val, 3 = raw val, 4 = red plot, 5 = ir plot, 6 = off
static int screenID = 0;
static int lastActiveScreen = 2;   // default home screen

// Plot buffers (shared storage)
static long irHistory[calcDataPoints];
static long redHistory[calcDataPoints];
static int plotBuffer[calcDataPoints];
static int bufferIndex = 0;

// Flags
volatile bool buttonFlag = false;
volatile uint8_t pcflagGraph = false;

// New: VALUE_PIN interrupt flag and debounce timer
volatile bool pcflagValue = false;
volatile uint32_t lastValueInterrupt = 0;

// Forward declarations
void IRAM_ATTR graphISR();
void IRAM_ATTR valueISR();
bool readValueButton();
void checkResetButton();
void updateScale(long history[], int size);

// Pulsating buzzer
unsigned long pulseStartTime = 0;
bool isPulsing = false;

// Sleep system
static unsigned long lastUserAction = 0;
static int sleepCounter = 0;
static const int sleepThreshold = 150;
static int count = 5;

void IRAM_ATTR graphISR() {
  buttonFlag = true;
  pcflagGraph = true;
}

void IRAM_ATTR valueISR() {
  uint32_t now = millis();
  // simple debounce inside ISR (cheap)
  if ((now - lastValueInterrupt) > 250) {
    pcflagValue = true;
    lastValueInterrupt = now;
  }
}

bool readValueButton() {
  static uint32_t last = 0;
  static bool lastState = HIGH;
  bool s = digitalRead(VALUE_PIN);
  if (s != lastState) {
    last = millis();
  }
  if ((millis() - last) > 50) {
    if (s == LOW && lastState == HIGH) {
      lastState = s;
      return true;
    }
  }
  lastState = s;
  return false;
}

void checkResetButton() {
  static bool last = HIGH;
  bool s = digitalRead(RESET_PIN);
  if (s == LOW && last == HIGH) {
    ESP.restart();
  }
  last = s;
}

void updateScale(long history[], int size) {
  long currentMin = history[0];
  long currentMax = history[0];

  for (int i = 1; i < size; i++) {
    if (history[i] < currentMin) currentMin = history[i];
    if (history[i] > currentMax) currentMax = history[i];
  }

  currentMin -= 500;
  currentMax += 500;
  if (currentMax <= currentMin) currentMax = currentMin + 1;
}

void triggerPulse() {
  if (!isPulsing) {
    digitalWrite(HEART_PIN, HIGH);
    pulseStartTime = millis();
    isPulsing = true;
  }
}

void go_sleep() {
  screenID = 7;
  display.clearDisplay();
  display.display();
  particleSensor.shutDown();
  delay(10);
  esp_sleep_enable_ext0_wakeup((gpio_num_t)RESET_PIN, 0);
  esp_deep_sleep_start();
}

// -------------------
void setup() {
  Serial.begin(115200);

  Wire.begin();
  delay(50);

  // Initialize display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 not found"));
    screenID = -1;
  } else {
    display.clearDisplay();
    display.display();
  }

  // Initialize sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println(F("MAX30102 not found"));
    screenID = -1;
  } else {
    byte ledBrightness = 60;
    byte sampleAverage = 4;
    byte ledMode = 2;
    int sampleRate = 1000;
    int pulseWidth = 411;
    int adcRange = 4096;
    particleSensor.setup(
      ledBrightness,
      sampleAverage,
      ledMode,
      sampleRate,
      pulseWidth,
      adcRange
    );
    particleSensor.setPulseAmplitudeRed(0x1F);
    particleSensor.setPulseAmplitudeIR(0x1F);
  }

  // Initialize modules
  setupSpo();
  setupBpm();

  // Pins
  pinMode(GRAPH_PIN, INPUT_PULLUP);
  pinMode(VALUE_PIN, INPUT_PULLUP);
  pinMode(RESET_PIN, INPUT_PULLUP);
  pinMode(HEART_PIN, OUTPUT);
  digitalWrite(HEART_PIN, LOW);

  // Attach interrupts
  attachInterrupt(digitalPinToInterrupt(GRAPH_PIN), graphISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(VALUE_PIN), valueISR, FALLING);

  // initialize histories
  for (int i = 0; i < calcDataPoints; ++i) {
    irHistory[i] = 0;
    redHistory[i] = 0;
    plotBuffer[i] = SCREEN_HEIGHT - 1;
  }

  screenID = 0; // splash on boot
  lastUserAction = millis();
}

// -------------------
unsigned long lastBeepMillis = 0;
bool beeping = false;

void loop() {
  // Non-blocking sensor processing
  loopSpo();   // updates spo2 & validSPO2 globals in spo module
  loopBpm();   // updates instantaneous and average bpm in bpm module

  // read sensor once for waveform plotting
  particleSensor.check();
  long irValue = particleSensor.getIR();
  long redValue = particleSensor.getRed();

  // update histories
  irHistory[bufferIndex] = irValue;
  redHistory[bufferIndex] = redValue;
  bufferIndex = (bufferIndex + 1) % calcDataPoints;

  if (pcflagGraph && !digitalRead(GRAPH_PIN)) {
    lastUserAction = millis();

    // If countdown screen → interrupt it
    if (screenID == 6) {
      screenID = 4;
    } else if (screenID != 4 && screenID != 5) {
      screenID = 4;
    } else {
      screenID = (screenID == 4) ? 5 : 4;
    }
  }
  pcflagGraph = false;

  // VALUE button ALWAYS works (even during countdown)
  if (pcflagValue || readValueButton()) {
    // consume ISR flag if present
    pcflagValue = false;
    lastUserAction = millis();

    // If in countdown, break out to normal view
    if (screenID == 6) {
      screenID = 2; // default home
    } else if (screenID == 4 || screenID == 5) {
      // exit plots to main average screen
      screenID = 2;
    } else {
      // toggle between main avg (2) and raw (3)
      if (screenID == 2) screenID = 3;
      else screenID = 2;
    }
  }

  // RESET button
  checkResetButton();

  // Update sleep counter
  bool isPressed =
    (digitalRead(GRAPH_PIN) == LOW || digitalRead(VALUE_PIN) == LOW);
  if (irValue < FINGER_THRESHOLD && !isPressed) {  // no finger or no button pressed
    sleepCounter++;
  } else {
    sleepCounter = 0;
  }

  // Enter countdown mode once sleepCounter reaches 100
  if (sleepCounter >= 100 && sleepCounter < sleepThreshold) {
    // Save the screen they were on before countdown —— "ONLY once"
    if (screenID != 6) {
      lastActiveScreen = screenID;
    }
    screenID = 6;
    count = ((sleepThreshold - sleepCounter) / 10) + 1;
  } else if (sleepCounter >= sleepThreshold) {
    go_sleep();
  }

  if (isPulsing) {
    if (millis() - pulseStartTime >= 25) {
      // stopping effect
      noTone(HEART_PIN);
      digitalWrite(HEART_PIN, LOW);
      isPulsing = false;
    }
  }

  // Beep and heart animation trigger (only if not screen off)
  bool fingerPresent = (irValue >= FINGER_THRESHOLD);
  if (screenID == 6 && fingerPresent) {
    screenID = lastActiveScreen;
    sleepCounter = 0;
  }
  if (screenID != 7 && fingerPresent) {
    if (detectPulse(irValue)) {
      // tone(HEART_PIN, 4000, 25) was here
      triggerPulse();
      lastBeepMillis = millis();
      beeping = true;
    }
  }

  // Replaces tone() function for louder buzzer
  if (beeping && (millis() - lastBeepMillis) > 25) {
    digitalWrite(HEART_PIN, LOW);
    beeping = false;
  }

  // Draw the current screen (preserve templates)
  switch (screenID) {
    case -1:
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(5, 30);
      display.println("DEVICE ERROR!");
      display.display();
      break;

    case 0:
      drawSplashScreen();
      delay(2000);
      screenID = 1;
      break;

    case 1:
      drawConfiguration();
      break;

    case 2: {
      int avgB = getAverageBPM();
      int avgS = (int)getSpo2();
      bool vS = isSpo2Valid();
      bool beatNow = (millis() - lastBeepMillis) < 200;
      updateHeartDisplay(avgB, avgS, vS, beatNow, irValue);
      break;
    }

    case 3: {
      int instB = (int)getInstantBPM();
      int spoVal = (int)getSpo2();
      bool v = isSpo2Valid();
      updateDisplay(instB, spoVal, v, irValue);
      break;
    }

    case 4:
      drawRedPlot(redHistory, calcDataPoints, redValue);
      break;

    case 5:
      drawIRPlot(irHistory, calcDataPoints, irValue);
      break;

    case 6:
      drawCountDown(count);
      break;

    case 7:
      display.clearDisplay();
      display.display();
      display.ssd1306_command(SSD1306_DISPLAYOFF);
      break;

    default:
      display.clearDisplay();
      display.display();
      break;
  }

  delay(10); // keep the loop responsive but not 100% tight
}

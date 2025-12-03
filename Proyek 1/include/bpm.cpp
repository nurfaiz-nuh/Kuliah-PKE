#include "bpm.h"
#include <Arduino.h>
#include "config.h"
#include "MAX30105.h"
#include "pulseDetector.h"

// extern sensor declared in main
extern MAX30105 particleSensor;

// Local module state (static inside this translation unit)
static const byte HR_RATE_SIZE = 4;
static byte hrRates[HR_RATE_SIZE] = {0,0,0,0};
static byte hrRateSpot = 0;

static unsigned long hrLastBeat = 0;
static float hrBPM = 0.0f;
static int hrAvgBPM = 0;

void setupBpm() {
  // nothing to init (sensor set up in main)
}

void loopBpm() {
  // Non-blocking read
  particleSensor.check();
  long irValue = particleSensor.getIR();

  if (irValue < FINGER_THRESHOLD) {
    return; // no finger -> skip
  }

  if (detectPulse(irValue)) {
    unsigned long now = millis();
    unsigned long delta = now - hrLastBeat;
    hrLastBeat = now;

    if (delta > 0) {
      hrBPM = 60000.0f / (float)delta;
    }

    if (hrBPM > 43 && hrBPM < 255) {
      hrRates[hrRateSpot++] = (byte)hrBPM;
      hrRateSpot %= HR_RATE_SIZE;

      int sum = 0;
      for (byte i = 0; i < HR_RATE_SIZE; i++) sum += hrRates[i];
      hrAvgBPM = sum / HR_RATE_SIZE;

      Serial.print("BPM=");
      Serial.print(hrBPM);
      Serial.print(" Avg=");
      Serial.println(hrAvgBPM);
    }
  }
}

// Accessors
float getInstantBPM() {
  return hrBPM;
}
int getAverageBPM() {
  return hrAvgBPM;
}

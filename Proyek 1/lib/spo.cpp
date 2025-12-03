#include "spo.h"
#include "spo2_algorithm.h"
#include "config.h"
#include "MAX30105.h"

// extern sensor from main.ino
extern MAX30105 particleSensor;

// Buffers (module-local)
#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)
static uint16_t irBuffer[100];
static uint16_t redBuffer[100];
#else
static uint32_t irBuffer[100];
static uint32_t redBuffer[100];
#endif

static const int bufferLength = 100;
static int spoBufferIndex = 0;
static bool bufferReady = false;

static int32_t spo2_val = 0;
static int8_t validSPO2_val = 0;
static int32_t heartRate_val = 0;
static int8_t validHeartRate_val = 0;

void setupSpo() {
  // nothing additional required — sensor is initialized in main
}

void loopSpo() {
  // Only take a sample if new data is available
  // Use particleSensor.available() if present; check() beforehand
  particleSensor.check();
#ifdef GET_AVAILABLE_EXISTS
  if (!particleSensor.available()) {
    return;
  }
#endif

  // Read one sample (non-blocking)
  uint32_t red = particleSensor.getRed();
  uint32_t ir  = particleSensor.getIR();
  particleSensor.nextSample();

  if (ir < FINGER_THRESHOLD) {
    // finger removed: reset buffer and mark invalid
    spoBufferIndex = 0;
    bufferReady = false;
    validSPO2_val = 0;
    return;
  }

  redBuffer[spoBufferIndex] = red;
  irBuffer[spoBufferIndex] = ir;
  spoBufferIndex++;

  if (spoBufferIndex >= bufferLength) {
    bufferReady = true;
    spoBufferIndex = 0;
  }

  if (bufferReady) {
    maxim_heart_rate_and_oxygen_saturation(
      irBuffer,
      bufferLength,
      redBuffer,
      &spo2_val,
      &validSPO2_val,
      &heartRate_val,
      &validHeartRate_val
    );

    // shift last 25 samples to front (rolling window)
    for (int i = 25; i < bufferLength; i++) {
      redBuffer[i - 25] = redBuffer[i];
      irBuffer[i - 25] = irBuffer[i];
    }
    spoBufferIndex = 75;
    bufferReady = false;
  }
}

// Accessors
int32_t getSpo2() {
  return spo2_val;
}
int8_t isSpo2Valid() {
  return validSPO2_val;
}

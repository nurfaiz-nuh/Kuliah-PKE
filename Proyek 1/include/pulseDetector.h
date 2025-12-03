#ifndef PULSE_DETECTOR_H
#define PULSE_DETECTOR_H

// === CONFIGURABLE PARAMETERS ===
static const long PULSE_THRESHOLD = FINGER_THRESHOLD; // rely on config.h
static const int  MIN_RISE_STEPS  = 3;
static const uint16_t SAMPLE_RATE_MS = 10;

// Module-local state
static long pd_lastValue = 0;
static bool pd_rising = false;
static int  pd_riseCount = 0;

inline bool detectPulse(long irValue)
{
    bool beat = false;

    if (irValue < PULSE_THRESHOLD) {
        pd_rising = false;
        pd_riseCount = 0;
        pd_lastValue = irValue;
        return false;
    }

    if (irValue > pd_lastValue) {
        pd_rising = true;
        pd_riseCount++;
    } else {
        if (pd_rising && pd_riseCount >= MIN_RISE_STEPS) {
            beat = true;
        }
        pd_rising = false;
        pd_riseCount = 0;
    }

    pd_lastValue = irValue;
    return beat;
}

#endif // PULSE_DETECTOR_H

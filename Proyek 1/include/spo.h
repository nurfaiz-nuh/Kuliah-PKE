#ifndef SPO_H
#define SPO_H

#include <stdint.h>

// Setup & loop
void setupSpo();
void loopSpo();

// Accessors
int32_t getSpo2();
int8_t isSpo2Valid();

#endif // SPO_H

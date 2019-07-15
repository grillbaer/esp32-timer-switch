#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <Arduino.h>

void initDisplay();
void updateDisplay(uint32_t onSecs, uint32_t offSecs, bool on, uint32_t current, bool autoSwitch);

#endif /* DISPLAY_H_ */

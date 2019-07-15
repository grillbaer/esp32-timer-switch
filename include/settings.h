#ifndef SETTINGS_H_
#define SETTINGS_H_

#include <Arduino.h>

class Settings {
public:
    uint32_t onMinSecs;
    uint32_t onMaxSecs;
    uint32_t offMinSecs;
    uint32_t offMaxSecs;
    bool autoSwitch;
    bool initiallyOn;

    void setDefault();
    void makeValid();
    bool load();
    void store();

    uint32_t getOnSecs();
    uint32_t getOffSecs();
};

#endif /* SETTINGS_H_ */
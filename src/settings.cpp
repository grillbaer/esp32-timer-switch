#include "settings.h"
#include "EEPROM.h"

#include <stdlib.h>

#define MAGIC 0xE15E

void Settings::setDefault() {
    onMinSecs = onMaxSecs = 3*60;
    offMinSecs = offMaxSecs = 15;
    autoSwitch = true;
    initiallyOn = true;
}

bool Settings::load() {
    EEPROM.begin(20);

    uint16_t magic = EEPROM.readShort(0);
    if (magic != MAGIC) return false;

    onMinSecs = EEPROM.readULong(2);
    onMaxSecs = EEPROM.readULong(6);

    offMinSecs = EEPROM.readULong(10);
    offMaxSecs = EEPROM.readULong(14);

    autoSwitch = EEPROM.readBool(18);
    initiallyOn = EEPROM.readBool(19);

    makeValid();

    return true;
}

void Settings::store() {
    EEPROM.writeShort(0, MAGIC);

    EEPROM.writeULong(2, onMinSecs);
    EEPROM.writeULong(6, onMaxSecs);

    EEPROM.writeULong(10, offMinSecs);
    EEPROM.writeULong(14, offMaxSecs);

    EEPROM.writeBool(18, autoSwitch);
    EEPROM.writeBool(19, initiallyOn);

    EEPROM.commit();
}

uint32_t limitSecs(uint32_t secs) {
    if (secs < 1) return 1;
    else if (secs > 99999) return 99999;
    else return secs;
}

void Settings::makeValid() {
    onMinSecs = limitSecs(onMinSecs);
    onMaxSecs = limitSecs(onMaxSecs);
    if (onMinSecs > onMaxSecs) onMinSecs = onMaxSecs;

    offMinSecs = limitSecs(offMinSecs);
    offMaxSecs = limitSecs(offMaxSecs);
    if (offMinSecs > offMaxSecs) offMinSecs = offMaxSecs;
}

uint32_t getSecsInRange(uint32_t min, uint32_t max) {
    return min + rand() % (max-min+1);
}

uint32_t Settings::getOnSecs() {
    return getSecsInRange(onMinSecs, onMaxSecs);
}
uint32_t Settings::getOffSecs() {
    return getSecsInRange(offMinSecs, offMaxSecs);
}

#include <Arduino.h>
#include "display.h"

// controlled switch
#define SWITCH_PIN 16

// blinky
#define BLINKY_PIN 12
int blinky = 1;

// skip button
#define SKIP_PIN 15
volatile bool skip = false;
volatile uint32_t skipDebounceMillis = 0;

// switch on timer button
#define TOGGLE_AUTO_SWITCH_PIN 13
bool autoSwitch = true;
volatile bool toggleAuto = false;
volatile uint32_t toggleAutoDebounceMillis = 0;

// on/off target times
uint32_t onSecs = 3*60;
uint32_t offSecs = 15;

// current state
bool on = true;
uint32_t currentSecs = 0;


void skipDown();
void toggleAutoDown();


void setup()
{
	Serial.begin(115200);
	Serial.println("STARTED");

	initDisplay();

	pinMode(BLINKY_PIN, OUTPUT);

	pinMode(SWITCH_PIN, OUTPUT);
	digitalWrite(SWITCH_PIN, on);

	pinMode(SKIP_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(SKIP_PIN), skipDown, FALLING);
	
	pinMode(TOGGLE_AUTO_SWITCH_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(TOGGLE_AUTO_SWITCH_PIN), toggleAutoDown, FALLING);
}

void loop()
{
	uint32_t startMicros = micros();

	// blinky
	digitalWrite(BLINKY_PIN, blinky);
	blinky = !blinky;

	if (toggleAuto) {
		autoSwitch = !autoSwitch;
		toggleAuto = false;
	}

	// switch 
	if (   skip 
		|| (autoSwitch && on && currentSecs >= onSecs) 
		|| (autoSwitch && !on && currentSecs >= offSecs)) {
		skip = false;
		on = !on;
		currentSecs = 0;
		digitalWrite(SWITCH_PIN, on);
		Serial.println(on ? "out=ON" : "out=OFF");
	}

	updateDisplay(onSecs, offSecs, on, currentSecs, autoSwitch);

	uint32_t elapsed = micros() - startMicros;
	if (elapsed > 500000u) {
		elapsed = 0;
	}
	usleep(500000u - elapsed);

	digitalWrite(BLINKY_PIN, blinky);
	blinky = !blinky;

	if (skip || toggleAuto)
		return;
	
	usleep(500000u);

	currentSecs = (currentSecs + 1) % 100000;
}

bool isDebouncedDown(volatile uint32_t *debounceMillis) {
	return true;
	uint32_t now = millis();
	if (now - *debounceMillis > 50UL) {
		*debounceMillis = now;
		return true;
	}
	return false;
}

void IRAM_ATTR skipDown() {
	noInterrupts();
	if (isDebouncedDown(&skipDebounceMillis)) {
		skip = true;
	}
	interrupts();
}

void IRAM_ATTR toggleAutoDown() {
	noInterrupts();
	if (isDebouncedDown(&toggleAutoDebounceMillis)) {
		toggleAuto = true;
	}
	interrupts();
}

#include <Arduino.h>
#include "display.h"

// controlled switch
#define SWITCH_PIN 16

// blinky
#define BLINKY_PIN 12
int blinky = 1;

bool on = false;
uint16_t onSecs = 3*60;
uint16_t offSecs = 15;
uint16_t remaining = 0;

void setup()
{
	Serial.begin(115200);
	Serial.println("Starting");

	initDisplay();

	pinMode(BLINKY_PIN, OUTPUT);
	pinMode(SWITCH_PIN, OUTPUT);
}

void loop()
{
	uint32_t startMicros = micros();

	// blinky
	digitalWrite(BLINKY_PIN, blinky);
	blinky = !blinky;

	if (remaining == 0) {
		on = !on;
		remaining = on ? onSecs : offSecs;
		Serial.printf("Switching to %s for %ds\n", on ? "ON" : "OFF", (int) remaining);
		digitalWrite(SWITCH_PIN, on);
	}

	updateDisplay(onSecs, offSecs, on, remaining);

	remaining -= 1;

	uint32_t elapsed = micros() - startMicros;
	if (elapsed > 500000u) {
		elapsed = 0;
	}
	usleep(500000u - elapsed);

	digitalWrite(BLINKY_PIN, blinky);
	blinky = !blinky;
	usleep(500000u);
}

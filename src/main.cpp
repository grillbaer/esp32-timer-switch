#include "display.h"
#include "settings.h"
#include "parser.h"

#include <string>
#include <Arduino.h>

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
volatile bool toggleAuto = false;
volatile uint32_t toggleAutoDebounceMillis = 0;

// on/off target times
Settings settings;
uint32_t onSecs;
uint32_t offSecs;

// current state
bool on = true;
uint32_t currentSecs = 0;

// command parsing
std::string readCommand;

void skipDown();
void toggleAutoDown();

void setOn(bool on);
void setAutoSwitch(bool autoSwitch);

void readSerialCommand();
void command(std::string &cmd);
void printState();

void setup()
{
	if (!settings.load()) {
		settings.setDefault();
	}
	on = settings.initiallyOn;
	onSecs = settings.getOnSecs();
	offSecs = settings.getOffSecs();

	Serial.begin(115200);
	Serial.println("started");
	printState();

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
		setAutoSwitch(!settings.autoSwitch);
		toggleAuto = false;
	}

	// switch 
	if (   skip 
		|| (settings.autoSwitch && on && currentSecs >= onSecs) 
		|| (settings.autoSwitch && !on && currentSecs >= offSecs)) {
		skip = false;
		setOn(!on);
	}

	updateDisplay(onSecs, offSecs, on, currentSecs, settings.autoSwitch);

	readSerialCommand();

	uint32_t elapsed = micros() - startMicros;
	if (elapsed > 500000u) elapsed = 0;
	usleep(500000u - elapsed);

	digitalWrite(BLINKY_PIN, blinky);
	blinky = !blinky;

	if (skip || toggleAuto)
		return;
	
	readSerialCommand();
	
	elapsed = micros() - startMicros;
	if (elapsed > 1000000u) elapsed = 0;
	usleep(1000000u - elapsed);

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

void printOut()           { Serial.printf("out %s\n", on ? "on" : "off"); }
void printOnSecs()        { Serial.printf("on %ld\n", (long)onSecs); }
void printOffSecs()       { Serial.printf("off %ld\n", (long)offSecs); }
void printCurrentSecs()   { Serial.printf("current %ld\n", (long)currentSecs); }
void printOnRange()       { Serial.printf("onrange %ld %ld\n", (long)settings.onMinSecs, (long)settings.onMaxSecs); }
void printOffRange()      { Serial.printf("offrange %ld %ld\n", (long)settings.offMinSecs, (long)settings.offMaxSecs); }
void printAutoSwitch()    { Serial.printf("auto %s\n", settings.autoSwitch ? "on" : "off"); }
void printInitiallyOn()   { Serial.printf("init %s\n", settings.initiallyOn ? "on" : "off"); }

void printState() {
	printOut();
	printCurrentSecs();
	if (on) printOnSecs(); else printOffSecs();
	printOnRange();
	printOffRange();
	printAutoSwitch();
	printInitiallyOn();
}

void printHelp() {
	Serial.println(
	"out on|off           switch output on or off\n"
	"toggle               toggle output\n"
	"onrange <min> <max>  set ON seconds between <min> and <max> (1..99999)\n"
	"offrange <min> <max> set OFF seconds between <min> and <max> (1..99999)\n"
	"auto on|off          set automatic switching on or off\n"
	"init on|off          set initial output after power-on\n"
	"store                store settings permanently\n"
	"load                 load permanently stored settings\n"
	"state                print current states\n"
	"help                 print help\n"
	);
}

void setOn(bool value) {
	on = value;
	digitalWrite(SWITCH_PIN, value);
	
	currentSecs = 0;
	if (value) onSecs = settings.getOnSecs();
	else offSecs = settings.getOffSecs();

	printOut();
}

void setAutoSwitch(bool value) {
	settings.autoSwitch = value;
	printAutoSwitch();
}

void setInitiallyOn(bool value) {
	settings.initiallyOn = value;
	printInitiallyOn();
}

void limitOnOffRange() {
	if (onSecs < settings.onMinSecs) onSecs = settings.onMinSecs;
	if (onSecs > settings.onMaxSecs) onSecs = settings.onMaxSecs;
	if (offSecs < settings.offMinSecs) offSecs = settings.offMinSecs;
	if (offSecs > settings.offMaxSecs) offSecs = settings.offMaxSecs;
}

void setOnRange(uint32_t min, uint32_t max) {
	settings.onMinSecs = min;
	settings.onMaxSecs = max;
	limitOnOffRange();
	printOnRange();
}

void setOffRange(uint32_t min, uint32_t max) {
	settings.offMinSecs = min;
	settings.offMaxSecs = max;
	limitOnOffRange();
	printOffRange();
}

void readSerialCommand() {
	int c = 0;
	while (c != '\n' && Serial.available()) {
		c = Serial.read();
		if (c == '\e' || c == 2) { // escape and ctrl+c
			readCommand = "";
			Serial.println();
		} else if (c == '\b') { // backspace
			readCommand.pop_back();
			Serial.print("\b \b");
		} else if (c == '\n') { // enter
			Serial.println();
			command(readCommand);
			readCommand = "";
		} else {
			Serial.printf("%c", (char)c);
			readCommand.push_back(c);
		}
	}
}

void command(std::string &cmd) {
	Parser parser(cmd);
	const std::string stmt = parser.nextPart();
	if (stmt == "out") {
		const bool value = parser.nextOnOff();
		parser.end();
		if (parser.isOk()) setOn(value);
	} else if (stmt == "toggle") {
		parser.end();
		if (parser.isOk()) setOn(!on);
	} else if (stmt == "onrange" || stmt == "offrange") {
		const uint32_t min = parser.nextUint32();
		const uint32_t max = parser.nextUint32();
		parser.end();
		if (parser.isOk()) {
			if (min < 1 || min > 99999 || max < 1 || max > 99999 || min > max) {
				Serial.println("ERR: min and max must be between 1 and 99999 and min <= max");
			} else {
				if (stmt == "onrange") setOnRange(min, max);
				else setOffRange(min, max);
			}
		}
	} else if (stmt == "auto") {
		const bool value = parser.nextOnOff();
		parser.end();
		if (parser.isOk()) setAutoSwitch(value);
	} else if (stmt == "init") {
		const bool value = parser.nextOnOff();
		parser.end();
		if (parser.isOk()) setInitiallyOn(value);
	} else if (stmt == "state") {
		printState();
	} else if (stmt == "help") {
		printHelp();
	} else if (stmt == "store") {
		settings.store();
		Serial.println("ok");
	} else if (stmt == "load") {
		settings.load();
		limitOnOffRange();
		Serial.println("ok");
	} else if (stmt == "") {
		// no op
	} else {
		Serial.printf("ERR: unknown command: %s - enter 'help'\n", stmt.c_str());
	}

	if (!parser.isOk()) {
		Serial.printf("ERR: %s - enter 'help'\n", parser.getError().c_str());
	}
}

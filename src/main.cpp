# Drone King of the Hill
# Copyright (C) 2015 Jan Tim Jagenberg
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


#include "Arduino.h"
#include "ColorSensor.h"
#include "OneButton.h"
#include "FastLED.h"

#define SENSOR_LED 12
#define BUTTON_PIN 11
#define DATA_PIN 16
#define CLOCK_PIN 15
#define NUM_LEDS 30
#define INTENSITY 127
#define TEST_INTERVAL 100
#define WIN_INTERVAL 3000
#define DEAD_INTERVAL 5000

typedef enum {
	TEAM_SELECTION, ROUND_SELECTION, RUNNING, RESULT
} State;

ColorSensor colorSensor(SENSOR_LED);
OneButton button(BUTTON_PIN, true);
CRGB leds[NUM_LEDS];
State state = TEAM_SELECTION;
uint8_t teams = 2;
uint8_t rounds = 3;
uint8_t points[5];

uint32_t lastChecked = 0;
uint32_t firstCovered = 0;
uint32_t lastWin = DEAD_INTERVAL + 1;
uint32_t lastUncovered = DEAD_INTERVAL + 2;

void clearLeds() {
	for (int i = 0; i < NUM_LEDS; ++i) {
		leds[i] = CRGB(0, 0, 0);
	}
}

uint8_t flipLedIndex(uint8_t in) {
	return NUM_LEDS - 1 - in;
}

void rotateState() {
	if (state == TEAM_SELECTION) {
		state = ROUND_SELECTION;
	} else if (state == ROUND_SELECTION) {
		state = RUNNING;
	} else if (state == RUNNING) {
		state = RESULT;
	} else if (state == RESULT) {
		for (int i = 0; i < 4; ++i) {
			points[i] = 0;
		}
		state = TEAM_SELECTION;
	}
}

void rotateNumberOfTeams() {
	if (teams < 4) {
		teams++;
	} else {
		teams = 2;
	}
}

void showNumberOfTeams() {
	clearLeds();
	switch (teams) {
	case 2:
		for (int i = 0; i < 15; ++i) {
			leds[i] = CRGB(INTENSITY, 0, 0);
			leds[i + 15] = CRGB(0, INTENSITY, 0);
		}
		FastLED.show();
		break;
	case 3:
		for (int i = 0; i < 10; ++i) {
			leds[i] = CRGB(INTENSITY, 0, 0);
			leds[i + 10] = CRGB(0, INTENSITY, 0);
			leds[i + 20] = CRGB(0, 0, INTENSITY);
		}
		FastLED.show();
		break;
	case 4:
		for (int i = 0; i < 7; ++i) {
			leds[i] = CRGB(INTENSITY, 0, 0);
			leds[i + 7] = CRGB(0, INTENSITY, 0);
			leds[i + 14] = CRGB(0, 0, INTENSITY);
			leds[i + 21] = CRGB(INTENSITY, INTENSITY, 0);
		}
		FastLED.show();
		break;
	default:
		FastLED.show();
		break;
	}
}

void rotateNumberOfRounds() {
	switch (rounds) {
	case 1:
		rounds = 3;
		break;
	case 3:
		rounds = 5;
		break;
	case 5:
		rounds = 7;
		break;
	case 7:
		rounds = 1;
		break;
	default:
		rounds = 1;
		break;
	}
}

void showNumberOfRounds() {
	clearLeds();
	for (int i = 0; i < rounds; ++i) {
		leds[flipLedIndex(0 + i * 4)] = CRGB(INTENSITY, INTENSITY, INTENSITY);
		leds[flipLedIndex(1 + i * 4)] = CRGB(INTENSITY, INTENSITY, INTENSITY);
		leds[flipLedIndex(2 + i * 4)] = CRGB(INTENSITY, INTENSITY, INTENSITY);
	}
	FastLED.show();
}

void checkProgress() {
	uint32_t now = millis();
	if (lastChecked + TEST_INTERVAL < now) {
		lastChecked = now;
		if (colorSensor.isCovered()) {
			if (firstCovered == 0) {
				firstCovered = now;
			}
			bool coveredLongEnough = now - firstCovered > WIN_INTERVAL;
			bool uncoveredLongerThenDeadInterval = firstCovered - lastUncovered > DEAD_INTERVAL;
			bool wasUncoveredSinceLastWin = lastUncovered > lastWin;
			if (coveredLongEnough && uncoveredLongerThenDeadInterval && wasUncoveredSinceLastWin) {
				Team winner = colorSensor.getTeam();
				points[winner]++;
				lastWin = now;
				for (int i = 0; i < 4; ++i) {
					if (points[i] >= rounds) {
						state = RESULT;
						break;
					}
				}
			}
		} else if (firstCovered != 0) {
			firstCovered = 0;
			lastUncovered = now;
		}
	}
}

void showProgress() {
	clearLeds();
	uint8_t multiplier = NUM_LEDS / rounds - 1;

	// target as white dot
	leds[flipLedIndex(rounds * multiplier)].red = INTENSITY;
	leds[flipLedIndex(rounds * multiplier)].green = INTENSITY;
	leds[flipLedIndex(rounds * multiplier)].blue = INTENSITY;

	// current points as color bar
	for (int i = 0; i < NUM_LEDS; ++i) {
		if (i >= flipLedIndex(points[0] * multiplier)) {
			leds[i].red = INTENSITY;
		}
		if (i >= flipLedIndex(points[1] * multiplier)) {
			leds[i].green = INTENSITY;
		}
		if (i >= flipLedIndex(points[2] * multiplier)) {
			leds[i].blue = INTENSITY;
		}
		if (i >= flipLedIndex(points[3] * multiplier)) {
			leds[i].red = INTENSITY;
			leds[i].green = INTENSITY;
		}
	}

	FastLED.show();
}

void showResult() {
	CRGB winner;
	if (points[0] > points[1] && points[0] > points[2] && points[0] > points[3]) {
		winner = CRGB(INTENSITY, 0, 0);
	} else	if (points[1] > points[0] && points[1] > points[2] && points[1] > points[3]) {
		winner = CRGB(0, INTENSITY, 0);
	} else	if (points[2] > points[1] && points[2] > points[0] && points[2] > points[3]) {
		winner = CRGB(0, 0, INTENSITY);
	} else	if (points[3] > points[1] && points[3] > points[2] && points[3] > points[0]) {
		winner = CRGB(INTENSITY, INTENSITY, 0);
	}
	for (int i = NUM_LEDS - 1; i >= 0; --i) {
		clearLeds();
		leds[i] = winner;
		FastLED.show();
		delay(33);
	}
	for (int i = 0; i < NUM_LEDS; ++i) {
		clearLeds();
		leds[i] = winner;
		FastLED.show();
		delay(33);
	}
}

void setup() {
	FastLED.addLeds<LPD8806, DATA_PIN, CLOCK_PIN, GRB>(leds, NUM_LEDS);
	colorSensor.setup();
	button.setClickTicks(25);
	button.setPressTicks(666);
	button.attachLongPressStart(rotateState);
}

void loop() {
	button.tick();
	switch (state) {
	case TEAM_SELECTION: {
		button.attachClick(rotateNumberOfTeams);
		showNumberOfTeams();
		break;
	}
	case ROUND_SELECTION: {
		button.attachClick(rotateNumberOfRounds);
		showNumberOfRounds();
		break;
	}
	case RUNNING: {
		checkProgress();
		showProgress();
		break;
	}
	case RESULT: {
		showResult();
		break;
	}
	}
}

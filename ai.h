#ifndef _AI_H_
#define _AI_H_

#include "names.h"

class AI {
public:
	uint8_t firstNameID, lastNameID;
	uint8_t numRuns;
	uint32_t score;
	int16_t weights[6];

	void newName() {
		firstNameID = random(0, 28);
		lastNameID = random(0, 27);
	}

	/*void printName() {
		char firstName[9];
		strcpy_P(firstName, (char*)pgm_read_word(&(first_names[firstNameID])));
		Serial.print(firstName);

		Serial.print(" ");

		char lastName[14];
		strcpy_P(lastName, (char*)pgm_read_word(&(last_names[lastNameID])));
		Serial.print(lastName);
	}

	void printWeights() {
		Serial.print(F("\tHeight: ")); Serial.println(weights[0]);
		Serial.print(F("\tHoles: ")); Serial.println(weights[1]);
		Serial.print(F("\tWells: ")); Serial.println(weights[2]);
		Serial.print(F("\tLines: ")); Serial.println(weights[3]);
		Serial.print(F("\tBumpiness: ")); Serial.println(weights[4]);
		Serial.print(F("\tBlockades: ")); Serial.println(weights[5]);
	}*/

	void enterName(char *buffer) {
		char firstName[9];
		strcpy_P(firstName, (char*)pgm_read_word(&(first_names[firstNameID])));

		char lastName[14];
		strcpy_P(lastName, (char*)pgm_read_word(&(last_names[lastNameID])));

		uint8_t i = 0;
		for(i = 0; i < 9; i++) {
			if(firstName[i] != 0) {
				*(buffer++) = firstName[i];
			}
			else {
				break;
			}
		}
		*(buffer++) = ' ';
		for(i = 0; i < 14; i++) {
			if(lastName[i] != 0) {
				*(buffer++) = lastName[i];
			}
			else {
				break;
			}
		}
		*buffer = 0;
	}

	void randomize() {
		newName();

		weights[0] = random(-255, 1);
		weights[1] = random(-255, 1);
		weights[2] = random(-255, 1);
		weights[3] = random(0, 256);
		weights[4] = random(-255, 1);
		weights[5] = random(-255, 1);

		numRuns = 0;
		score = 0;
	}

	void breed(AI *parent1, AI *parent2) {
		newName();
		
		for(uint8_t i = 0; i < 6; i++) {
			int rv = random(0, 2);
			// determine which parent to choose from
			if(rv == 0) {
				// take the chromosome from parent 1
				weights[i] = parent1->weights[i];
			}
			else {
				// take the chromosome from parent 2
				weights[i] = parent2->weights[i];
			}
			// randomly mutate
			if(random(0, 100) < 2) {
				//weights[i] = random(-255, 256);
				// keep our ideology of rewarding lines and punishing everything else
				if(i == 3) {
					weights[i] = random(0, 256);
				}
				else {
					weights[i] = random(-255, 1);
				}
			}
		}
		
		numRuns = 0;
		score = 0;
	}

	uint8_t addLines(uint8_t lines) {
		// add to our score based on how many we cleared at once
		if(lines == 1) {
			score += 40;
			return 40;
		}
		else if(lines == 2) {
			score += 120;
			return 120;
		}
		else if(lines == 3) {
			score += 300;
			return 300;
		}
		else if(lines >= 4) {
			score += 1200;
			return 1200;
		}

		return 0;
	}

	uint8_t newRun() {
		numRuns++;
	}

	uint8_t getNumRuns() {
		return numRuns;
	}

	uint32_t getScore() {
		return score;
	}

	void drawHash(RGBmatrixPanel *matrix) {
		for(uint8_t x = 0; x < 6; x++) {
			uint16_t colour = matrix->ColorHSV(((uint8_t)weights[x]) * 6, 255, 255, true);
			matrix->drawPixel((x / 2) + 5, (x % 2) + 13, colour);
		}
	}

	AI operator=(AI other) {
		firstNameID = other.firstNameID;
		lastNameID = other.lastNameID;
		score = other.score;
		numRuns = other.numRuns;

		for(uint8_t i = 0; i < 6; i++) {
			weights[i] = other.weights[i];
		}
		
		return *this;
	}
};

#endif

#ifndef _SHAPES_H_
#define _SHAPES_H_

#include "randomBag.h"

PROGMEM uint8_t shapeI = {0b11110000};
PROGMEM uint8_t shapeT = {0b11100100};
PROGMEM uint8_t shapeL = {0b11101000};
PROGMEM uint8_t shapeJ = {0b11100010};
PROGMEM uint8_t shapeS = {0b01101100};
PROGMEM uint8_t shapeZ = {0b11000110};
PROGMEM uint8_t shapeO = {0b01100110};

/**
 * Pick the next random shape
 */
uint8_t *randomShape() {
	static uint8_t *shapePointers[7] = {
		&shapeI, &shapeT, &shapeL, &shapeJ, &shapeS, &shapeZ, &shapeO
	};
	static RandomBag bag;
	return shapePointers[bag.next()];
}

/**
 * Draw a shape at the given location
 * Use this for drawing the currently moving shape - not
 * locked shapes!
 */
void drawShape(uint8_t *shape, uint8_t x, uint8_t y, uint8_t negate = 0) {
	// determine the colour
	uint16_t colour = 0;
	if(!negate) {
		if(shape == &shapeI) {
			colour = matrix.Color888(0, 255, 255, true);
		}
		else if(shape == &shapeT) {
			colour = matrix.Color888(128, 0, 128, true);
		}
		else if(shape == &shapeL) {
			colour = matrix.Color888(255, 127, 0, true);
		}
		else if(shape == &shapeJ) {
			colour = matrix.Color888(0, 0, 255, true);
		}
		else if(shape == &shapeS) {
			colour = matrix.Color888(0, 255, 0, true);
		}
		else if(shape == &shapeZ) {
			colour = matrix.Color888(255, 0, 0, true);
		}
		else if(shape == &shapeO) {
			colour = matrix.Color888(255, 255, 0, true);
		}
		else {
			// unknown shape?
			return;
		}
	}

	// draw it!
	uint8_t shapeRaw = pgm_read_byte(shape);
	for(uint8_t i = 0; i < 4; i++) {
		if(shapeRaw & (0b10000000 >> i)) {
			matrix.drawPixel(32 - (y + 4), x + i + 1, colour);
		}
		if(shapeRaw & (0b10000000 >> (i + 4))) {
			matrix.drawPixel(32 - (y + 5), x + i + 1, colour);
		}
	}
}

#endif
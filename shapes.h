#ifndef _SHAPES_H_
#define _SHAPES_H_

#include "randomBag.h"

PROGMEM uint16_t shapeI[4] = {	0b0000111100000000,
								0b0010001000100010,
								0b0000000011110000,
								0b0100010001000100
							};
PROGMEM uint16_t shapeT[4] = {	0b0100111000000000,
								0b0100011001000000,
								0b0000111001000000,
								0b0100110001000000
							};
PROGMEM uint16_t shapeL[4] = {	0b0010111000000000,
								0b0100010001100000,
								0b0000111010000000,
								0b1100010001000000
							};
PROGMEM uint16_t shapeJ[4] = {	0b1000111000000000,
								0b0110010001000000,
								0b0000111000100000,
								0b0100010011000000
							};
PROGMEM uint16_t shapeS[4] = {	0b0110110000000000,
								0b0100011000100000,
								0b0000011011000000,
								0b1000110001000000
							};
PROGMEM uint16_t shapeZ[4] = {	0b1100011000000000,
								0b0010011001000000,
								0b0000110001100000,
								0b0100110010000000
							};
PROGMEM uint16_t shapeO[4] = {	0b0110011000000000,
								0b0110011000000000,
								0b0110011000000000,
								0b0110011000000000
							};

/**
 * Pick the next random shape
 */
uint16_t *randomShape() {
	static uint16_t *shapePointers[7] = {
		&shapeI[0], &shapeT[0], &shapeL[0], &shapeJ[0], &shapeS[0], &shapeZ[0], &shapeO[0]
	};
	static RandomBag bag;
	return shapePointers[bag.next()];
}

/**
 * Get the colour of a specific shape
 */
uint16_t getShapeColour(uint16_t *shape, uint8_t divideAmount = 1) {
	uint16_t colour = 0;
	if(shape == &shapeI[0]) {
		colour = matrix.Color888(0, 255 / divideAmount, 255 / divideAmount, true);
	}
	else if(shape == &shapeT[0]) {
		colour = matrix.Color888(128 / divideAmount, 0, 128 / divideAmount, true);
	}
	else if(shape == &shapeL[0]) {
		colour = matrix.Color888(255 / divideAmount, 127 / divideAmount, 0, true);
	}
	else if(shape == &shapeJ[0]) {
		colour = matrix.Color888(0, 0, 255 / divideAmount, true);
	}
	else if(shape == &shapeS[0]) {
		colour = matrix.Color888(0, 255 / divideAmount, 0, true);
	}
	else if(shape == &shapeZ[0]) {
		colour = matrix.Color888(255 / divideAmount, 0, 0, true);
	}
	else if(shape == &shapeO[0]) {
		colour = matrix.Color888(255 / divideAmount, 255 / divideAmount, 0, true);
	}
	return colour;
}

/**
 * Draw a shape at the given location
 * Use this for drawing the currently moving shape - not
 * locked shapes!
 */
void drawShape(uint16_t *shape, int8_t x, int8_t y, uint8_t rotation, uint16_t colour) {
	// draw it!
	uint16_t shapeRaw = pgm_read_word(shape + rotation);
	for(int8_t yi = 0; yi < 4; yi++) {
		for(int8_t xi = 0; xi < 4; xi++) {
			if(shapeRaw & (32768 >> ((yi * 4) + xi))) {
				matrix.drawPixel(32 - (y + yi + 7), x + xi + 1, colour);
			}
		}
	}
}

/**
 * Figure out how left a given piece can go in it's current rotation
 */
int8_t leftMostX(uint16_t *shape, uint8_t rotation) {
	uint16_t shapeRaw = pgm_read_word(shape + rotation);
	int8_t minX = -2;
	while(minX < 7) {
		uint8_t good = 1;
		for(int8_t yi = 0; yi < 4 && good; yi++) {
			for(int8_t xi = 0; xi < 4 && good; xi++) {
				// make sure the shape exists at this test point
				if(shapeRaw & (32768 >> ((yi * 4) + xi))) {
					// test if it's hitting a boundary
					if(minX + xi < 0) {
						// yup, it is
						// this one is no good
						good = 0;
					}
				}
			}
		}
		if(good) {
			return minX;
		}
		minX++;
	}
}

/**
 * Figure out how left a given piece can go in it's current rotation
 */
int8_t rightMostX(uint16_t *shape, uint8_t rotation) {
	uint16_t shapeRaw = pgm_read_word(shape + rotation);
	int8_t maxX = 9;
	while(maxX > 0) {
		uint8_t good = 1;
		for(int8_t yi = 0; yi < 4 && good; yi++) {
			for(int8_t xi = 0; xi < 4 && good; xi++) {
				// make sure the shape exists at this test point
				if(shapeRaw & (32768 >> ((yi * 4) + xi))) {
					// test if it's hitting a boundary
					if(maxX + xi >= 10) {
						// yup, it is
						// this one is no good
						good = 0;
					}
				}
			}
		}
		if(good) {
			return maxX;
		}
		maxX--;
	}
}

#endif
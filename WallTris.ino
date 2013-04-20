#include <Adafruit_GFX.h>
#include <RGBmatrixPanel.h>
#include "shapes.h"

/**
 * The matrix that we'll be using
 *
 * Pin Mapping
 * ###########
 * * A ~ A0
 * * B ~ A1
 * * C ~ A2
 * * CLK ~ 8
 * * LAT ~ A3
 * * OE ~ 9
 * * Double buffer ~ false
 */
RGBmatrixPanel matrix(A0, A1, A2, 8, A3, 9, false);

/**
 * A large generic buffer that will hold all our data
 */
uint8_t dataBuffer[128];

/**
 * Pointers to locations in the dataBuffer that our RGB components will be stored
 */
uint8_t *aBuffer, *bBuffer, *cBuffer;

/**
 * Utility function to set a value in the buffer
 */
void setBufferPixel(uint8_t *buffer, uint8_t x, uint8_t y, uint8_t on) {
	// make sure we have a valid x and y
	if(x >= 10 || y >= 20) {
		return;
	}

	if(on) {
		*(buffer + (y * 2) + (x >> 3)) |= 1 << (x % 8);
	}
	else {
		*(buffer + (y * 2) + (x >> 3)) &= ~(1 << (x % 8));
	}
}

/**
 * Utility function to read a value in the buffer
 */
uint8_t getBufferPixel(uint8_t *buffer, uint8_t x, uint8_t y) {
	// make sure we have a valid x and y
	if(x >= 10 || y >= 20) {
		return 0;
	}

	return (*(buffer + (y * 2) + (x >> 3)) & (1 << (x % 8)));
}

/**
 * Use this to draw / update the actual game region of the board
 */
void updatePlayingField() {
	for(uint8_t y = 0; y < 20; y++) {
		for(uint8_t x = 0; x < 10; x++) {
			uint8_t r = getBufferPixel(aBuffer, x, y) ? 1 : 0;
			uint8_t g = getBufferPixel(bBuffer, x, y) ? 1 : 0;
			uint8_t b = getBufferPixel(cBuffer, x, y) ? 1 : 0;
			matrix.drawPixel(32 - (y + 3), x, matrix.Color888(r * 255, g * 255, b * 255, true));
		}
	}
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
			matrix.drawPixel(32 - (y + 4), x + i, colour);
		}
		if(shapeRaw & (0b10000000 >> (i + 4))) {
			matrix.drawPixel(32 - (y + 5), x + i, colour);
		}
	}
}

/**
 * Program entry point
 */
void setup() {
	// seed our RNG
	randomSeed(analogRead(0));

	// point our buffers to where they're going
	aBuffer = &dataBuffer[0];
	bBuffer = &dataBuffer[40];
	cBuffer = &dataBuffer[80];

	// initialize the matrix
	matrix.begin();

	// draw the board outline
	uint16_t outlineColour = matrix.Color888(255, 255, 255, true);
	for(uint8_t y = 1; y < 22; y++) {
		matrix.drawPixel(32 - (y + 3), 0, outlineColour);
		matrix.drawPixel(32 - (y + 3), 11, outlineColour);
	}
	for(uint8_t x = 1; x < 11; x++) {
		//matrix.drawPixel(29, x, outlineColour);
		matrix.drawPixel(8, x, outlineColour);
	}

	// draw some shapes
	/*drawShape(&shapeI, 4, 0);
	drawShape(&shapeT, 4, 3);
	drawShape(&shapeL, 4, 6);
	drawShape(&shapeJ, 4, 9);
	drawShape(&shapeS, 4, 12);
	drawShape(&shapeZ, 4, 15);
	drawShape(&shapeO, 4, 18);*/
}

/**
 * Pick the next random shape
 */
uint8_t *randomShape() {
	static uint8_t *shapePointers[7] = {
		&shapeI, &shapeT, &shapeL, &shapeJ, &shapeS, &shapeZ, &shapeO
	};
	return shapePointers[random(0, 7)];
}

/**
 * Main program loop
 */
void loop() {
	static uint8_t y = 0;
	static uint8_t *shape = randomShape();

	// draw our shape
	drawShape(shape, 4, y);

	// timing delay
	delay(100);

	// clear the old shape
	drawShape(shape, 4, y, 1);

	// move the shape
	y += 1;
	if(y >= 19) {
		y = 0;
		shape = randomShape();
	}
}
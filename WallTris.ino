#include <Adafruit_GFX.h>
#include <RGBmatrixPanel.h>

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

#include "shapes.h"

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
			matrix.drawPixel(32 - (y + 3), x + 1, matrix.Color888(r * 255, g * 255, b * 255, true));
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
}

/**
 * Main program loop
 */
void loop() {
	static uint8_t y = 0;
	static uint8_t *shape = randomShape();
	static uint8_t *nextShape = randomShape();
	static uint8_t *next2Shape = randomShape();
	static uint8_t *next3Shape = randomShape();
	static uint8_t *next4Shape = randomShape();

	// draw our shape
	drawShape(shape, 3, y);

	// timing delay
	delay(100);

	// clear the old shape
	drawShape(shape, 3, y, 1);

	// move the shape
	y += 1;

	// spawn a new shape
	if(y >= 19) {
		y = 0;
		drawShape(nextShape, 11, 0, 1);
		drawShape(next2Shape, 11, 3, 1);
		drawShape(next3Shape, 11, 6, 1);
		drawShape(next4Shape, 11, 9, 1);
		shape = nextShape;
		nextShape = next2Shape;
		next2Shape = next3Shape;
		next3Shape = next4Shape;
		next4Shape = randomShape();
		drawShape(nextShape, 11, 0);
		drawShape(next2Shape, 11, 3);
		drawShape(next3Shape, 11, 6);
		drawShape(next4Shape, 11, 9);
	}
}
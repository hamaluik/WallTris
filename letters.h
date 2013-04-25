#ifndef _LETTERS_H_
#define _LETTERS_H_

#include "numbers.h"

PROGMEM uint8_t letters[] = {
	// A
	0b00001000,
	0b00010100,
	0b00011100,
	0b00010100,
	0b00010100,

	// B
	0b00011000,
	0b00010100,
	0b00011000,
	0b00010100,
	0b00011000,

	// C
	0b00001100,
	0b00010000,
	0b00010000,
	0b00010000,
	0b00001100,

	// D
	0b00011000,
	0b00010100,
	0b00010100,
	0b00010100,
	0b00011000,

	// E
	0b00011100,
	0b00010000,
	0b00011000,
	0b00010000,
	0b00011100,

	// F
	0b00011100,
	0b00010000,
	0b00011000,
	0b00010000,
	0b00010000,

	// G
	0b00001110,
	0b00010000,
	0b00010110,
	0b00010010,
	0b00001110,

	// H
	0b00010100,
	0b00010100,
	0b00011100,
	0b00010100,
	0b00010100,

	// I
	0b00011100,
	0b00001000,
	0b00001000,
	0b00001000,
	0b00011100,

	// J
	0b00011100,
	0b00001000,
	0b00001000,
	0b00001000,
	0b00010000,

	// K
	0b00010010,
	0b00010100,
	0b00011000,
	0b00010100,
	0b00010010,

	// L
	0b00010000,
	0b00010000,
	0b00010000,
	0b00010000,
	0b00011100,

	// M
	0b00001010,
	0b00010101,
	0b00010101,
	0b00010001,
	0b00010001,

	// N
	0b00010001,
	0b00011001,
	0b00010101,
	0b00010011,
	0b00010001,

	// O
	0b00001000,
	0b00010100,
	0b00010100,
	0b00010100,
	0b00001000,

	// P
	0b00011000,
	0b00010100,
	0b00011000,
	0b00010000,
	0b00010000,

	// Q
	0b00001000,
	0b00010100,
	0b00010100,
	0b00010100,
	0b00001110,

	// R
	0b00011000,
	0b00010100,
	0b00011000,
	0b00010100,
	0b00010100,

	// S
	0b00001100,
	0b00010000,
	0b00001000,
	0b00000100,
	0b00011000,

	// T
	0b00011100,
	0b00001000,
	0b00001000,
	0b00001000,
	0b00001000,

	// U
	0b00010100,
	0b00010100,
	0b00010100,
	0b00010100,
	0b00011100,

	// V
	0b00010100,
	0b00010100,
	0b00010100,
	0b00010100,
	0b00001000,

	// W
	0b00010001,
	0b00010001,
	0b00010101,
	0b00010101,
	0b00001010,

	// X
	0b00100010,
	0b00010100,
	0b00001000,
	0b00010100,
	0b00100010,

	// Y
	0b00010100,
	0b00010100,
	0b00001000,
	0b00001000,
	0b00001000,

	// Z
	0b00011100,
	0b00000100,
	0b00001000,
	0b00010000,
	0b00011100,

	// :
	0b00000000,
	0b00010000,
	0b00000000,
	0b00010000,
	0b00000000,

	// !
	0b00010000,
	0b00010000,
	0b00010000,
	0b00000000,
	0b00010000,

	// -
	0b00000000,
	0b00000000,
	0b00011100,
	0b00000000,
	0b00000000,
};

void drawLetter(RGBmatrixPanel *matrix, int8_t x, int8_t y, int8_t c, uint16_t &colour) {
	for(int8_t xi = 0; xi < 5; xi++) {
		for(int8_t yi = 0; yi < 5; yi++) {
			if(x + xi >= 0 && x + xi < 16 && y + yi >= 0 && y + yi < 32) {
				/*// byte row = yi / 2
				// byte hi/lo = yi % 2
				if(pgm_read_byte(&numbers[(3 * n) + (yi / 2)]) & (0b10000000 >> (((yi % 2) * 4) + xi))) {
					matrix->drawPixel(31 - (y + yi), x + xi, colour);
				}*/
				if(pgm_read_byte(&letters[(5 * c) + yi]) & (0b00010000 >> xi)) {
					matrix->drawPixel(31 - (y + yi), x + xi, colour);
				}
			}
		}
	}
}

inline uint8_t getLetterWidth(int8_t c) {
	if(c == 'w' || c == 'n' || c == 'm') {
		return 6;
	}
	if(c == 'g' || c == 'q' || c == 'k' || c == 'g') {
		return 5;
	}
	return 4;
}

void drawString(RGBmatrixPanel *matrix, int8_t x, int8_t y, char *str) {
	char c = *str;
	uint16_t colour = matrix->Color888(255, 255, 255, true);
	while(c != 0) {
		// handle colour changes
		if(c == '@') {
			colour = matrix->Color888(255, 0, 0, true);
		}
		else if(c == '#') {
			colour = matrix->Color888(255, 127, 0, true);
		}
		else if(c == '$') {
			colour = matrix->Color888(0, 127, 255, true);
		}
		else if(c == '^') {
			colour = matrix->Color888(127, 0, 127, true);
		}
		else if(c == '&') {
			colour = matrix->Color888(0, 255, 0, true);
		}

		// handle characters
		else if(c >= 'a' && c <= 'z') {
			// it's a letter
			drawLetter(matrix, x, y, c - 'a', colour);
			x += getLetterWidth(c);
		}
		else if(c == ':') {
			drawLetter(matrix, x, y, 26, colour);
			x += 2;
		}
		else if(c == '!') {
			drawLetter(matrix, x, y, 27, colour);
			x += 2;
		}
		else if(c == '-') {
			drawLetter(matrix, x, y, 28, colour);
			x += 3;
		}
		else if(c >= '0' && c <= '9') {
			// it's a number
			drawDigit(matrix, x, y, c - '0', colour);
			x += 4;
		}
		else if(c == ' ') {
			x += 4;
		}
		c = *(++str);
	}
}

int8_t getStringWidth(char *str) {
	char c = *str;
	int8_t width = 0;
	while(c != 0) {
		if(c >= 'a' && c <= 'z') {
			// it's a letter
			width += getLetterWidth(c);
		}
		else if((c >= '0' && c <= '9') || c == ' ') {
			// it's a number
			width += 4;
		}
		else if(c == '!' || c == ':') {
			width += 2;
		}
		else if(c == '-') {
			width += 3;
		}
		c = *(++str);
	}
	return width;
}

#endif
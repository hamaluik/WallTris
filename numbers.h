#ifndef _NUMBERS_H_
#define _NUMBERS_H_

PROGMEM uint8_t numbers[30] = {
	// 0
	0b00100101,
	0b01010101,
	0b00100000,

	// 1
	0b00100110,
	0b00100010,
	0b01110000,

	// 2
	0b01100001,
	0b00110100,
	0b01110000,

	// 3
	0b01100001,
	0b00100001,
	0b01100000,

	// 4
	0b01010101,
	0b00110001,
	0b00010000,

	// 5
	0b01110100,
	0b01100001,
	0b01100000,

	// 6
	0b00100100,
	0b01100101,
	0b00100000,

	// 7
	0b01110001,
	0b00100100,
	0b01000000,

	// 8
	0b00100101,
	0b00100101,
	0b00100000,

	// 9
	0b00100101,
	0b00110001,
	0b00100000
};

void drawDigit(RGBmatrixPanel *matrix, int8_t x, int8_t y, uint8_t n, uint16_t colour) {
	for(int8_t xi = 0; xi < 4; xi++) {
		for(int8_t yi = 0; yi < 5; yi++) {
			if(x + xi >= 0 && x + xi < 16 && y + yi >= 0 && y + yi < 32) {
				// byte row = yi / 2
				// byte hi/lo = yi % 2
				if(pgm_read_byte(&numbers[(3 * n) + (yi / 2)]) & (0b10000000 >> (((yi % 2) * 4) + xi))) {
					matrix->drawPixel(31 - (y + yi), x + xi, colour);
				}
			}
		}
	}
}

char numberStringBuffer[33];
void drawNumber(RGBmatrixPanel *matrix, int8_t x, int8_t y, uint32_t n, uint16_t colour) {
	sprintf(numberStringBuffer, "%lu", n);
	uint8_t len = strlen(numberStringBuffer);
	for(uint8_t xi = 0; xi < len; xi++) {
		drawDigit(matrix, x + (xi * 4), y, numberStringBuffer[xi] - '0', colour);
	}
}

int8_t numberLength(uint32_t n) {
	sprintf(numberStringBuffer, "%lu", n);
	return strlen(numberStringBuffer);
}

#endif
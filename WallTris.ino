#include <Adafruit_GFX.h>
#include <RGBmatrixPanel.h>
//#include <MemoryFree.h>

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
#include "ai.h"
#include "letters.h"

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
void setBufferPixel(uint8_t *buffer, int8_t x, int8_t y, uint8_t on) {
	// make sure we have a valid x and y
	if(x < 0 || y < 0 || x >= 10 || y >= 20) {
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
uint8_t getBufferPixel(uint8_t *buffer, int8_t x, int8_t y) {
	// make sure we have a valid x and y
	if(x < 0 || y < 0 || x >= 10 || y >= 20) {
		return 1;
	}

	return (*(buffer + (y * 2) + (x >> 3)) & (1 << (x % 8)));
}

void drawOutline() {
	uint16_t outlineColour = matrix.Color888(255, 255, 255, true);
	for(uint8_t y = 4; y < 25; y++) {
		matrix.drawPixel(32 - (y + 3), 0, outlineColour);
		matrix.drawPixel(32 - (y + 3), 11, outlineColour);
	}
	for(uint8_t x = 1; x < 11; x++) {
		//matrix.drawPixel(29, x, outlineColour);
		matrix.drawPixel(5, x, outlineColour);
	}
}

/**
 * Use this to draw / update the actual game region of the board
 */
void updatePlayingField() {
	for(int8_t y = 0; y < 20; y++) {
		for(int8_t x = 0; x < 10; x++) {
			uint8_t a = getBufferPixel(aBuffer, x, y) ? 1 : 0;
			uint8_t b = getBufferPixel(bBuffer, x, y) ? 1 : 0;
			uint8_t c = getBufferPixel(cBuffer, x, y) ? 1 : 0;

			uint16_t colour = 0;
			uint8_t type = (a ? 4 : 0) | (b ? 2 : 0) | (c ? 1 : 0);
			switch(type) {
				case 1:
					colour = getShapeColour(&shapeI[0]);
					break;
				case 2:
					colour = getShapeColour(&shapeT[0]);
					break;
				case 3:
					colour = getShapeColour(&shapeL[0]);
					break;
				case 4:
					colour = getShapeColour(&shapeJ[0]);
					break;
				case 5:
					colour = getShapeColour(&shapeS[0]);
					break;
				case 6:
					colour = getShapeColour(&shapeZ[0]);
					break;
				case 7:
					colour = getShapeColour(&shapeO[0]);
					break;
			}

			matrix.drawPixel(32 - (y + 7), x + 1, colour);
		}
	}
}

/**
 * Keep track of our AIs here
 */
AI aiGeneration[8];
uint32_t numGenerations = 0;
AI winners[4];
AI *currentAi;

char headerBuffer[48];
PROGMEM char headerBufferNamePrefix[] = "@tetris! #ai: ";
PROGMEM char headerBufferFormatGeneration[] = "^generation: %lu";
void buildHeaderBuffer() {
	static uint8_t showingName = 0;
	showingName = !showingName;
	if(showingName) {
		strcpy_P(headerBuffer, &headerBufferNamePrefix[0]);
		currentAi->enterName(&headerBuffer[14]);
	}
	else {
		sprintf_P(headerBuffer, &headerBufferFormatGeneration[0], numGenerations);
	}
}

char footerBuffer[48];
PROGMEM char footerBufferScoreFormat[] = "$score: %lu";
PROGMEM char footerBufferHiScoreFormat[] = "&hiscore: %lu";
void buildFooterBuffer(uint8_t showingHiScore) {
	static uint32_t maxScore = 0;
	static uint32_t score;
	score = currentAi->getScore();
	if(score > maxScore) {
		maxScore = score;
	}
	if(showingHiScore) {
		sprintf_P(footerBuffer, &footerBufferHiScoreFormat[0], maxScore);
	}
	else {
		sprintf_P(footerBuffer, &footerBufferScoreFormat[0], score);
	}
}

uint8_t drawFooter = 1;
void scrollText() {
	static int8_t headerLocation = 16;
	static int8_t footerLocation = 16;
	static unsigned long time, lastTime = 0, updateTime = 75;
	static uint8_t showingMaxScore = 0;

	time = millis();
	// deal with scrolling the score
	if(time - lastTime >= updateTime) {
		headerLocation--;
		if(headerLocation < (-1 * getStringWidth(headerBuffer))) {
			headerLocation = 16;
			buildHeaderBuffer();
		}
		matrix.fillRect(26, 0, 6, 16, 0);
		drawString(&matrix, headerLocation, 0, headerBuffer);

		if(drawFooter) {
			footerLocation--;
			buildFooterBuffer(showingMaxScore);
			if(footerLocation < (-1 * getStringWidth(footerBuffer))) {
				footerLocation = 16;
				showingMaxScore = !showingMaxScore;
			}
			matrix.fillRect(0, 0, 5, 16, 0);
			drawString(&matrix, footerLocation, 27, footerBuffer);
		}

		lastTime = time;
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

	// start our serial connection
	//Serial.begin(115200l);

	// draw the board outline
	drawOutline();

	// set up our AIs
	for(uint8_t i = 0; i < 8; i++) {
		aiGeneration[i].randomize();
	}
	currentAi = &aiGeneration[0];
	currentAi->drawHash(&matrix);
	buildHeaderBuffer();
	//currentAi->enterName(&headerBuffer[0]);
	//currentAi->printName();
	//Serial.println(F("\n###############"));
	//currentAi->printWeights();

	/*matrix.fillRect(26, 11, 6, 5, 0);
	drawNumber(&matrix, 12, 0, currentAi->getNumRuns(), matrix.Color888(255, 128, 0, true));*/
}

uint8_t getBufferValue(int8_t x, int8_t y) {
	uint8_t a = getBufferPixel(aBuffer, x, y) ? 1 : 0;
	uint8_t b = getBufferPixel(bBuffer, x, y) ? 1 : 0;
	uint8_t c = getBufferPixel(cBuffer, x, y) ? 1 : 0;
	return ((a ? 4 : 0) | (b ? 2 : 0) | (c ? 1 : 0));
}

uint8_t intersects(uint16_t *shape, int16_t x, int16_t y, uint8_t rotation) {
	uint16_t shapeRaw = pgm_read_word(shape + rotation);
	for(int8_t yi = 0; yi < 4; yi++) {
		for(int8_t xi = 0; xi < 4; xi++) {
			// make sure the shape exists at this test point
			if(shapeRaw & (32768 >> ((yi * 4) + xi))) {
				// test if it's hitting a boundary
				if(x + xi < 0 || x + xi >= 10 || y + yi < 0 || y + yi >= 20) {
					return 1;
				}

				// grab the field at that location
				if(getBufferValue(x + xi, y + yi)) {
					return 1;
				}
			}
		}
	}
	return 0;
}

/**
 * Place a shape into the field
 */
void emplaceShape(uint16_t *shape, int8_t x, int8_t y, uint8_t rotation) {
	uint8_t type = 0;
	if(shape == &shapeI[0]) {
		type = 1;
	}
	else if(shape == &shapeT[0]) {
		type = 2;
	}
	else if(shape == &shapeL[0]) {
		type = 3;
	}
	else if(shape == &shapeJ[0]) {
		type = 4;
	}
	else if(shape == &shapeS[0]) {
		type = 5;
	}
	else if(shape == &shapeZ[0]) {
		type = 6;
	}
	else if(shape == &shapeO[0]) {
		type = 7;
	}
	uint16_t shapeRaw = pgm_read_word(shape + rotation);
	for(int8_t yi = 0; yi < 4; yi++) {
		for(int8_t xi = 0; xi < 4; xi++) {
			// make sure the shape exists at this test point
			if(shapeRaw & (32768 >> ((yi * 4) + xi))) {
				if(type & 4) {
					setBufferPixel(aBuffer, x + xi, y + yi, 1);
				}
				if(type & 2) {
					setBufferPixel(bBuffer, x + xi, y + yi, 1);
				}
				if(type & 1) {
					setBufferPixel(cBuffer, x + xi, y + yi, 1);
				}
			}
		}
	}
}

/**
 * Remove a piece from the field
 */
void removeShape(uint16_t *shape, int8_t x, int8_t y, uint8_t rotation) {
	uint16_t shapeRaw = pgm_read_word(shape + rotation);
	for(int8_t yi = 0; yi < 4; yi++) {
		for(int8_t xi = 0; xi < 4; xi++) {
			// make sure the shape exists at this test point
			if(shapeRaw & (32768 >> ((yi * 4) + xi))) {
				setBufferPixel(aBuffer, x + xi, y + yi, 0);
				setBufferPixel(bBuffer, x + xi, y + yi, 0);
				setBufferPixel(cBuffer, x + xi, y + yi, 0);
			}
		}
	}
}

/**
 * Utility function to determine if a block can spawn or not
 */
uint8_t canSpawn() {
	for(uint8_t y = 0; y < 2; y++) {
		for(uint8_t x = 3; x < 7; x++) {
			uint8_t a = getBufferPixel(aBuffer, x, y) ? 1 : 0;
			uint8_t b = getBufferPixel(bBuffer, x, y) ? 1 : 0;
			uint8_t c = getBufferPixel(cBuffer, x, y) ? 1 : 0;
			if(a || b || c) {
				return 0;
			}
		}
	}

	return 1;
}

/**
 * Utility function to clear the playing field
 */
void clearField() {
	for(uint8_t y = 0; y < 20; y++) {
		for(uint8_t x = 0; x < 10; x++) {
			setBufferPixel(aBuffer, x, y, 0);
			setBufferPixel(bBuffer, x, y, 0);
			setBufferPixel(cBuffer, x, y, 0);
		}
	}
}

/**
 * Determine a score for the given move
 * Picking the lowest score should give us the best move
 * Current parameters taken from "Using a Genetic Algorithm to Weight an Evaluation Function for Tetris"
 * by Landon Flom and Cliff Robinson (Colorado State University, Department of Computer Science)
 */
long moveFitness(uint16_t *shape, int8_t x, int8_t y, uint8_t rotation) {
	// place the shape into the buffer (for now)
	emplaceShape(shape, x, y, rotation);

	// evaluate the pile height (highest column)
	int maxPileHeight = 0;
	for(uint8_t x = 0; x < 10; x++) {
		for(uint8_t y = 0; y < 20; y++) {
			if(getBufferValue(x, y)) {
				if((20 - y) > maxPileHeight) {
					maxPileHeight = (20 - y);
					y = 20;
					break;
				}
			}
		}
	}

	// count the number of holes (open spaces that we can't fill because a block is in the way)
	int numHoles = 0;
	for(int8_t x = 0; x < 10; x++) {
		// find the highest block
		for(int8_t y = 0; y < 20; y++) {
			if(getBufferValue(x, y)) {
				// ok, we have a block,
				// count how many openings there are beneath it
				for(int8_t yi = y + 1; yi < 20; yi++) {
					if(!getBufferValue(x, yi)) {
						numHoles++;
					}
				}
				// move on to the next column
				y = 20;
				break;
			}
			/*if(!getBufferValue(x, y)
				&& getBufferValue(x - 1, y)
				&& getBufferValue(x + 1, y)
				&& getBufferValue(x, y - 1)
				&& getBufferValue(x, y + 1)) {
					numHoles++;
			}*/
		}
	}
	
	// count the number of wells
	int numWells = 0;
	for(int8_t x = 0; x < 10; x++) {
		for(int8_t y = 0; y < 20; y++) {
			// determine if the left and right of this are blocked
			if(getBufferValue(x - 1, y) && getBufferValue(x + 1, y)) {
				// yup, they are
				// see if we have at least 2 empty blocks below us
				if(!getBufferValue(x, y + 1) && !(getBufferValue(x, y + 2))) {
					// yup, we have a well!
					numWells++;
					// move down to a new well
					y += 3;
				}
			}
		}
	}
	
	// count the number of lines made
	int numLines = 0;
	for(int8_t y = 0; y < 20; y++) {
		uint8_t lineFull = 1;
		for(int8_t x = 0; x < 10 && lineFull; x++) {
			if(!getBufferValue(x, y)) {
				lineFull = 0;
				break;
			}
		}
		if(lineFull) {
			numLines++;
		}
	}

	// remove the number of lines from the total height (we will be clearing them after all)
	maxPileHeight -= numLines;
	if(maxPileHeight < 0) {
		maxPileHeight = 0;
	}

	// calculate the "bumpiness"
	int bumpiness = 0;
	int8_t heights[10];
	for(int8_t x = 0; x < 10; x++) {
		// find the highest non-open block
		for(int8_t y = 0; y < 20; y++) {
			if(!getBufferValue(x, y)) {
				heights[x] = 20 - y;
				break;
			}
		}
	}
	for(uint8_t i = 1; i < 10; i++) {
		int8_t diff = heights[i] - heights[i - 1];
		if(diff < 0) {
			diff *= -1;
		}
		bumpiness += diff;
	}

	// calculate the number of blockades
	int numBlockades = 0;
	for(int8_t x = 0; x < 10; x++) {
		for(int8_t y = 0; y < 20; y++) {
			// if this is a block, see if there any spaces below it
			if(getBufferValue(x, y)) {
				uint8_t hasSpaces = 0;
				for(int8_t yi = y + 1; !hasSpaces && yi < 20; yi++) {
					if(!getBufferValue(x, yi)) {
						hasSpaces = 1;
						break;
					}
				}
				if(hasSpaces) {
					numBlockades++;
				}
			}
		}
	}
	 
	// ok, now that we're done counting, lets remove the shape
	removeShape(shape, x, y, rotation);

	// now calculate the fitness
	return 	(  long(currentAi->weights[0] * maxPileHeight)
			+ long(currentAi->weights[1] * numHoles)
			+ long(currentAi->weights[2] * numWells)
			+ long(currentAi->weights[3] * numLines)
			+ long(currentAi->weights[4] * bumpiness)
			+ long(currentAi->weights[5] * numBlockades));
}

/**
 * Clear lines and animate it
 */
uint8_t clearLines() {
	static uint16_t white = matrix.Color888(255, 255, 255, true);

	// determine which lines we need to clear
	uint8_t numCleared = 0;
	uint8_t clearLines[20];
	for(int8_t y = 0; y < 20; y++) {
		uint8_t lineFull = 1;
		for(int8_t x = 0; x < 10 && lineFull; x++) {
			if(!getBufferValue(x, y)) {
				lineFull = 0;
				break;
			}
		}
		if(lineFull) {
			clearLines[y] = 1;
			numCleared++;
		}
		else {
			clearLines[y] = 0;
		}
	}

	// quit if we didn't clear anything
	if(!numCleared) {
		return 0;
	}

	// don't draw the footer while we're flashing the score gained
	drawFooter = 0;

	// draw our added score
	matrix.fillRect(0, 0, 5, 16, 0);
	switch(numCleared) {
		case 1:
			drawNumber(&matrix, 4, 27, 40, matrix.Color888(255, 0, 0, true));
			break;
		case 2:
			drawNumber(&matrix, 2, 27, 120, matrix.Color888(255, 128, 0, true));
			break;
		case 3:
			drawNumber(&matrix, 2, 27, 300, matrix.Color888(255, 255, 0, true));
			break;
		case 4:
			drawNumber(&matrix, 0, 27, 1200, matrix.Color888(0, 255, 0, true));
			break;
	}

	// flash the lines white
	for(uint8_t y = 0; y < 20; y++) {
		if(clearLines[y]) {
			// we need to clear it!
			for(uint8_t x = 0; x < 10; x++) {
				matrix.drawPixel(32 - (y + 7), x + 1, white);
			}
		}
	}

	// wait a bit
	static unsigned long startTime;
	startTime = millis();
	while(millis() - startTime < 25) {
		scrollText();
	}
	//delay(25);

	// clear them!
	for(uint8_t y = 0; y < 20; y++) {
		if(clearLines[y]) {
			// we need to clear it!
			for(int8_t x = 0; x < 10; x++) {
				setBufferPixel(aBuffer, x, y, 0);
				setBufferPixel(bBuffer, x, y, 0);
				setBufferPixel(cBuffer, x, y, 0);
			}
		}
	}
	updatePlayingField();

	// start dropping things
	// (move bottom to top so we don't overwrite stuff)
	for(int8_t y = 19; y >= 0; --y) {
		// count how many blocks were cleared below us
		uint8_t numBelow = 0;
		for(int8_t yi = y + 1; yi < 20; yi++) {
			if(yi <= 20 && clearLines[yi]) {
				numBelow++;
			}
		}

		// move us down by that many blocks!
		if(numBelow > 0) {
			// wait a bit
			startTime = millis();
			while(millis() - startTime < 25) {
				scrollText();
			}

			for(uint8_t x = 0; x < 10; x++) {
				// get our current values
				uint8_t a = getBufferPixel(aBuffer, x, y) ? 1 : 0;
				uint8_t b = getBufferPixel(bBuffer, x, y) ? 1 : 0;
				uint8_t c = getBufferPixel(cBuffer, x, y) ? 1 : 0;

				// clear our current values
				setBufferPixel(aBuffer, x, y, 0);
				setBufferPixel(bBuffer, x, y, 0);
				setBufferPixel(cBuffer, x, y, 0);

				// set our values at the new locations
				setBufferPixel(aBuffer, x, y + numBelow, a);
				setBufferPixel(bBuffer, x, y + numBelow, b);
				setBufferPixel(cBuffer, x, y + numBelow, c);
			}
			updatePlayingField();
		}
	}
	// clear our added score area
	matrix.fillRect(0, 0, 5, 16, 0);

	// renable footer drawing
	drawFooter = 1;

	return numCleared;
}

/**
 * Keep track of our AI's goals
 */
uint8_t goalFound = 0;
int8_t goalRotation = 0;
int8_t goalX = 0;
int8_t goalY = 0;

// keep track of where our current piece is
int8_t trueRotation = 0;
int8_t trueX = 3;
int8_t trueY = -1;

unsigned long dropTime = 100;
unsigned long updateTime = 50;

AI tempAI;
void swapAI(AI &a, AI &b) {
	tempAI = a;
	a = b;
	b = tempAI;
}

void shuffleAIs() {
	for(int i = 7; i > 0; --i) {
		swapAI(aiGeneration[i], aiGeneration[random(0, i + 1)]);
	}
}

/**
 * Main program loop
 */
void loop() {
	static uint16_t white = matrix.Color888(255, 255, 255, true);
	static uint16_t *shape = randomShape();
	static uint16_t *nextShape = randomShape();
	static uint16_t *next2Shape = randomShape();
	static uint16_t *next3Shape = randomShape();
	static uint16_t *next4Shape = randomShape();
	
	// draw our new shape queue
	drawShape(nextShape, 11, 0, 0, getShapeColour(nextShape));
	drawShape(next2Shape, 11, 3, 0, getShapeColour(next2Shape));
	drawShape(next3Shape, 11, 6, 0, getShapeColour(next3Shape));
	drawShape(next4Shape, 11, 9, 0, getShapeColour(next4Shape));

	// now animate our piece going into position
	uint8_t emplaced = 0;
	unsigned long time = millis();
	unsigned long lastTimeUpdate = time, lastDropUpdate = time;
	while(!emplaced) {
		time = millis();

		// scroll our score text
		scrollText();

		// deal with AI updates
		if(time - lastTimeUpdate >= updateTime) {
			// choose our action
			// make finding the goal a top priority
			if(!goalFound) {
				// find our goal location
				long bestFitness = -10000000;

				for(uint8_t rotation = 0; rotation < 4; rotation++) {
					// draw our starting shape
					drawShape(shape, trueX, trueY, trueRotation, getShapeColour(shape, 5 - rotation));

					// loop through all our x-positions
					int8_t xmin = leftMostX(shape, rotation);
					int8_t xmax = rightMostX(shape, rotation);
					for(int8_t x = xmin; x <= xmax; x++) {
						// figure out the lowest it can go in this column
						for(int8_t y = 0; y < 20; y++) {
							// scroll our score text
							scrollText();

							// see if we'll intersect if we go any lower
							if(intersects(shape, x, y + 1, rotation)) {
								// draw it
								/*drawShape(shape, x, y, rotation, white);
								delay(50);
								drawShape(shape, x, y, rotation, 0);*/

								// get the fitness for here
								long fitness = moveFitness(shape, x, y, rotation);
								// see if it's better
								if(fitness > bestFitness) {
									goalRotation = rotation;
									goalX = x;
									goalY = y;
									bestFitness = fitness;
								}
								// don't go any deeper
								break;
							}
						}
					}
				}
				goalFound = 1;

				// draw our starting shape
				//drawShape(shape, trueX, -3, trueRotation, 0);
				drawShape(shape, trueX, trueY, trueRotation, getShapeColour(shape));
			}

			// allow hard drops if we've hit our target rotation and x location
			/*if(trueRotation == goalRotation && trueX == goalX) {
				// find the y that we can drop to
				for(int8_t y = trueY; y < 20; y++) {
					if(intersects(shape, trueX, y + 1, trueRotation)) {
						// found it!
						trueY = y;
						y = 20;
						break;
					}
				}
			}*/

			// deal with rotation next
			if(trueRotation != goalRotation) {
				// figure out the direction to rotate in
				int8_t oldRotation = trueRotation;
				if(goalRotation > trueRotation) {
					trueRotation++;
					if(trueRotation == 4) {
						trueRotation = 0;
					}
				}
				else {
					trueRotation--;
					if(trueRotation == -1) {
						trueRotation = 3;
					}
				}

				// determine if this rotation is valid or not
				uint8_t rotationFailed = 0;
				if(intersects(shape, trueX, trueY, trueRotation))  {
					// shit!
					// try wall-kicking against the left
					if(!intersects(shape, trueX + 1, trueY, trueRotation)) {
						// yay! we can wall-kick against the left
						trueX++;
					}
					// nope, try against the right?
					else if(!intersects(shape, trueX -1, trueY, trueRotation)) {
						// yay! we can wall-kick against the right
						trueX--;
					}
					else {
						// welp, no rotating for us to do!
						rotationFailed = 1;
					}
				}

				if(rotationFailed) {
					trueRotation = oldRotation;
				}
				else {
					// erase our old drawing
					drawShape(shape, trueX, trueY, oldRotation, 0);
					// draw our new rotated shape
					drawShape(shape, trueX, trueY, trueRotation, getShapeColour(shape));
				}
			}
			// deal with motion
			else if(trueX != goalX) {
				// erase our old drawing
				drawShape(shape, trueX, trueY, trueRotation, 0);

				if(goalX > trueX) {
					trueX++;
					// don't intersect with shit!
					if(intersects(shape, trueX, trueY, trueRotation)) {
						trueX--;
					}
				}
				else {
					trueX--;
					// don't intersect with shit!
					if(intersects(shape, trueX, trueY, trueRotation)) {
						trueX++;
					}
				}

				// draw our new translated shape
				drawShape(shape, trueX, trueY, trueRotation, getShapeColour(shape));
			}

			// update our last time
			lastTimeUpdate = time;
		}

		// deal with drop updates
		if(time - lastDropUpdate >= dropTime) {
			// figure out if it will intersect on next drop
			if(intersects(shape, trueX, trueY + 1, trueRotation)) {
				// yup!
				// place it!
				emplaceShape(shape, trueX, trueY, trueRotation);
				updatePlayingField();
				emplaced = 1;
				drawOutline();

				// and reset
				trueY = 0;
				trueX = 3;
				trueRotation = 0;
				goalFound = 0;
			}
			else {
				// remove the old shape
				drawShape(shape, trueX, trueY, trueRotation, 0);

				// drop it!
				trueY++;

				// draw it in it's new position
				drawShape(shape, trueX, trueY, trueRotation, getShapeColour(shape));
			}

			// update our last time
			lastDropUpdate = time;
		}
	}

	// clear any lines that we have made
	while(currentAi->addLines(clearLines()));

	// if we can't spawn a new one, clear and start over!
	if(!canSpawn()) {
		clearField();
		updatePlayingField();

		static uint32_t lastScore = 0;
		//Serial.print(F("\tRun ")); Serial.print(currentAi->getNumRuns()); Serial.print(F(" completed with score: ")); Serial.println(currentAi->getScore() - lastScore);
		lastScore = currentAi->getScore();

		// tell our AI that it's done
		currentAi->newRun();

		// figure out if we need a new AI or not
		if(currentAi->getNumRuns() >= 1) {
			// yup, we do!
			//Serial.println();
			//Serial.print(F("\tScore: ")); Serial.println(currentAi->getScore());
			/*Serial.print(F("Score: "));
			Serial.println(currentAi->getScore());*/

			// move on to the next AI
			currentAi++;
			//aiNumber++;
			if(currentAi > (&aiGeneration[7])) {
				// make a new generation!
				//Serial.println();
				//Serial.println(F("Determining winners..."));
				//Serial.print(F("\tFree memory: ")); Serial.println(freeMemory());
				numGenerations++;

				// shuffle our AIs
				//Serial.println(F("\tShuffling ais..."));
				shuffleAIs();

				// now pair them up and determine the winners
				for(uint8_t i = 0; i < 8; i += 2) {
					if(aiGeneration[i].getScore() > aiGeneration[i + 1].getScore()) {
						//Serial.print(F("\t")); aiGeneration[i].printName(); Serial.print(F(" beats ")); aiGeneration[i + 1].printName(); Serial.print(F(" with a score of ")); Serial.println(aiGeneration[i].getScore());
						winners[i / 2] = aiGeneration[i];
					}
					else {
						//Serial.print(F("\t")); aiGeneration[i + 1].printName(); Serial.print(F(" beats ")); aiGeneration[i].printName(); Serial.print(F(" with a score of ")); Serial.println(aiGeneration[i + 1].getScore());
						winners[i / 2] = aiGeneration[i + 1];
					}
				}

				//Serial.println(F("###############"));
				//Serial.println(F("Winners:"));
				/*for(uint8_t i = 0; i < 4; i++) {
					winners[i].printName(); Serial.print(F(", "));
				}*/
				//Serial.println(F("\n###############"));

				// now that we have our winners, lets breed new children!
				//Serial.println(F("Breeding new generation..."));
				for(uint8_t i = 0; i < 8; i++) {
					AI *winnerA = &winners[random(0, 4)];
					AI *winnerB = &winners[random(0, 4)];
					//Serial.print(F("Child ")); Serial.print(i); Serial.println(F(" has parents:"));
					//winnerA->printName(); Serial.print(F(", "));
					//winnerB->printName(); Serial.println();
					aiGeneration[i].breed(&winners[random(0, 4)], &winners[random(0, 4)]);
					//Serial.println(F("With weights:"));
					//aiGeneration[i].printWeights();
					//Serial.println(F("--------------"));
				}
				//Serial.println(F("Done!"));

				// reset our current AI
				currentAi = &aiGeneration[0];
			}

			// draw our new hash
			currentAi->drawHash(&matrix);
			//currentAi->enterName(&headerBuffer[0]);
			buildHeaderBuffer();
			//Serial.println();
			//currentAi->printName();
			//Serial.println(F("\n###############"));
			//currentAi->printWeights();
			lastScore = 0;
		}
		
		/*matrix.fillRect(26, 11, 6, 5, 0);
		drawNumber(&matrix, 12, 0, currentAi->getNumRuns(), matrix.Color888(255, 128, 0, true));*/
	}

	// clear our shape queue
	drawShape(nextShape, 11, 0, 0, 0);
	drawShape(next2Shape, 11, 3, 0, 0);
	drawShape(next3Shape, 11, 6, 0, 0);
	drawShape(next4Shape, 11, 9, 0, 0);

	// continue our shape queue
	shape = nextShape;
	nextShape = next2Shape;
	next2Shape = next3Shape;
	next3Shape = next4Shape;
	next4Shape = randomShape();

	// animate our shape queue
	for(int8_t i = 2; i >= 0; --i) {
		drawShape(nextShape, 11, i, 0, getShapeColour(nextShape));
		drawShape(next2Shape, 11, i + 3, 0, getShapeColour(next2Shape));
		drawShape(next3Shape, 11, i + 6, 0, getShapeColour(next3Shape));
		drawShape(next4Shape, 11, i + 9, 0, getShapeColour(next4Shape));
		delay(25);
		drawShape(nextShape, 11, i, 0, 0);
		drawShape(next2Shape, 11, i + 3, 0, 0);
		drawShape(next3Shape, 11, i + 6, 0, 0);
		drawShape(next4Shape, 11, i + 9, 0, 0);
	}
}

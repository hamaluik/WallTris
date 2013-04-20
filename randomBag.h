#ifndef _RANDOM_BAG_H_
#define _RANDOM_BAG_H_

class RandomBag {
private:
	uint8_t tiles[14];
	uint8_t *currentTile;

	void swapTiles(uint8_t &a, uint8_t &b) {
		uint8_t c = a;
		a = b;
		b = c;
	}

	// algorithm from http://www.cplusplus.com/reference/algorithm/random_shuffle/
	void shuffleBag(int offset) {
		for(int i = 6; i > 0; --i) {
			swapTiles(tiles[offset + i], tiles[offset + random(0, i + 1)]);
		}
	}

public:
	RandomBag() {
		currentTile = &tiles[0];
		// fill it with a sorted list
		for(int i = 0; i < 7; i++) {
			tiles[i] = i;
			tiles[i + 7] = i;
		}
		// and shuffle the bags!
		shuffleBag(0);
		shuffleBag(7);
	}

	uint8_t next() {
		currentTile++;
		if(currentTile == &tiles[7]) {
			// regenerate the first bag
			shuffleBag(0);
		}
		else if(currentTile == &tiles[13]) {
			// regenerate the second bag
			shuffleBag(7);
		}
		else if(currentTile > &tiles[13]) {
			// move back to the first tile in the first bag
			currentTile = &tiles[0];
		}
		return *currentTile;
	}
};

#endif
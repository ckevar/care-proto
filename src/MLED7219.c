#include "MLED7219.h"

#include <cstdio>
#include <cstring>
#include <cstdlib>

unsigned char CH[] = {
	0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000, // space 
	0b10000,  0b10000,  0b10000,  0b10000,  0b10000,  0b00000,  0b10000,  0b00000,  // ! 
	0b101000, 0b101000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000, // quotes 
	0b0000000, 0b0101000, 0b1111100, 0b0101000, 0b1111100, 0b0101000, 0b0000000, 0b0000000, // # 
	0b001000, 0b011100, 0b100000, 0b011000, 0b000100, 0b111000, 0b010000, 0b000000, // $ 
	0b110010, 0b110010, 0b000100, 0b001000, 0b010000, 0b100110, 0b100110, 0b000000, // % 
	0b010000, 0b101000, 0b101000, 0b010000, 0b101010, 0b100100, 0b011010, 0b000000, // & 
	0b10000,  0b10000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  0b00000,  // ' 
	0b001000, 0b010000, 0b100000, 0b100000, 0b100000, 0b010000, 0b001000, 0b000000, // ( 
	0b100000, 0b010000, 0b001000, 0b001000, 0b001000, 0b010000, 0b100000, 0b000000, // ) 
	0b000000, 0b001000, 0b001000, 0b111110, 0b010100, 0b100010, 0b000000, 0b000000, // * 
	0b000000, 0b001000, 0b001000, 0b111110, 0b001000, 0b001000, 0b000000, 0b000000, // + 
	0b000000, 0b000000, 0b000000, 0b000000, 0b110000, 0b110000, 0b010000, 0b100000, // 0, 
	0b000000, 0b000000, 0b000000, 0b111100, 0b000000, 0b000000, 0b000000, 0b000000, // - 
	0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b110000, 0b110000, 0b000000, // . 
	0b000100, 0b001000, 0b001000, 0b010000, 0b010000, 0b100000, 0b100000, 0b000000, // / 
	0b011000, 0b100100, 0b100100, 0b100100, 0b100100, 0b100100, 0b011000, 0b000000, // 0 
	0b010000, 0b110000, 0b010000, 0b010000, 0b010000, 0b010000, 0b111000, 0b000000, // 1 
	0b011000, 0b100100, 0b000100, 0b001000, 0b010000, 0b100000, 0b111100, 0b000000, // 2 
	0b011000, 0b100100, 0b000100, 0b001000, 0b000100, 0b100100, 0b011000, 0b000000, // 3 
	0b000100, 0b001100, 0b010100, 0b100100, 0b111100, 0b000100, 0b000100, 0b000000, // 4 
	0b111100, 0b100000, 0b111000, 0b000100, 0b000100, 0b100100, 0b011000, 0b000000, // 5 
	0b011000, 0b100000, 0b100000, 0b111000, 0b100100, 0b100100, 0b011000, 0b000000, // 6 
	0b111100, 0b000100, 0b000100, 0b001000, 0b010000, 0b100000, 0b100000, 0b000000, // 7 
	0b011000, 0b100100, 0b100100, 0b011000, 0b100100, 0b100100, 0b011000, 0b000000, // 8 
	0b011000, 0b100100, 0b100100, 0b011100, 0b000100, 0b000100, 0b011000, 0b000000, // 9 
	0b00000,  0b00000,  0b00000,  0b00000,  0b10000,  0b00000,  0b10000,  0b00000,  // : 
	0b000000, 0b000000, 0b000000, 0b000000, 0b010000, 0b000000, 0b010000, 0b100000, // ; 
	0b000000, 0b000000, 0b001000, 0b010000, 0b100000, 0b010000, 0b001000, 0b000000, // < 
	0b000000, 0b000000, 0b111100, 0b000000, 0b111100, 0b000000, 0b000000, 0b000000, // = 
	0b000000, 0b000000, 0b100000, 0b010000, 0b001000, 0b010000, 0b100000, 0b000000, // > 
	0b011000, 0b100100, 0b000100, 0b011000, 0b010000, 0b000000, 0b010000, 0b000000, // ? 
	0b011100, 0b100010, 0b101110, 0b110110, 0b101100, 0b100000, 0b011100, 0b000000, // @ 
	0b011000, 0b100100, 0b100100, 0b100100, 0b111100, 0b100100, 0b100100, 0b000000, // A 
	0b111000, 0b100100, 0b100100, 0b111000, 0b100100, 0b100100, 0b111000, 0b000000, // B 
	0b011000, 0b100100, 0b100000, 0b100000, 0b100000, 0b100100, 0b011000, 0b000000, // C 
	0b111000, 0b100100, 0b100100, 0b100100, 0b100100, 0b100100, 0b111000, 0b000000, // D 
	0b111100, 0b100000, 0b100000, 0b111000, 0b100000, 0b100000, 0b111100, 0b000000, // E 
	0b111100, 0b100000, 0b100000, 0b111000, 0b100000, 0b100000, 0b100000, 0b000000, // F 
	0b011000, 0b100100, 0b100000, 0b101100, 0b100100, 0b100100, 0b011100, 0b000000, // G 
	0b100100, 0b100100, 0b100100, 0b111100, 0b100100, 0b100100, 0b100100, 0b000000, // H 
	0b111000, 0b010000, 0b010000, 0b010000, 0b010000, 0b010000, 0b111000, 0b000000, // I 
	0b001100, 0b000100, 0b000100, 0b000100, 0b100100, 0b100100, 0b011000, 0b000000, // J 
	0b100100, 0b100100, 0b101000, 0b110000, 0b101000, 0b100100, 0b100100, 0b000000, // K 
	0b100000, 0b100000, 0b100000, 0b100000, 0b100000, 0b100000, 0b111100, 0b000000, // L 
	0b100010, 0b110110, 0b101010, 0b101010, 0b100010, 0b100010, 0b100010, 0b000000, // M 
	0b100010, 0b100010, 0b110010, 0b101010, 0b100110, 0b100010, 0b100010, 0b000000, // N 
	0b011000, 0b100100, 0b100100, 0b100100, 0b100100, 0b100100, 0b011000, 0b000000, // O 
	0b111000, 0b100100, 0b100100, 0b111000, 0b100000, 0b100000, 0b100000, 0b000000, // P 
	0b011000, 0b100100, 0b100100, 0b100100, 0b100100, 0b100100, 0b011000, 0b000100, // Q 
	0b111000, 0b100100, 0b100100, 0b111000, 0b100100, 0b100100, 0b100100, 0b000000, // R 
	0b011000, 0b100100, 0b100000, 0b011000, 0b000100, 0b000100, 0b111000, 0b000000, // S 
	0b111110, 0b001000, 0b001000, 0b001000, 0b001000, 0b001000, 0b001000, 0b000000, // T 
	0b100100, 0b100100, 0b100100, 0b100100, 0b100100, 0b100100, 0b011000, 0b000000, // U 
	0b100010, 0b100010, 0b100010, 0b100010, 0b010100, 0b010100, 0b001000, 0b000000, // V 
	0b100010, 0b100010, 0b100010, 0b101010, 0b101010, 0b101010, 0b010100, 0b000000, // W 
	0b100010, 0b100010, 0b010100, 0b001000, 0b010100, 0b100010, 0b100010, 0b000000, // X 
	0b100010, 0b100010, 0b100010, 0b010100, 0b001000, 0b001000, 0b001000, 0b000000, // Y 
	0b111100, 0b000100, 0b000100, 0b001000, 0b010000, 0b100000, 0b111100, 0b000000, // Z 
	0b11000,  0b10000,  0b10000,  0b10000,  0b10000,  0b10000,  0b11000,  0b00000,  // [ 
	0b100000, 0b010000, 0b010000, 0b001000, 0b001000, 0b000100, 0b000100, 0b000000, // "\"" 
	0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000, // empty 
	0b11000,  0b01000,  0b01000,  0b01000,  0b01000,  0b01000,  0b11000,  0b00000,  // ] 
	0b010000, 0b101000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000, // hat 
	0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b111100, 0b000000, // _ 
	0b100000, 0b010000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000, 0b000000, // ` 
	0b000000, 0b000000, 0b011000, 0b000100, 0b011100, 0b100100, 0b011100, 0b000000, // a 
	0b100000, 0b100000, 0b111000, 0b100100, 0b100100, 0b100100, 0b111000, 0b000000, // b 
	0b000000, 0b000000, 0b011000, 0b100100, 0b100000, 0b100100, 0b011000, 0b000000, // c 
	0b000100, 0b000100, 0b011100, 0b100100, 0b100100, 0b100100, 0b011100, 0b000000, // d 
	0b000000, 0b000000, 0b011000, 0b100100, 0b111100, 0b100000, 0b011000, 0b000000, // e 
	0b001000, 0b010000, 0b111000, 0b010000, 0b010000, 0b010000, 0b010000, 0b000000, // f 
	0b000000, 0b000000, 0b011000, 0b100100, 0b100100, 0b011100, 0b000100, 0b111000, // g 
	0b100000, 0b100000, 0b111000, 0b100100, 0b100100, 0b100100, 0b100100, 0b000000, // h 
	0b010000, 0b000000, 0b110000, 0b010000, 0b010000, 0b010000, 0b111000, 0b000000, // i 
	0b000100, 0b000000, 0b001100, 0b000100, 0b000100, 0b000100, 0b100100, 0b011000, // j 
	0b100000, 0b100000, 0b100100, 0b101000, 0b110000, 0b101000, 0b100100, 0b000000, // k 
	0b110000, 0b010000, 0b010000, 0b010000, 0b010000, 0b010000, 0b111000, 0b000000, // l 
	0b000000, 0b000000, 0b111100, 0b101010, 0b101010, 0b101010, 0b101010, 0b000000, // m 
	0b000000, 0b000000, 0b111000, 0b100100, 0b100100, 0b100100, 0b100100, 0b000000, // n 
	0b000000, 0b000000, 0b011000, 0b100100, 0b100100, 0b100100, 0b011000, 0b000000, // o 
	0b000000, 0b000000, 0b111000, 0b100100, 0b100100, 0b111000, 0b100000, 0b100000, // p 
	0b000000, 0b000000, 0b011100, 0b100100, 0b100100, 0b011100, 0b000100, 0b000100, // q 
	0b000000, 0b000000, 0b101100, 0b110000, 0b100000, 0b100000, 0b100000, 0b000000, // r 
	0b000000, 0b000000, 0b011100, 0b100000, 0b011000, 0b000100, 0b111000, 0b000000, // s 
	0b010000, 0b010000, 0b111000, 0b010000, 0b010000, 0b010000, 0b001000, 0b000000, // t 
	0b000000, 0b000000, 0b100100, 0b100100, 0b100100, 0b100100, 0b011100, 0b000000, // u 
	0b000000, 0b000000, 0b100010, 0b100010, 0b100010, 0b010100, 0b001000, 0b000000, // v 
	0b000000, 0b000000, 0b101010, 0b101010, 0b101010, 0b101010, 0b010100, 0b000000, // w 
	0b000000, 0b000000, 0b100010, 0b010100, 0b001000, 0b010100, 0b100010, 0b000000, // x 
	0b000000, 0b000000, 0b100100, 0b100100, 0b100100, 0b011100, 0b000100, 0b111000, // y 
	0b000000, 0b000000, 0b111000, 0b001000, 0b010000, 0b100000, 0b111000, 0b000000, // z 
	0b001000, 0b010000, 0b010000, 0b100000, 0b010000, 0b010000, 0b001000, 0b000000, // { 
	0b10000,  0b10000,  0b10000,  0b10000,  0b10000,  0b10000,  0b10000,  0b00000,  // | 
	0b100000, 0b010000, 0b010000, 0b001000, 0b010000, 0b010000, 0b100000, 0b000000, // } 
	0b000000, 0b000000, 0b010100, 0b101000, 0b000000, 0b000000, 0b000000, 0b000000, // ~ 
 };

int MLED7219::init(int cs) {
	if(openSPI(cs, SPI_MODE_0, 1000000) < 0)
		return -1;
	
	if(reset() < 0) {
		fprintf(stderr, "[error] reseting MAX7219\n");
		return -1;
	}

	if(setScanLimit(7) < 0) {
		fprintf(stderr, "[error] set scan limit \n");
		return -1;
	}

	if(disableShutdown() < 0) {
		fprintf(stderr, "[error] disable shutdown mode\n");
		return -1;
	}
	memset(led, 0, 350);
	return 0;
}

int MLED7219::setxy(unsigned char col, unsigned char row) {
	return updatexy(col, row, 1);
}

int MLED7219::clearxy(unsigned char col, unsigned char row) {
	return updatexy(col, row, 0);
}

int MLED7219::setBuff(unsigned char *ledMatrix) {
	
	for (int i = 0; i < 8; ++i) {
		buff[0] = i + 1;
		buff[1] = 0;
		for (int j = 0; j < 8; ++j) {
			printf("%d ", ledMatrix[i * 8 + j]);
			buff[1] <<= 1;
			if(ledMatrix[i * 8 + j])
				buff[1] |= 1;
		}
		printf("%d \n", buff[1]);
		if(writeSPI(buff, 2) < 0) {
			fprintf(stderr, "[error] while writing setbuff\n");
			return -1;
		}
	}
	return 0;
}

int MLED7219::clear() {
	memset(led, 0, 64);
	return setBuff(led);
}

int MLED7219::displayBuffer(int x, size_t length) {
	static int j = 0;
	for (size_t i = 0; i < length; ++i) {
		buff[0] = i + 1;
		buff[1] = (led[i + j] << x%8) | (led[i + j + 8] >> (7 - (x%8)));
		if(writeSPI(buff, 2) < 0) {
			fprintf(stderr, "[error] while writing setbuff\n");
			return -1;
		}
	}
	x++;
	if (x%8 == 0) j += 8; 
	if ((j + 8) > buffCounter) j = 0;

	return 0;	
}

int MLED7219::heart(int randomAllow) {
	unsigned char mleds[64] = {
		0,0,0,0,0,0,0,0,
		0,1,1,0,0,1,1,1,
		1,1,1,1,1,1,1,1,
		1,1,1,1,1,1,1,1,
		0,1,1,1,1,1,1,0,
		0,0,1,1,1,1,0,0,
		0,0,0,1,1,0,0,0,
		0,0,0,0,0,0,0,0};
	
	if (randomAllow)
		return randomBuffFail(mleds);
	else 
		return setBuff(mleds);
}

int MLED7219::displayHeart() {
	unsigned char mleds[8] = {
		0b00000000,
		0b01100110,
		0b11111111,
		0b11111111,
		0b01111110,
		0b00111100,
		0b00011000,
		0b00000000};
	memcpy(led, mleds, 8);
	return displayBuffer(0, 8);
}

int MLED7219::smile() {
	unsigned char mleds[8] = {
		0b00111100,
		0b01000010,
		0b10100101,
		0b10000001,
		0b10100101,
		0b10011001,
		0b01000010,
		0b00111100 };
	memcpy(led, mleds, 8);
	return displayBuffer(0, 8);
}

int MLED7219::displayStopSymbol() {
	unsigned char mleds[8] = {
		0b00000001, 
		0b00111010, 
		0b01000110, 
		0b10001001, 
		0b10010001, 
		0b01100010, 
		0b01111000, 
		0b10000000, // 0 
	};
	memcpy(led, mleds, 8);
	return displayBuffer(0, 8);
}


void MLED7219::printfbuff() {
	for(int i = 0; i < 64; i++) {
		printf("%d\n", led[i]);
		if (i%8 == 0) printf("\n\n");
	}
}

int MLED7219::loadChar2Buff(char c) {
	memcpy(led + buffCounter, CH + (c - 32) * 8, 8);
	buffCounter += 8;
	if (buffCounter > 43*8) {
		printf("buffCounter %d\n", buffCounter);
		buffCounter = 0;
		return -1;
	}
	return 0;
}
int MLED7219::loadString2Buff(const char *text, size_t count) {
	if (count > 43) {
		fprintf(stderr, "[warning] buffer only supports 8 char\n");
		count = 43;
	}
	for (size_t i = 0; i < count; i++) {
		if(loadChar2Buff(text[i]) < 0) {
			fprintf(stderr, "[error] couldnt render char\n");
			return - 1;
		}
	}
	return 0;
}

int MLED7219::displayQuestionMark() {
	char c = '?';
	memcpy(led, CH + (c - 32) * 8, 8);
	return displayBuffer(0, 8);
}

int MLED7219::randomBuffFail(unsigned char *ledMatrix) {
	unsigned char r = rand() % 10;
	for (int i = 0; i < 8; ++i) {
		buff[0] = i + 1;
		for (int j = 0; j < 8; ++j) {
			buff[1] <<= 1;
			if(ledMatrix[i * 8 + j] ) {
				if (r)
					buff[1] |= (rand() % 2);
				else
					buff[1] |= 1;
			}
		}
		if(writeSPI(buff, 2) < 0) {
			fprintf(stderr, "[error] while writing setbuff\n");
			return -1;
		}
	}
	return 0;
}

int MLED7219::intensityBreath() {
	// to be called as part of iteration
	static unsigned char i = 0;
	static unsigned char beat[] = {3, 3, 7, 11, 15, 13, 10, 9, 10, 12, 15, 12, 9, 7, 5, 3};
	i++;
	if (i > (sizeof(beat) + 1)) i = 0;
	return setIntensity(beat[i]);
}

int MLED7219::updatexy(unsigned char col, unsigned char row, unsigned char onOff) {
	if (col > 7) {
		fprintf(stderr, "[warning] col larger than matrix dims \n");
		col = 7;
	}

	if (row > 7) {
		fprintf(stderr, "[warning] row larger than matrix dims \n");
		row = 7;
	}

	led[col * 8 + row] = onOff;

	buff[0] = col + 1;
	buff[1] = 0;

	for(int i = 0; i < 8; i++) {
		buff[1] <<= 1;
		if(led[col * 8 + i])
			buff[1] |= 1;
	}

	return writeSPI(buff, 2);
}

int MLED7219::destroy() {
	return closeSPI();
}
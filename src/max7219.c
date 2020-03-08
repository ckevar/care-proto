#include "max7219.h"

#include <stdio.h>

int MAX7219::reset() {
	confBuff[1] = 0;
	for (unsigned char i = 0; i < 16; ++i) {
		confBuff[0] = i;
		if(writeSPI(confBuff, 2) < 0)
			return -1;
	}
	return 0;
}

int MAX7219::setScanLimit(unsigned char limit) {
	confBuff[0] = MAX7219_REG_SCAN_LIMIT;
	if (limit > 7) {
		fprintf(stderr, "[warning] maximum scan limit is 7\n");
		limit = 7;
	}
	confBuff[1] = limit;
	return writeSPI(confBuff, 2);
}

int MAX7219::disableShutdown() {
	confBuff[0] = MAX7219_REG_SHUTDOWN;
	confBuff[1] = MAX7219_NORMAL_MODE;
	return writeSPI(confBuff, 2);
} 

int MAX7219::shutdownMode() {
	confBuff[0] = MAX7219_REG_SHUTDOWN;
	confBuff[1] = MAX7219_SHUTDOWN_MODE;
	return writeSPI(confBuff, 2);
}

int MAX7219::setIntensity(unsigned char i) {
	confBuff[0] = MAX7219_REG_INTENSITY;
	confBuff[1] = i;
	return writeSPI(confBuff, 2);
}
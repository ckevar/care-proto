#ifndef MAX7219_H
#define MAX7219_H 

#include "spi.h"

// REGISTERS
#define MAX7219_REG_DIGIT_0			0x01
#define MAX7219_REG_DIGIT_1			0x02
#define MAX7219_REG_DIGIT_2			0x03
#define MAX7219_REG_DIGIT_3			0x04
#define MAX7219_REG_DIGIT_4			0x05
#define MAX7219_REG_DIGIT_5			0x06
#define MAX7219_REG_DIGIT_6			0x07
#define MAX7219_REG_DIGIT_7			0x08
#define MAX7219_REG_DECODE_MODE		0x09
#define MAX7219_REG_INTENSITY		0x0a
#define MAX7219_REG_SCAN_LIMIT 		0x0b
#define MAX7219_REG_SHUTDOWN		0x0c
#define MAX7219_REG_DISPLAY_TEST	0x0F

// REGISTERS COMMAND
#define MAX7219_SHUTDOWN_MODE		0x00
#define MAX7219_NORMAL_MODE			0x01

class MAX7219 : public SPIDev
{
public:
	MAX7219() :
		SPIDev() {
			confBuff[0] = 0; 
			confBuff[1] = 0;
		}

	int reset();
	int setScanLimit(unsigned char limit);
	int setIntensity(unsigned char i);
	int disableShutdown();
	int shutdownMode();
private:
	unsigned char confBuff[2];
	// ~MAX7219MLED();
};

#endif

#ifndef MLED7219_H
#define MLED7219_H 

#include "max7219.h"

class MLED7219 : public MAX7219
{
public:
	MLED7219() :
		MAX7219() {buffCounter = 0;}
	int init(int cs);
	int setxy(unsigned char col, unsigned char row);
	int clearxy(unsigned char col, unsigned char row);
	int clear();
	int intensityBreath();
	int setBuff(unsigned char *ledMatrix);
	int displayBuffer(int x, size_t length);
	int randomBuffFail(unsigned char *ledMatrix);
	int heart(int randomAllow);
	int loadChar2Buff(char c);
	int loadString2Buff(const char *text, size_t count);

	int displayStopSymbol();
	int displayQuestionMark();
	int displayHeart();
	int smile();

	void printfbuff();
	int destroy();
	// ~MLED7219();
private: 
	unsigned char led[43*8];
	unsigned short buffCounter;
	unsigned char buff[2];
	int updatexy(unsigned char col, unsigned char row, unsigned char onOff);
};

#endif

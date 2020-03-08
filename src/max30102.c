#include "max30102.h"

#include <stdio.h>

void MAX30102::reset() {
	sendOne(MAX30102_MODE_CONFIG, MAX30102_RESET);	// reset
}

int MAX30102::settings() {
	// read and clear status register
	reset();
	queryOne(MAX30102_INT_STATUS1, &status);

	/* INTERRUPTION ENABLE when:*/
	// the buffer is almos full, 
	// data is ready and,
	// there's ambient light that cannot be remove
	if(sendOne(MAX30102_INT_EN1, MAX30102_A_FULL | MAX30102_PPG_RDY | MAX30102_ALC_OVF)) // INT settings
		return -1;

	/* INTERRUPTION2 ENABLE When: */
	// none
	if(sendOne(MAX30102_INT_EN2, 0x00))
		return -1;

	/* buffer pointer starts at 0 */
	if (sendOne(MAX30102_FIFO_WR_PTR, 0x00))		// where the buffer starts within the device
		return -1; 
	/* clear overflow counter */
	if(sendOne(MAX30102_OVERFLOW_COUNTER, 0x00))
		return -1;
	/* buffer pointer starts at o */
	if(sendOne(MAX30102_FIFO_RD_PTR, 0x00))
		return -1;

	/* FIFO SETTINGS */
	// sample average 1
	// Fifo rollover false
	// FIFO almos full 17
	if(sendOne(MAX30102_FIFO_CONFIG, MAX30102_A_FULL_17))	
		return -1;

	/* MODE SETTINGS */
	// 0x03 spo2 mode
	if(sendOne(MAX30102_MODE_CONFIG, MAX30102_MODE_CTL_HR_SPO2))
		return -1;

	/* SPO2 SETTINGS */
	// range 4096nA
	// LED pulse width 411us
	#ifdef MAX30102_100HZ
		// sample rate 100Hz
		if(sendOne(MAX30102_SPO2_CONFIG, MAX30102_SPO2_ADC_RGE_4096 | MAX30102_SPO2_SR_100HZ | MAX30102_LED_PW_411))
			return -1;
	#else 	
		// sample rate 50Hz
		if(sendOne(MAX30102_SPO2_CONFIG, MAX30102_SPO2_ADC_RGE_4096 | MAX30102_SPO2_SR_50HZ | MAX30102_LED_PW_411))
			return -1;
	#endif

	/* LED 1 SETTINGS */
	// 7.2mA
	if(sendOne(MAX30102_LED_PULSE_AMPLITUD1, 0x24))
		return -1;

	/* LED 2 SETTINGS */
	// 7.2mA
	if(sendOne(MAX30102_LED_PULSE_AMPLITUD2, 0x24))
		return -1;

	/* PILOT SETTINGS */
	// No found in the documentation
	// 25mA
	if(sendOne(MAX30102_PILOT_AMPLITUD, 0x7F))
		return -1;

	return 0;
}

int MAX30102::readFifo(unsigned *redLED, unsigned *irLED) {
	unsigned char buff[10];
	*redLED = 0;
	*irLED = 0;
	
	if(query(MAX30102_FIFO_DATA_REG, buff, 6))
		return -1;

	/* RED LED */
	mc_tmp = buff[0];
	mc_tmp <<= 16;
	*redLED += mc_tmp;

	mc_tmp = buff[1];
	mc_tmp <<= 8;
	*redLED += mc_tmp;

	mc_tmp = buff[2];
	*redLED += mc_tmp;

	/* IR LED */
	mc_tmp = buff[3];
	mc_tmp <<= 16;
	*irLED += mc_tmp;

	mc_tmp = buff[4];
	mc_tmp <<= 8;
	*irLED += mc_tmp;

	mc_tmp = buff[5];
	*irLED += mc_tmp;

	*redLED &= 0x03FFFF; 
	*irLED &= 0x03FFFF;

	return 0;
}

void MAX30102::shutdown() {
	sendOne(MAX30102_MODE_CONFIG, MAX30102_MODE_SHDN);	// reset
}

void MAX30102::checkNewFifoData() {
	dataReady = 0;

	while(dataReady == 0) {
		queryOne(MAX30102_INT_STATUS1, &status);
		if (status & MAX30102_ALC_OVF) printf("[warning] there's ambient light \n");
		dataReady = status & MAX30102_PPG_RDY;
	}
		
	queryOne(MAX30102_INT_STATUS2, &status); // temperature

}


#ifndef MAX30102_H
#define MAX30102_H 

#include "i2c.h"
#include "kalman.h"

// I2C address device
#define MAX30102_ADDRESS 0xae >> 1

// STATUS REGISTERS
#define MAX30102_INT_STATUS1 	0x00
#define MAX30102_INT_STATUS2	0x01
#define MAX30102_INT_EN1	0x02
#define MAX30102_INT_EN2	0x03

// STATUS FIFO REGISTERS
#define MAX30102_FIFO_WR_PTR 	0x04
#define MAX30102_OVERFLOW_COUNTER 	0x05
#define MAX30102_FIFO_RD_PTR	0x06
#define MAX30102_FIFO_DATA_REG 	0x07

//CONFIGURATION REGISTER
#define MAX30102_FIFO_CONFIG 0x08
#define MAX30102_MODE_CONFIG 0x09
#define MAX30102_SPO2_CONFIG 0x0A
#define MAX30102_LED_PULSE_AMPLITUD1 0x0C
#define MAX30102_LED_PULSE_AMPLITUD2 0x0D
#define MAX30102_PILOT_AMPLITUD 0x10
#define MAX30102_MULTI_LED_MODE_CONTROL_REG_L 0x11
#define MAX30102_MULTI_LED_MODE_CONTROL_REG_H 0x12

// DIE TEMPERATURE REGISTER
#define MAX30102_DIE_TEMP_INTEGER 	0x1F
#define MAX30102_DIE_TEMP_FRACTION	0x20 
#define MAX30102_DIE_TEMP_CONFIG 	0x21 

// PART ID REGISTER
#define MAX30102_REV_ID		0xFE	// contact Maxim Integrated to 
											// get your device's revision 
#define MAX30102_PART_ID 	0xFF

/* BITS ON THE REGISTER */

// STATUS BITS
#define MAX30102_A_FULL		0x80	// new fifo data ready
#define MAX30102_PPG_RDY	0x40	// new fifo data ready
#define MAX30102_ALC_OVF	0x20	// ambient light overflow cancellation

// SAMPLE AVERAGING
#define MAX30102_SMP_AVE_2	0x02 	// average 2 samples
#define MAX30102_SMP_AVE_4	0x40 	// average 4 samples
#define MAX30102_SMP_AVE_8 	0x60	// average 8 samples	
#define MAX30102_SMP_AVE_16 	0x80	// average 16 samples	
#define MAX30102_SMP_AVE_32 	0xA0	// average 32 samples	

// Fifo rollover 
#define MAX30102_FIFO_ROLL 	0x10 	// fifo roll over

// FIfo almos full
#define MAX30102_A_FULL_32 	0x00 	// unread data samples in fifo when
#define MAX30102_A_FULL_31 	0x01 	// an interrupt is issued
#define MAX30102_A_FULL_30 	0x02
#define MAX30102_A_FULL_29 	0x03
#define MAX30102_A_FULL_28 	0x04
#define MAX30102_A_FULL_27 	0x05
#define MAX30102_A_FULL_26 	0x06
#define MAX30102_A_FULL_25 	0x07
#define MAX30102_A_FULL_24 	0x08
#define MAX30102_A_FULL_23 	0x09
#define MAX30102_A_FULL_22 	0x0A
#define MAX30102_A_FULL_21 	0x0B
#define MAX30102_A_FULL_20 	0x0C
#define MAX30102_A_FULL_19 	0x0D
#define MAX30102_A_FULL_18 	0x0E
#define MAX30102_A_FULL_17 	0x0F


// MODE SETTINGS
#define MAX30102_MODE_SHDN 	0x80 	// Shutdown the device, with one
#define MAX30102_RESET		0x40	// reset the device 
#define MAX30102_MODE_CTL_HR 	0x02 	// only Heart Rate (red led only)
#define MAX30102_MODE_CTL_HR_SPO2	0x03 	// heart rate + spo2 (red and ir led)
#define MAX30102_MODE_CTL_MULTI 	0x07	// multi LED enabled

// SPO2 RANGE CONTROL
#define MAX30102_SPO2_ADC_RGE_2048	0x00	// full scale 2048 nA
#define MAX30102_SPO2_ADC_RGE_4096 	0x20 	// full scale 4096 nA
#define MAX30102_SPO2_ADC_RGE_8192 	0x40 	// 8192 nA 	
#define MAX30102_SPO2_ADC_RGE_16384 0x60 	// 16384 nA

// SAMPLE RATE CONTROL
#define MAX30102_SPO2_SR_50HZ	0x00 // sampling at 50Hz
#define MAX30102_SPO2_SR_100HZ 	0x04 // sampling at 100HZ
#define MAX30102_SPO2_SR_200HZ 	0x08 // sampling at 100HZ
#define MAX30102_SPO2_SR_400HZ 	0x0C // sampling at 100HZ
#define MAX30102_SPO2_SR_800HZ 	0x10 // sampling at 100HZ
#define MAX30102_SPO2_SR_1000HZ	0x14 // sampling at 100HZ
#define MAX30102_SPO2_SR_1600HZ 0x18 // sampling at 100HZ
#define MAX30102_SPO2_SR_3200HZ 0x1C // sampling at 100HZ

// LED PULSE WIDTH
#define MAX30102_LED_PW_69	0x00 // pulse width 68.95
#define MAX30102_LED_PW_118	0x01 // pulse width 117.78
#define MAX30102_LED_PW_215	0x02 // pulse width 215.14
#define MAX30102_LED_PW_411	0x03 // pulse width 410.75

class MAX30102 : public I2CDev
{
public:
	
	MAX30102() : 
		I2CDev(MAX30102_ADDRESS) {}

	void reset();
	int settings();
	int readFifo();
	void checkNewFifoData();
	int readFifo(unsigned *redLED, unsigned *irLED);
	void enableKalmanHR();

	unsigned mc_tmp;
	unsigned char status;
	unsigned char dataReady;
	// ~MAX30102();
private:
};
#endif
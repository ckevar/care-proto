#include "i2c.h"

#include <unistd.h>				//Needed for I2C port
#include <fcntl.h>				//Needed for I2C port
#include <sys/ioctl.h>			//Needed for I2C port
#include <linux/i2c-dev.h>		//Needed for I2C port
#include <stdio.h>
#include <string.h>

I2CDev::I2CDev(int add) {
	strcpy(m_devName, "/dev/i2c-1");
	m_address = add;
}

int I2CDev::init() {
	m_fdDev = open(m_devName, O_RDWR);
	
	if (m_fdDev < 0) {
		fprintf(stderr, "[error] could not open the device\n");
		return -1;
	}
	
	if (ioctl(m_fdDev, I2C_SLAVE, m_address) < 0) {
		fprintf(stderr, "[error] failed acquire the bus\n");
		return -1;
	}

	return 0;
}

int I2CDev::send(unsigned char reg, unsigned char *buff, int len) {
	m_buff[0] = reg;
	memcpy(m_buff + 1, buff, len);
	return i2cWrite(buff, len + 1);
}

int I2CDev::sendOne(unsigned char reg, unsigned char data) {
	m_buff[0] = reg;
	m_buff[1] = data;
	return i2cWrite(m_buff, 2);
}

int I2CDev::query(unsigned char reg, unsigned char *buff, int len) {
	if (i2cWrite(&reg, 1)) return -1;
	return i2cRead(buff, len);
}

int I2CDev::queryOne(unsigned char reg, unsigned char *data) {
	return query(reg, data, 1);	
}

int I2CDev::setAddress(int add) {
	m_address = add;
	if (ioctl(m_fdDev, I2C_SLAVE, m_address) < 0) {
		fprintf(stderr, "[error] failed acquire the bus\n");
		fprintf(stderr, "[error] failed acquire the bus\n");
		return -1;
	}	
	return 0;
}

int I2CDev::i2cWrite(unsigned char *buff, int len) {
	if(write(m_fdDev, buff, len) != len) {
		// fprintf(stderr, "[error] failed to write\n");
		printf("[error] failed to write\n");
		return -1;
	} 
	return 0;
}

int I2CDev::i2cRead(unsigned char *buff, int len) {
	if (read(m_fdDev, buff, len) != len) {
		// fprintf(stderr, "[error] failed to read\n");
		printf( "[error] failed to read\n");
		return -1;
	}
	return 0;
}

void I2CDev::terminate() {
	close(m_fdDev);
}
#ifndef I2C_H
#define I2C_H 

class I2CDev {
public:
	I2CDev(int add);

	int init();
	int send(unsigned char reg, unsigned char *buff, int len);
	int sendOne(unsigned char reg, unsigned char data);
	int query(unsigned char reg, unsigned char *buff, int len);
	int queryOne(unsigned char reg, unsigned char *data);

	int setAddress(int add);
	void terminate();
	// ~I2CDev();
private:
	char m_devName[10];	// file name of the i2c on the raspberry
	int m_address;					// address to interact with (external hardware)
	int m_fdDev;						// file descriptor of the i2c on the raspberry
	unsigned char m_buff[10];
	int i2cWrite(unsigned char *buff, int len);
	int i2cRead(unsigned char *buff, int len);
};

#endif
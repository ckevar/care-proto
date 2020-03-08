#ifndef SPI_H
#define SPI_H

#include <linux/spi/spidev.h>              //Needed for SPI port
#include <stddef.h>

class SPIDev
{
public:
	SPIDev();
	int openSPI(int cs, unsigned char mode, unsigned speed);
	int writeSPI(unsigned char *buff, size_t count);
	int closeSPI();
	// ~spi_device();
private:
	int fd;
	struct spi_ioc_transfer spi;
};

#endif


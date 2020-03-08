#include "spi.h"


#include <unistd.h>                             //Needed for SPI port
#include <fcntl.h>                              //Needed for SPI port
#include <sys/ioctl.h>                  //Needed for SPI port
#include <cstdio>
#include <cstring>

SPIDev::SPIDev() {	
	memset(&spi, 0, sizeof(spi));
	
	spi.tx_buf = (unsigned) NULL;
	spi.rx_buf = (unsigned) NULL;
	spi.delay_usecs = 0;
	spi.cs_change = 0;
	spi.bits_per_word = 8;	// 16 is not accepted neither 12
}

int SPIDev::openSPI(int spi_module, unsigned char spi_mode, unsigned spi_speed) {
	if (spi_module)
		fd = open("/dev/spidev0.1", O_RDWR);
	else
		fd = open("/dev/spidev0.0", O_RDWR);

	if (fd < 0) {
		fprintf(stderr, "[error] could not open SPI dev\n");
		return -1;
	}

	if (ioctl(fd, SPI_IOC_WR_MODE, &spi_mode) < 0) {
		close(fd);
		fprintf(stderr, "[error] could not set SPI mode (WR)\n");
		return -1;
	}

	if(ioctl(fd, SPI_IOC_RD_MODE, &spi_mode) < 0) {
		close(fd);
		fprintf(stderr, "[error] could not set SPI mode (RD)\n");
		return -1;
	}

	if (ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spi.bits_per_word) < 0) {
		close(fd);
		fprintf(stderr, "[error] could not set SPI bitsPerWord (WR)\n");
		return -1;
	}

	if (ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &spi.bits_per_word) < 0) {
		close(fd);
		fprintf(stderr, "[error] could not set SPI bitsPerWord (RD)\n");
		return -1;
	}

	if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed) < 0) {
		close(fd);
		fprintf(stderr, "[error] could not set SPI speed (WR)\n");
		return -1;
	}

	if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed) < 0) {
		close(fd);
		fprintf(stderr, "[error] could not set SPI speed (RD)\n");
		return -1;
	}
	spi.speed_hz = spi_speed;
	return fd;
}

int SPIDev::writeSPI(unsigned char *buff, size_t count) {
	spi.tx_buf = (unsigned) buff;
	spi.len = count; 
	return ioctl(fd, SPI_IOC_MESSAGE(1), &spi);
}

int SPIDev::closeSPI() {
	return close(fd);
}
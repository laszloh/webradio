/*
 * spi.h
 *
 * Created: 25.03.2017 21:40:04
 *  Author: Simon
 */ 


#ifndef SPI_H_
#define SPI_H_

void spi_init(void);

void spi_byte_tx(uint8_t data);

void spi_buf_tx(uint8_t *buf, size_t count);

#endif /* SPI_H_ */
#include <Arduino.h>

#ifndef __LIN_BUS_H__
#define __LIN_BUS_H__

#define STATUS_OK 1
#define STATUS_ERROR -1
#define STATUS_NONE 0

#define BAUD_19200  19200

#define BIT(data, shift) ((addr & (1 << shift)) >> shift)
uint8_t addrParity(uint8_t addr)
{
	uint8_t p0 = BIT(addr, 0) ^ BIT(addr, 1) ^ BIT(addr, 2) ^ BIT(addr, 4);
	uint8_t p1 = ~(BIT(addr, 1) ^ BIT(addr, 3) ^ BIT(addr, 4) ^ BIT(addr, 5));
	return (p0 | (p1 << 1)) << 6;
}

uint8_t dataChecksum(uint8_t id, const uint8_t *message, uint8_t nBytes)
{
  uint16_t sum = id | addrParity(id);
	while (nBytes-- > 0)
		sum += *(message++);
	// Add the carry
	while (sum >> 8) // In case adding the carry causes another carry
		sum = (sum & 255) + (sum >> 8);
	return (~sum);
}
#endif //__LIN_BUS_H__

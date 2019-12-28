#pragma once

#include <avr/io.h>

extern const uint8_t IO_PCMSK;

static inline void io_init()
{
	PORTB   = 0xF8;
	DDRB    = 0x07;
}

// bit 0: gd
// bit 1: gc
static inline uint8_t io_get_simp2c()
{
	uint8_t rs(0);

	// gd
	if (PINB & _BV(4))
		rs |= _BV(0);

	// gc
	if (PINB & _BV(3))
		rs |= _BV(1);

	return rs;
}

// bit 0: green
// bit 1: red
// bit 2: blue
// other bits must be 0
static inline void io_set_drv(uint8_t drv)
{
	PORTB = drv | 0xF8;
}

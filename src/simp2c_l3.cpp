#include <avr/interrupt.h>
#include <avr/io.h>
#include "simp2c_l2.h"
#include "simp2c_l3.h"
#include "timer.h"
#include "io.h"

struct SCtx
{
	CSimp2c simp2c;
	uint8_t data[3];
	uint8_t bitcnt;
	uint8_t bytecnt;
};

static SCtx g_ctx;

ISR(PCINT0_vect)
{
	switch (g_ctx.simp2c(io_get_simp2c()))
	{
		case CSimp2c::EVT_SYNC:
			g_ctx.bitcnt = 0;
			g_ctx.bytecnt = 0;
			g_ctx.data[0] = g_ctx.data[1] = g_ctx.data[2] = 0;
			break;

		case CSimp2c::EVT_BIT_1:
			if (g_ctx.bytecnt <= 2)
				g_ctx.data[g_ctx.bytecnt] |= 1 << g_ctx.bitcnt;

			/* FALLTHROUGH */
		case CSimp2c::EVT_BIT_0:
			if (++g_ctx.bitcnt == 8)
			{
				g_ctx.bitcnt = 0;
				if (g_ctx.bytecnt <= 2)
					if (++g_ctx.bytecnt == 3)
						timer_update(g_ctx.data);
			}
			break;

		case CSimp2c::EVT_ERROR:
			g_ctx.bytecnt = 0xFF;
			g_ctx.data[2] = 0xFF;
			break;

		default:
			break;
	}
}

void simp2c_l3_init()
{
	PCMSK	= IO_PCMSK;
	GIMSK	|= _BV(PCIE);
}

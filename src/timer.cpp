#include <util/atomic.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/io.h>
#include <string.h>
#include "io.h"

enum EMode
{
	MODE_RR,	// 0 red rising (goto 1 or 6)
	MODE_RF_BR,	// 1 red falling && blue rising (goto 2 or 4)
	MODE_BF_GR,	// 2 blue falling && green rising (goto 3 or 5)
	MODE_GF_RR,	// 3 green falling && red rising (goto 1 or 6)
	MODE_BF,	// 4 blue falling (goto 7)
	MODE_GF,	// 5 green falling (goto 7)
	MODE_RF,	// 6 red falling (goto 7)
	MODE_PI,	// 7 display from raspberry (stays at 7)
};

struct SIsrCtx
{
	// current rgb values (0...255 each)
	uint8_t cur_rgb[3];

	// phase of pwm (0...255)
	uint8_t pwm_phase;

	// rgb values to update on next pwm
	volatile uint8_t next_rgb[3];

	// true if pwm overflowed and can set next_rgb (for idle loop)
	volatile bool next_cycle;
};

struct SIdleCtx
{
	// current mode (EMode)
	uint8_t mode;

	// phase of current mode (0...255)
	uint8_t mode_phase;

	// rgb values to update (from pi) in next idle loop
	uint8_t pi_rgb[3];

	// true if received at least one rgb from pi
	bool pi;
};

static SIsrCtx g_isr_ctx;
static SIdleCtx g_idle_ctx;

ISR(TIM0_COMPA_vect)
{
	// for 8-digit resolution (explained):
	// phase 01234567
	// cur=0 00000000
	// cur=1 10000000
	// cur=2 11000000
	// cur=3 11100000
	// cur=4 11110000
	// cur=5 11111000
	// cur=6 11111100
	// cur=7 11111110
	//
	// state is 1 when phase < cur

	uint8_t state(0);
	uint8_t phase(g_isr_ctx.pwm_phase);

	if (phase < g_isr_ctx.cur_rgb[0])
		state |= _BV(1);

	if (phase < g_isr_ctx.cur_rgb[1])
		state |= _BV(0);

	if (phase < g_isr_ctx.cur_rgb[2])
		state |= _BV(2);

	io_set_drv(state);

	g_isr_ctx.pwm_phase = ++phase;
	if (!phase)
	{
		g_isr_ctx.cur_rgb[0] = g_isr_ctx.next_rgb[0];
		g_isr_ctx.cur_rgb[1] = g_isr_ctx.next_rgb[1];
		g_isr_ctx.cur_rgb[2] = g_isr_ctx.next_rgb[2];
		g_isr_ctx.next_cycle = true;
	}
}

void timer_init()
{
	// CTC, 9600000 / 128 / 256 = 292 Hz LED driving frequency
	TCCR0A	= _BV(WGM01);
	TCCR0B	= _BV(CS00);
	OCR0A	= 0x80;
	TIMSK0	= _BV(OCIE0A);
}

void timer_update(const uint8_t rgb[3])
{
	memcpy(g_idle_ctx.pi_rgb, rgb, sizeof(g_idle_ctx.pi_rgb));
	g_idle_ctx.pi = true;
}

static inline bool update_next_from_mode()
{
	uint8_t rgb[3];

	if (g_idle_ctx.mode == MODE_PI)
	{
		// special case, copied from pi_rgb
		memcpy(rgb, g_idle_ctx.pi_rgb, sizeof(rgb));
	}
	else
	{
		// indexes are split into 4-bit nibbles:
		// - high nibble: falling color index
		// - low nibble: rising color index
		// if nibble is 0xF, then it's not used

		static const uint8_t idx_map[] PROGMEM =
		{
			0xF0,	// 0 red rising
			0x02,	// 1 red falling && blue rising
			0x21,	// 2 blue falling && green rising
			0x10,	// 3 green falling && red rising
			0x2F,	// 4 blue falling
			0x1F,	// 5 green falling
			0x0F,	// 6 red falling
		};

		const uint8_t idx(pgm_read_byte_near(idx_map + g_idle_ctx.mode));
		const uint8_t falling_idx((idx & 0xF0) >> 4);
		const uint8_t rising_idx(idx & 0x0F);

		memset(rgb, 0, sizeof(rgb));

		if (falling_idx != 0x0F)
			rgb[falling_idx] = g_idle_ctx.mode_phase ^ 0xFF;

		if (rising_idx != 0x0F)
			rgb[rising_idx] = g_idle_ctx.mode_phase;
	}

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		g_isr_ctx.next_rgb[0] = rgb[0];
		g_isr_ctx.next_rgb[1] = rgb[1];
		g_isr_ctx.next_rgb[2] = rgb[2];
	}

	return !++g_idle_ctx.mode_phase;
}

static inline void next_mode()
{
	static const uint8_t modes_no_pi[] PROGMEM =
	{
		MODE_RF_BR,	// red rising -> red falling && blue rising
		MODE_BF_GR,	// red falling && blue rising -> blue falling && green rising
		MODE_GF_RR,	// blue falling && green rising -> green falling && red rising
		MODE_RF_BR,	// green falling && red rising -> red falling && blue rising
		// other modes not specified as they won't be triggered with no signal from pi
	};

	static const uint8_t modes_pi[] PROGMEM =
	{
		MODE_RF,	// red rising -> red falling
		MODE_BF,	// red falling && blue rising -> blue falling
		MODE_GF,	// blue falling && green rising -> green falling
		MODE_RF,	// green falling && red rising -> red falling
		MODE_PI,	// blue falling -> pi
		MODE_PI,	// green falling -> pi
		MODE_PI,	// red falling -> pi
		MODE_PI,	// pi -> pi
	};

	g_idle_ctx.mode = pgm_read_byte_near((g_idle_ctx.pi ? modes_pi : modes_no_pi) + g_idle_ctx.mode);
}

void timer_idle()
{
	if (!g_isr_ctx.next_cycle)
		return;

	g_isr_ctx.next_cycle = false;

	if (update_next_from_mode())
		next_mode();
}

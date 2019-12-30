#include <avr/sleep.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include "timer.h"
#include "simp2c_l3.h"
#include "io.h"

int main()
{
	io_init();
	timer_init();
	simp2c_l3_init();

	sei();

	wdt_enable(WDTO_15MS);
	set_sleep_mode(SLEEP_MODE_IDLE);
	for (;;)
	{
		wdt_reset();
		sleep_mode();
		timer_idle();
	}

	/* NOTREACHED */
	return 0;
}

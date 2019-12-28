#pragma once

void timer_init();
void timer_idle();

// to be called with interrupts disabled
void timer_update(const uint8_t rgb[3]);

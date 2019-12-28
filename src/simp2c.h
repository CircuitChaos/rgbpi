#pragma once

#include <inttypes.h>

// bit 0: gd
// bit 1: gc

class CSimp2c
{
public:
	CSimp2c();

	enum EEvt
	{
		EVT_NONE,
		EVT_SYNC,
		EVT_BIT_1,
		EVT_BIT_0,
		EVT_ERROR,
	};

	EEvt operator()(uint8_t value);

private:
	enum EBitEvent
	{
		BE_L,
		BE_H,
		BE_R,
		BE_F,
	};

	uint8_t m_last;
	bool m_sync;

	static EBitEvent getBitEvent(bool last, bool cur);
};

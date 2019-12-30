#include <avr/io.h>
#include "simp2c_l2.h"

CSimp2c::CSimp2c(): m_last(0x03), m_sync(false)
{
}

CSimp2c::EEvt CSimp2c::operator()(uint8_t value)
{
	const EBitEvent c(getBitEvent(m_last & _BV(1), value & _BV(1)));
	const EBitEvent d(getBitEvent(m_last & _BV(0), value & _BV(0)));
	m_last = value;

	switch (c)
	{
		case BE_H:
			if (d == BE_F)
			{
				m_sync = true;
				return EVT_SYNC;
			}

			break;

		case BE_F:
			if (!m_sync)
				break;

			if (d == BE_R || d == BE_F)
			{
				m_sync = false;
				return EVT_ERROR;
			}

			break;

		case BE_R:
			if (!m_sync)
				break;

			if (d == BE_H)
				return EVT_BIT_1;

			if (d == BE_L)
				return EVT_BIT_0;

			m_sync = false;
			return EVT_ERROR;

		default:
			break;
	}

	return EVT_NONE;
}

CSimp2c::EBitEvent CSimp2c::getBitEvent(bool last, bool cur)
{
	if (!last && !cur)
		return BE_L;

	if (last && cur)
		return BE_H;

	if (!last && cur)
		return BE_R;

	return BE_F;
}

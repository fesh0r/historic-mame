#include "driver.h"
#include "sndhrdw/8910intf.h"



static struct AY8910interface interface =
{
	1,	/* 1 chip */
	1536000,	/* 1.536000000 MHZ ? */
	{ 0x30ff },
	{ input_port_0_r },
	{ input_port_1_r },
	{ 0 },
	{ 0 }
};



int champbas_sh_start(void)
{
	return AY8910_sh_start(&interface);
}

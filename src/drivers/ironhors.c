/***************************************************************************

IronHorse memory map (preliminary)

0000-00ff  Work area
2000-23ff  ColorRAM
2400-27ff  VideoRAM
2800-2fff  RAM
3000-30ff  First sprite bank?
3800-38ff  Second sprite bank?
3f00-3fff  Stack AREA
4000-ffff  ROM

Read:
0b01:	Player 2 controls
0b02:	Player 1 controls
0b03:	Coin + selftest
0a00:	DSW1
0b00:	DSW2
0900:	DSW3

Write:
0003:       Bit 1 selects the 1024 char bank, bit 3 the sprite RAM bank, other bits unknown
0004:       Bit 0 controls NMI, bit 3 controls FIRQ, other bits unknown
0020-003f:  Scroll registers

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "sndhrdw/generic.h"
#include "sndhrdw/2203intf.h"
#include "M6809.h"


extern unsigned char *ironhors_charbank;
extern unsigned char *ironhors_scroll;
static unsigned char *ironhors_interrupt_enable;

int ironhors_sh_start(void);
int ironhors_sh_timer_r(int offset);
void ironhors_sh_irqtrigger_w(int offset,int data);

void ironhors_vh_convert_color_prom(unsigned char *palette, unsigned char *colortable,const unsigned char *color_prom);
void ironhors_vh_screenrefresh(struct osd_bitmap *bitmap);


int ironhors_interrupt(void)
{
	if (cpu_getiloops() == 0)
	{
		if (*ironhors_interrupt_enable & 4) return M6809_INT_FIRQ;
	}
	else if (cpu_getiloops() % 2)
	{
		if (*ironhors_interrupt_enable & 1) return nmi_interrupt();
	}
	return ignore_interrupt();
}

static struct MemoryReadAddress ironhors_readmem[] =
{
//	{ 0x0000, 0x00ff, MRA_RAM },	/* ????? */
	{ 0x0020, 0x003f, MRA_RAM },
	{ 0x0b03, 0x0b03, input_port_0_r },	/* coins + selftest */
	{ 0x0b02, 0x0b02, input_port_1_r },	/* player 1 controls */
	{ 0x0b01, 0x0b01, input_port_2_r },	/* player 2 controls */
	{ 0x0a00, 0x0a00, input_port_3_r },	/* Dipswitch settings 0 */
	{ 0x0b00, 0x0b00, input_port_4_r },	/* Dipswitch settings 1 */
	{ 0x0900, 0x0900, input_port_5_r },	/* Dipswitch settings 2 */
	{ 0x2000, 0x2fff, MRA_RAM },
	{ 0x3000, 0x3fff, MRA_RAM },
	{ 0x4000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress ironhors_writemem[] =
{
	{ 0x0003, 0x0003, MWA_RAM, &ironhors_charbank },
	{ 0x0004, 0x0004, MWA_RAM, &ironhors_interrupt_enable },
	{ 0x0020, 0x003f, MWA_RAM, &ironhors_scroll },
//	{ 0x0000, 0x00ff, MWA_RAM },
	{ 0x0800, 0x0800, soundlatch_w },
	{ 0x0900, 0x0900, ironhors_sh_irqtrigger_w },  /* cause interrupt on audio CPU */
	{ 0x0a00, 0x0a00, MWA_NOP },	/* ???? */
	{ 0x0b00, 0x0b00, watchdog_reset_w },
	{ 0x2000, 0x23ff, colorram_w, &colorram },
	{ 0x2400, 0x27ff, videoram_w, &videoram, &videoram_size },
	{ 0x2800, 0x2fff, MWA_RAM },
	{ 0x3000, 0x30ff, MWA_RAM, &spriteram_2 },
	{ 0x3100, 0x37ff, MWA_RAM },
	{ 0x3800, 0x38ff, MWA_RAM, &spriteram, &spriteram_size },
	{ 0x3900, 0x3fff, MWA_RAM },
	{ 0x4000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};


static struct MemoryReadAddress ironhors_sound_readmem[] =
{
	{ 0x0000, 0x3fff, MRA_ROM },
	{ 0x4000, 0x43ff, MRA_RAM },
	{ 0x8000, 0x8000, soundlatch_r },
	{ -1 }	/* end of table */
};
static struct MemoryWriteAddress ironhors_sound_writemem[] =
{
	{ 0x0000, 0x3fff, MWA_ROM },
	{ 0x4000, 0x43ff, MWA_RAM },
	{ -1 }	/* end of table */
};
static struct IOReadPort ironhors_sound_readport[] =
{
	{ 0x00, 0x00, ironhors_sh_timer_r },
	{ -1 }	/* end of table */
};
static struct IOWritePort ironhors_sound_writeport[] =
{
	{ 0x00, 0x00, YM2203_control_port_0_w },
	{ 0x01, 0x01, YM2203_write_port_0_w },
	{ -1 }	/* end of table */
};

INPUT_PORTS_START( ironhors_input_ports )
	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_4WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_4WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_4WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )


	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x04, 0x00, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Upright" )
	PORT_DIPSETTING(    0x04, "Cocktail" )
	PORT_DIPNAME( 0x18, 0x18, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x18, "30000 70000" )
	PORT_DIPSETTING(    0x10, "40000 80000" )
	PORT_DIPSETTING(    0x08, "40000" )
	PORT_DIPSETTING(    0x00, "50000" )
	PORT_DIPNAME( 0x60, 0x60, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x60, "Easy" )
	PORT_DIPSETTING(    0x40, "Normal" )
	PORT_DIPSETTING(    0x20, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x80, 0x80, "Attract Sound", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x0f, 0x0f, "Coin A", IP_KEY_NONE )
	PORT_DIPSETTING(    0x02, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x05, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x08, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x04, "3 Coins/2 Credits" )
	PORT_DIPSETTING(    0x01, "4 Coins/3 Credits" )
	PORT_DIPSETTING(    0x0f, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x03, "3 Coins/4 Credits" )
	PORT_DIPSETTING(    0x07, "2 Coins/3 Credits" )
	PORT_DIPSETTING(    0x0e, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x06, "2 Coins/5 Credits" )
	PORT_DIPSETTING(    0x0d, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0x0c, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0x0b, "1 Coin/5 Credits" )
	PORT_DIPSETTING(    0x0a, "1 Coin/6 Credits" )
	PORT_DIPSETTING(    0x09, "1 Coin/7 Credits" )
	PORT_DIPSETTING(    0x00, "Free Play" )
	PORT_DIPNAME( 0xf0, 0xf0, "Coin B", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "4 Coins/1 Credit" )
	PORT_DIPSETTING(    0x50, "3 Coins/1 Credit" )
	PORT_DIPSETTING(    0x80, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x40, "3 Coins/2 Credits" )
	PORT_DIPSETTING(    0x10, "4 Coins/3 Credits" )
	PORT_DIPSETTING(    0xf0, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x30, "3 Coins/4 Credits" )
	PORT_DIPSETTING(    0x70, "2 Coins/3 Credits" )
	PORT_DIPSETTING(    0xe0, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x60, "2 Coins/5 Credits" )
	PORT_DIPSETTING(    0xd0, "1 Coin/3 Credits" )
	PORT_DIPSETTING(    0xc0, "1 Coin/4 Credits" )
	PORT_DIPSETTING(    0xb0, "1 Coin/5 Credits" )
	PORT_DIPSETTING(    0xa0, "1 Coin/6 Credits" )
	PORT_DIPSETTING(    0x90, "1 Coin/7 Credits" )
/* 	PORT_DIPSETTING(    0x00, "Invalid" ) */

	PORT_START
	PORT_DIPNAME( 0x01, 0x00, "Flip Screen", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x01, "On" )
	PORT_DIPNAME( 0x02, 0x00, "Controls", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Single" )
	PORT_DIPSETTING(    0x02, "Dual" )
	PORT_BIT( 0xfc, IP_ACTIVE_HIGH, IPT_UNKNOWN )
INPUT_PORTS_END


static struct GfxLayout ironhors_charlayout =
{
	8,8,	/* 8*8 characters */
	2048,	/* 512 characters */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },	/* the four bitplanes are packed in one nibble */
	{ 0*4, 1*4, 0x8000*8+0*4, 0x8000*8+1*4, 2*4, 3*4, 0x8000*8+2*4, 0x8000*8+3*4 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8	/* every char takes 8 consecutive bytes */
};

static struct GfxLayout ironhors_spritelayout =
{
	16,16,	/* 16*16 sprites */
	512,	/* 512 sprites */
	4,	/* 4 bits per pixel */
	{ 0, 1, 2, 3 },	/* the four bitplanes are packed in one nibble */
	{ 0*4, 1*4, 0x8000*8+0*4, 0x8000*8+1*4, 2*4, 3*4, 0x8000*8+2*4, 0x8000*8+3*4,
           16*8+0*4, 16*8+1*4, 16*8+0x8000*8+0*4, 16*8+0x8000*8+1*4, 16*8+2*4, 16*8+3*4, 16*8+0x8000*8+2*4, 16*8+0x8000*8+3*4 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
           16*16, 17*16, 18*16, 19*16, 20*16, 21*16, 22*16, 23*16 },
	64*8	/* every sprite takes 8 consecutive bytes */
};

static struct GfxDecodeInfo ironhors_gfxdecodeinfo[] =
{
	{ 1, 0x00000, &ironhors_charlayout,       0, 16 },
	{ 1, 0x10000, &ironhors_spritelayout,     0, 16 },
	{ 1, 0x10000, &ironhors_charlayout,       0, 16 },  /* to handle 8x8 sprites */
	{ -1 } /* end of array */
};


/* This color prom comes from Yie ar Kung fu , mikie must have a similar one */
static unsigned char color_prom[] =
{
	/* 1L - palette red component */
	0x00,0x0E,0x0E,0x01,0x0E,0x00,0x0E,0x0B,0x0E,0xFE,0xF4,0xF0,0xFF,0xFA,0x0D,0x05,
	0x00,0x0E,0x0A,0x0C,0x09,0x0D,0x0E,0x0C,0x0A,0x00,0x0A,0x00,0x05,0x00,0x0D,0x0F,
	0x00,0x0E,0x0D,0x01,0x0E,0x00,0x0E,0xCB,0xFE,0xFE,0x84,0x00,0x0F,0x0A,0x0D,0x05,
	0x70,0x0E,0x0C,0x78,0x70,0x20,0x9D,0x1B,0xF4,0xC0,0x7F,0x00,0x03,0x25,0x78,0x7F,
	0x20,0x2E,0x95,0x70,0x7E,0x20,0x9E,0x3B,0x1E,0xFE,0xC4,0xC0,0x5F,0x5A,0x5D,0xB5,
	0x50,0xDE,0x5B,0xED,0x5D,0x4E,0x5E,0x88,0x5A,0xE0,0x28,0x00,0x9C,0x38,0x44,0x2F,
	0x00,0x0E,0x2E,0x71,0x1E,0x00,0xEE,0xEB,0xBE,0x2E,0x04,0xA0,0x7F,0x7A,0x2D,0x25,
	0xF0,0x2E,0x2F,0x05,0x2A,0xFE,0xCF,0x3B,0x5E,0x00,0x2A,0xE0,0xC9,0x37,0x5B,0x06,
	0x00,0x1E,0xCE,0x31,0x5E,0x00,0x0E,0x0B,0xCE,0x3E,0x54,0x90,0x3F,0x0A,0x9D,0x35,
	0x00,0xAE,0xE5,0x57,0x2C,0x5E,0x2E,0xEB,0x27,0x90,0x5C,0xE0,0xEA,0xE6,0x25,0x0F,
	0x90,0xCE,0x55,0x2D,0xFE,0x10,0x0E,0x0B,0x1E,0x7E,0xB4,0x20,0xFF,0xCA,0x5D,0x25,
	0xF0,0x1E,0xAE,0x2D,0x0E,0xCE,0x3F,0x8D,0x7B,0xE0,0x0A,0xE0,0x0B,0x2D,0x09,0xCF,
	0x50,0x2E,0x0E,0x39,0xFE,0x10,0x0E,0xCB,0x4E,0x2E,0x04,0x30,0x0F,0x2A,0xCD,0x65,
	0x20,0xEE,0xCF,0x28,0x0F,0xCF,0x3F,0x1B,0x36,0x00,0xAF,0x00,0x03,0x38,0xF6,0x20,
	0x10,0xFE,0x3E,0xFE,0xD0,0x50,0xC0,0x3B,0xFA,0x00,0x40,0xC0,0x1F,0x9F,0xCF,0x2F,
	0xD0,0xCE,0x0E,0x0E,0x10,0x10,0x70,0x2B,0xCA,0x60,0x20,0xF0,0x0F,0x9F,0x9F,0x5F,
	/* 3L - palette green component */
	0x00,0x08,0x09,0x0A,0x00,0x08,0x0E,0x0B,0x0E,0xF0,0xF3,0xF6,0xFD,0xF2,0x08,0x05,
	0x00,0x0E,0x0A,0x0C,0x05,0x0B,0x0C,0x0C,0x02,0x04,0x07,0x0A,0x05,0x08,0x08,0x00,
	0x00,0x08,0x0D,0x0A,0x00,0x08,0x0E,0xCB,0xFE,0xF0,0x83,0x06,0x0D,0x02,0x08,0x05,
	0x70,0x0E,0x04,0x78,0x77,0x2B,0x9D,0x1B,0xF4,0xC4,0x77,0x08,0x0A,0x26,0x79,0x70,
	0x20,0x28,0x9B,0x78,0x70,0x28,0x9E,0x3B,0x1E,0xF0,0xC3,0xC6,0x5D,0x52,0x58,0xB5,
	0x50,0xDE,0x5B,0xE6,0x5F,0x48,0x59,0x88,0x56,0xE4,0x24,0x08,0x9A,0x3C,0x44,0x20,
	0x00,0x08,0x29,0x7A,0x10,0x08,0xEE,0xEB,0xBE,0x20,0x03,0xA6,0x7D,0x72,0x28,0x25,
	0xF0,0x2E,0x29,0x0A,0x25,0xFF,0xCC,0x33,0x52,0x04,0x2E,0xE8,0xC6,0x3C,0x5B,0x06,
	0x00,0x18,0xC9,0x3A,0x50,0x08,0x0E,0x0B,0xCE,0x30,0x53,0x96,0x3D,0x02,0x98,0x35,
	0x00,0xAE,0xEA,0x59,0x28,0x5A,0x2B,0xEB,0x27,0x94,0x55,0xEA,0xEA,0xE6,0x25,0x00,
	0x90,0xC8,0x5B,0x2C,0xF0,0x18,0x0E,0x0B,0x1E,0x70,0xB3,0x26,0xFD,0xC2,0x58,0x25,
	0xF0,0x1E,0xA6,0x2C,0x0B,0xC8,0x39,0x8A,0x76,0xE4,0x02,0xEA,0x02,0x23,0x06,0xC0,
	0x50,0x28,0x09,0x38,0xF0,0x18,0x0E,0xCB,0x4E,0x20,0x03,0x36,0x0D,0x22,0xC8,0x60,
	0x20,0xEE,0xCC,0x28,0x00,0xC9,0x3F,0x1B,0x36,0x04,0xAC,0x08,0x0A,0x3D,0xF9,0x27,
	0x10,0xFE,0x3F,0xFF,0xDF,0x5A,0xCC,0x3B,0xF0,0x08,0x40,0xCA,0x17,0x95,0xC3,0x20,
	0xD0,0xCE,0x0F,0x0F,0x1F,0x1A,0x7C,0x2B,0xC0,0x60,0x29,0xFA,0x07,0x95,0x93,0x50,
	/* 2L - palette blue component */
	0x00,0x06,0x04,0x08,0x0B,0x0C,0x0E,0x0B,0x00,0xF2,0xF0,0xF9,0xFA,0xF2,0x03,0x05,
	0x00,0x0E,0x0A,0x0C,0x03,0x05,0x06,0x00,0x02,0x0B,0x05,0x0C,0x09,0x05,0x03,0x00,
	0x00,0x06,0x08,0x08,0x0B,0x0C,0x0E,0xCB,0xF0,0xF2,0x80,0x09,0x0A,0x02,0x03,0x05,
	0x70,0x0E,0x04,0x78,0x70,0x2F,0x98,0x1B,0xF4,0xCB,0x70,0x0F,0x09,0x27,0x7A,0x70,
	0x20,0x26,0x98,0x76,0x78,0x2C,0x9E,0x3B,0x10,0xF2,0xC0,0xC9,0x5A,0x52,0x53,0xB5,
	0x50,0xDE,0x5B,0xE6,0x5C,0x49,0x5A,0x88,0x54,0xEB,0x23,0x0F,0x96,0x3D,0x44,0x20,
	0x00,0x06,0x24,0x78,0x1B,0x0C,0xEE,0xEB,0xB0,0x22,0x00,0xA9,0x7A,0x72,0x23,0x25,
	0xF0,0x2E,0x28,0x07,0x27,0xF7,0xC6,0x33,0x52,0x0B,0x20,0xEF,0xC8,0x3E,0x5B,0x06,
	0x00,0x16,0xC4,0x38,0x5B,0x0C,0x0E,0x0B,0xC0,0x32,0x50,0x99,0x3A,0x02,0x93,0x35,
	0x00,0xAE,0xEC,0x5F,0x25,0x57,0x28,0xED,0x27,0x9B,0x53,0xEF,0xE9,0xEB,0x28,0x00,
	0x90,0xC6,0x58,0x29,0xF8,0x1C,0x0E,0x0B,0x10,0x72,0xB0,0x29,0xFA,0xC2,0x53,0x25,
	0xF0,0x1E,0xA7,0x29,0x08,0xC5,0x36,0x85,0x74,0xEB,0x02,0xEF,0x04,0x21,0x03,0xC0,
	0x50,0x26,0x04,0x3A,0xFB,0x1C,0x0E,0xCB,0x40,0x22,0x00,0x39,0x0A,0x22,0xC3,0x65,
	0x20,0xEE,0xC8,0x28,0x00,0xC8,0x39,0x1B,0x36,0x0B,0xA0,0x0F,0x09,0x3F,0xF5,0x20,
	0x10,0xFE,0x39,0xF0,0xD0,0x54,0xC9,0x3B,0xFB,0x0F,0x4F,0xCF,0x19,0x97,0xC5,0x20,
	0xD0,0xCE,0x09,0x00,0x10,0x14,0x79,0x2B,0xCB,0x6F,0x29,0xFF,0x09,0x97,0x95,0x50
};


static struct MachineDriver ironhors_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_M6809,
			2500000,        /* 2.50 Mhz? */
			0,
			ironhors_readmem,ironhors_writemem,0,0,
			ironhors_interrupt,8
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			14318180/4,	/* ??  */
			2,	/* memory region #2 */
			ironhors_sound_readmem,ironhors_sound_writemem,ironhors_sound_readport,ironhors_sound_writeport,
			ignore_interrupt,1	/* interrupts are triggered by the main CPU */
		}
	},
	30,
	1,
	0,      /* ironhors_init_machine */

	/* video hardware */
	32*8, 32*8, { 1*8, 31*8-1, 2*8, 30*8-1 },
	ironhors_gfxdecodeinfo,
	256,16*16+16*16,
	ironhors_vh_convert_color_prom,

	VIDEO_TYPE_RASTER,
	0,
	generic_vh_start,
	generic_vh_stop,
	ironhors_vh_screenrefresh,

	/* sound hardware */
	0,
	ironhors_sh_start,
	YM2203_sh_stop,
	YM2203_sh_update
};


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ironhors_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "13c_h03.bin", 0x4000, 0x8000, 0xda405692 )
	ROM_LOAD( "12c_h02.bin", 0xc000, 0x4000, 0x08faa46e )

	ROM_REGION(0x20000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "09f_h07.bin", 0x00000, 0x8000, 0xc6b59885 )
	ROM_LOAD( "06f_h04.bin", 0x08000, 0x8000, 0xb57060e6 )
	ROM_LOAD( "08f_h06.bin", 0x10000, 0x8000, 0xe182a082 )
	ROM_LOAD( "07f_h05.bin", 0x18000, 0x8000, 0xf688cb60 )

	ROM_REGION(0x10000)     /* 64k for audio cpu */
	ROM_LOAD( "10c_h01.bin", 0x0000, 0x4000, 0x3ba31251 )
ROM_END


struct GameDriver ironhors_driver =
{
	"Iron Horse",
	"ironhors",
	"Mirko Buffoni",
	&ironhors_machine_driver,

	ironhors_rom,
	0, 0,
	0,
        0,

	0/*TBR*/, ironhors_input_ports, 0/*TBR*/, 0/*TBR*/, 0/*TBR*/,

	color_prom, 0, 0,
        ORIENTATION_DEFAULT,

	0, 0
};

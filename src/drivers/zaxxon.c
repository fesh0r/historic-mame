/***************************************************************************

Zaxxon memory map (preliminary)

0000-1fff ROM 3
2000-3fff ROM 2
4000-4fff ROM 1
6000-67ff RAM 1
6800-6fff RAM 2
8000-83ff Video RAM
a000-a0ff sprites

read:
c000      IN0
c001      IN1
c002      DSW0
c003      DSW1
c100      IN2
see the input_ports definition below for details on the input bits

write:
c000-c002 ?
c006      ?
ff3c-ff3f sound (see below)
fff0      interrupt enable
fff1      ?
fff8-fff9 background playfield position
fffa      background color? (0 = standard  1 = reddish)
fffb      background enable?
fffe      ?

interrupts:
VBlank triggers IRQ, handled with interrupt mode 1
NMI enters the test mode.

-------------

Zaxxon Sound Information: (from the schematics)
by Frank Palazzolo

There are four registers in the 8255. they are mapped to
(111x xxxx 0011 11pp) by Zaxxon.  Zaxxon writes to these
at FF3C-FF3F.

There are three modes of the 8255, but by the schematics I
can see that Zaxxon is using "Mode 0", which is very simple.

Important Note:
These are all Active-Low outputs.
A 1 De-activates the sound, while a 0 Activates/Triggers it

Port A Output:
FF3C bit7 Battleship
     bit6 Laser
     bit5 Base Missle
     bit4 Homing Missle
     bit3 Player Ship D
     bit2 Player Ship C
     bit1 Player Ship B
     bit0 Player Ship A

Port B Output:
FF3D bit7 Cannon
     bit6 N/C
     bit5 M-Exp
     bit4 S-Exp
     bit3 N/C
     bit2 N/C
     bit1 N/C
     bit0 N/C

Port C Output:
FF3E bit7 N/C
     bit6 N/C
     bit5 N/C
     bit4 N/C
     bit3 Alarm 3
     bit2 Alarm 2
     bit1 N/C
     bit0 Shot

Control Byte:
FF3F Should be written an 0x80 for Mode 0
     (Very Simple) operation of the 8255

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"



int zaxxon_IN2_r(int offset);

extern unsigned char *zaxxon_background_position;
extern unsigned char *zaxxon_background_color;
extern unsigned char *zaxxon_background_enable;
void zaxxon_vh_convert_color_prom(unsigned char *palette, unsigned char *colortable,const unsigned char *color_prom);
int  zaxxon_vh_start(void);
void zaxxon_vh_stop(void);
void zaxxon_vh_convert_color_prom(unsigned char *palette, unsigned char *colortable,const unsigned char *color_prom);
void zaxxon_vh_screenrefresh(struct osd_bitmap *bitmap);



static struct MemoryReadAddress readmem[] =
{
	{ 0x0000, 0x4fff, MRA_ROM },
	{ 0x6000, 0x6fff, MRA_RAM },
	{ 0x8000, 0x83ff, MRA_RAM },
	{ 0xa000, 0xa0ff, MRA_RAM },
	{ 0xc000, 0xc000, input_port_0_r },	/* IN0 */
	{ 0xc001, 0xc001, input_port_1_r },	/* IN1 */
	{ 0xc100, 0xc100, input_port_2_r },	/* IN2 */
	{ 0xc002, 0xc002, input_port_3_r },	/* DSW0 */
	{ 0xc003, 0xc003, input_port_4_r },	/* DSW1 */
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress writemem[] =
{
	{ 0x0000, 0x4fff, MWA_ROM },
	{ 0x6000, 0x6fff, MWA_RAM },
	{ 0x8000, 0x83ff, videoram_w, &videoram, &videoram_size },
	{ 0xa000, 0xa0ff, MWA_RAM, &spriteram, &spriteram_size },
	{ 0xfff0, 0xfff0, interrupt_enable_w },
	{ 0xfff8, 0xfff9, MWA_RAM, &zaxxon_background_position },
	{ 0xfffa, 0xfffa, MWA_RAM, &zaxxon_background_color },
	{ 0xfffb, 0xfffb, MWA_RAM, &zaxxon_background_enable },
	{ -1 }  /* end of table */
};



/***************************************************************************

  Zaxxon uses NMI to trigger the self test. We use a fake input port to
  tie that event to a keypress.

***************************************************************************/
static int zaxxon_interrupt(void)
{
	if (readinputport(5) & 1)	/* get status of the F2 key */
		return nmi_interrupt();	/* trigger self test */
	else return interrupt();
}

INPUT_PORTS_START( input_ports )
	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN | IPF_8WAY )	/* the self test calls this UP */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP | IPF_8WAY )	/* the self test calls this DOWN */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* probably unused (the self test doesn't mention it) */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* probably unused (the self test doesn't mention it) */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* probably unused (the self test doesn't mention it) */

	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT | IPF_8WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN | IPF_8WAY | IPF_COCKTAIL )	/* the self test calls this UP */
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP | IPF_8WAY | IPF_COCKTAIL )	/* the self test calls this DOWN */
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* probably unused (the self test doesn't mention it) */
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* probably unused (the self test doesn't mention it) */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* probably unused (the self test doesn't mention it) */

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* probably unused (the self test doesn't mention it) */
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* probably unused (the self test doesn't mention it) */
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN )	/* probably unused (the self test doesn't mention it) */
	/* the coin inputs must stay active for exactly one frame, otherwise */
	/* the game will keep inserting coins. */
	PORT_BITX(0x20, IP_ACTIVE_HIGH, IPT_COIN1 | IPF_IMPULSE,
			"Coin A", IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	PORT_BITX(0x40, IP_ACTIVE_HIGH, IPT_COIN2 | IPF_IMPULSE,
			"Coin B", IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
	/* Coin Aux doesn't need IMPULSE to pass the test, but it still needs it */
	/* to avoid the freeze. */
	PORT_BITX(0x80, IP_ACTIVE_HIGH, IPT_COIN3 | IPF_IMPULSE,
			"Coin Aux", IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )

	PORT_START	/* DSW0 */
	PORT_DIPNAME( 0x03, 0x03, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "10000" )
	PORT_DIPSETTING(    0x01, "20000" )
	PORT_DIPSETTING(    0x02, "30000" )
	PORT_DIPSETTING(    0x00, "40000" )
	PORT_DIPNAME( 0x0c, 0x0c, "Difficulty???", IP_KEY_NONE )
	PORT_DIPSETTING(    0x0c, "Easy?" )
	PORT_DIPSETTING(    0x04, "Medium?" )
	PORT_DIPSETTING(    0x08, "Hard?" )
	PORT_DIPSETTING(    0x00, "Hardest?" )
	PORT_DIPNAME( 0x30, 0x30, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x10, "4" )
	PORT_DIPSETTING(    0x20, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "Infinite", IP_KEY_NONE, IP_JOY_NONE, 0 )
	PORT_DIPNAME( 0x40, 0x40, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Off" )
	PORT_DIPSETTING(    0x40, "On" )
	PORT_DIPNAME( 0x80, 0x00, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Upright" )
	PORT_DIPSETTING(    0x80, "Cocktail" )

	PORT_START	/* DSW1 */
 	PORT_DIPNAME( 0x0f, 0x03, "B Coin/Cred", IP_KEY_NONE )
	PORT_DIPSETTING(    0x0f, "4/1" )
	PORT_DIPSETTING(    0x07, "3/1" )
	PORT_DIPSETTING(    0x0b, "2/1" )
	PORT_DIPSETTING(    0x06, "2/1+Bonus each 5" )
	PORT_DIPSETTING(    0x0a, "2/1+Bonus each 2" )
	PORT_DIPSETTING(    0x03, "1/1" )
	PORT_DIPSETTING(    0x02, "1/1+Bonus each 5" )
	PORT_DIPSETTING(    0x0c, "1/1+Bonus each 4" )
	PORT_DIPSETTING(    0x04, "1/1+Bonus each 2" )
	PORT_DIPSETTING(    0x0d, "1/2" )
	PORT_DIPSETTING(    0x08, "1/2+Bonus each 5" )
	PORT_DIPSETTING(    0x00, "1/2+Bonus each 4" )
	PORT_DIPSETTING(    0x05, "1/3" )
	PORT_DIPSETTING(    0x09, "1/4" )
	PORT_DIPSETTING(    0x01, "1/5" )
	PORT_DIPSETTING(    0x0e, "1/6" )
	PORT_DIPNAME( 0xf0, 0x30, "A Coin/Cred", IP_KEY_NONE )
	PORT_DIPSETTING(    0xf0, "4/1" )
	PORT_DIPSETTING(    0x70, "3/1" )
	PORT_DIPSETTING(    0xb0, "2/1" )
	PORT_DIPSETTING(    0x60, "2/1+Bonus each 5" )
	PORT_DIPSETTING(    0xa0, "2/1+Bonus each 2" )
	PORT_DIPSETTING(    0x30, "1/1" )
	PORT_DIPSETTING(    0x20, "1/1+Bonus each 5" )
	PORT_DIPSETTING(    0xc0, "1/1+Bonus each 4" )
	PORT_DIPSETTING(    0x40, "1/1+Bonus each 2" )
	PORT_DIPSETTING(    0xd0, "1/2" )
	PORT_DIPSETTING(    0x80, "1/2+Bonus each 5" )
	PORT_DIPSETTING(    0x00, "1/2+Bonus each 4" )
	PORT_DIPSETTING(    0x50, "1/3" )
	PORT_DIPSETTING(    0x90, "1/4" )
	PORT_DIPSETTING(    0x10, "1/5" )
	PORT_DIPSETTING(    0xe0, "1/6" )

	PORT_START	/* FAKE */
	/* This fake input port is used to get the status of the F2 key, */
	/* and activate the test mode, which is triggered by a NMI */
	PORT_BITX(0x01, IP_ACTIVE_HIGH, IPT_SERVICE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
INPUT_PORTS_END



static struct GfxLayout charlayout1 =
{
	8,8,	/* 8*8 characters */
	256,	/* 256 characters */
	3,	/* 3 bits per pixel (actually 2, the third plane is 0) */
	{ 2*256*8*8, 256*8*8, 0 },	/* the bitplanes are separated */
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	8*8	/* every char takes 8 consecutive bytes */
};

static struct GfxLayout charlayout2 =
{
	8,8,	/* 8*8 characters */
	1024,	/* 1024 characters */
	3,	/* 3 bits per pixel */
	{ 2*1024*8*8, 1024*8*8, 0 },	/* the bitplanes are separated */
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	8*8	/* every char takes 8 consecutive bytes */
};

static struct GfxLayout spritelayout =
{
	32,32,	/* 32*32 sprites */
	64,	/* 64 sprites */
	3,	/* 3 bits per pixel */
	{ 2*64*128*8, 64*128*8, 0 },	/* the bitplanes are separated */
	{ 103*8, 102*8, 101*8, 100*8, 99*8, 98*8, 97*8, 96*8,
			71*8, 70*8, 69*8, 68*8, 67*8, 66*8, 65*8, 64*8,
			39*8, 38*8, 37*8, 36*8, 35*8, 34*8, 33*8, 32*8,
			7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7,
			24*8+0, 24*8+1, 24*8+2, 24*8+3, 24*8+4, 24*8+5, 24*8+6, 24*8+7 },
	128*8	/* every sprite takes 128 consecutive bytes */
};



static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x0000, &charlayout1,   0, 32 },	/* characters */
	{ 1, 0x1800, &charlayout2,   0, 32 },	/* background tiles */
	{ 1, 0x7800, &spritelayout,  0, 32 },	/* sprites */
	{ -1 } /* end of array */
};



/* these are actually Super Zaxxon's PROMs. Most colors are wrong. */
static unsigned char szaxxon_color_prom[] =
{
	/* U98 - palette */
	0x00,0x00,0xF0,0xF0,0x36,0x26,0x00,0xFF,0x00,0x00,0xF0,0xF6,0x26,0x36,0x00,0xFF,
	0x00,0x00,0xF0,0x36,0x00,0x06,0x00,0xFF,0x00,0x00,0xF0,0x06,0x06,0x00,0x00,0x00,
	0x00,0x00,0xF0,0x38,0x06,0x06,0x00,0x00,0x00,0x00,0x36,0xC6,0x46,0x85,0x00,0xFF,
	0x00,0x00,0x36,0x06,0x06,0x06,0x06,0x00,0x00,0xC0,0xDB,0xF6,0xF0,0xE0,0x80,0x06,
	0x00,0x04,0x02,0xF0,0x06,0x06,0x04,0x02,0x00,0x00,0x9B,0xA3,0xA4,0xEC,0xA3,0xA4,
	0x00,0xF0,0xE0,0x36,0x26,0x06,0xFF,0x00,0x00,0x18,0x85,0xB4,0xAC,0x14,0x84,0xE0,
	0x00,0x24,0x1B,0x82,0xE0,0x36,0x12,0x80,0x00,0x00,0xF6,0x18,0x40,0xB4,0xAC,0xA4,
	0x00,0xB4,0xAC,0xE0,0x18,0x40,0xA4,0x00,0x00,0xAC,0xB4,0x36,0xF6,0xA4,0x00,0x18,
	0x00,0xC6,0x06,0xF0,0x36,0x26,0x04,0x34,0x00,0xF6,0xDB,0xE0,0x06,0x26,0x03,0x00,
	0x00,0x00,0x20,0x06,0x26,0x34,0x32,0x30,0x00,0xF6,0x00,0xB4,0xAC,0x52,0xA5,0x06,
	0x00,0xA4,0x00,0x05,0x04,0x16,0x26,0xF6,0x00,0xA3,0x36,0x34,0x30,0xA4,0x16,0x20,
	0x00,0x26,0x85,0xF6,0xD2,0xC0,0x80,0x82,0x00,0x26,0x85,0xF6,0x30,0x28,0x20,0x18,
	0x00,0x26,0x85,0xF6,0x07,0x06,0x05,0x04,0x00,0xA3,0x26,0x06,0x04,0xA4,0x16,0x5D,
	0x00,0xF6,0x36,0x06,0x04,0x26,0x03,0x00,0x00,0x16,0x00,0xEE,0xE6,0xC6,0x5D,0x00,
	0x00,0x07,0x06,0x04,0xC4,0x85,0x05,0x05,0x00,0x00,0x36,0x16,0x05,0xEE,0xE6,0xC6,
	0x00,0xEE,0xE6,0xC4,0x16,0x05,0xC6,0x00,0x00,0xE6,0xEE,0x85,0x36,0xC6,0x00,0x16,
	/* U72 - character color lookup table */
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x03,0x00,0x04,0x00,0x03,0x03,0x02,0x01,
	0x04,0x01,0x04,0x01,0x04,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x06,0x06,0x06,0x01,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x03,0x00,0x04,0x00,0x03,0x03,0x02,0x01,
	0x04,0x01,0x04,0x01,0x04,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x06,0x06,0x06,0x01,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x03,0x00,0x04,0x00,0x03,0x03,0x02,0x01,
	0x04,0x01,0x04,0x01,0x04,0x01,0x01,0x01,0x01,0x01,0x01,0x03,0x06,0x06,0x06,0x01,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x03,0x00,0x04,0x00,0x03,0x03,0x02,0x01,
	0x04,0x01,0x04,0x01,0x04,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x05,0x05,0x06,0x01,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x03,0x00,0x04,0x00,0x03,0x03,0x02,0x01,
	0x04,0x01,0x04,0x01,0x04,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x05,0x05,0x06,0x01,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x03,0x00,0x04,0x00,0x03,0x03,0x02,0x01,
	0x04,0x01,0x04,0x01,0x04,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x05,0x05,0x06,0x01,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x03,0x00,0x04,0x00,0x03,0x03,0x02,0x01,
	0x04,0x01,0x04,0x01,0x04,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x05,0x05,0x04,0x01,
	0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x03,0x00,0x04,0x00,0x03,0x03,0x02,0x01,
	0x04,0x01,0x04,0x01,0x04,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x05,0x05,0x04,0x01
};



static struct MachineDriver machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			3072000,	/* 3.072 Mhz */
			0,
			readmem,writemem,0,0,
			zaxxon_interrupt,1
		}
	},
	60,
	1,	/* single CPU, no need for interleaving */
	0,

	/* video hardware */
	32*8, 32*8, { 2*8, 30*8-1, 0*8, 32*8-1 },
	gfxdecodeinfo,
	256,32*8,
	zaxxon_vh_convert_color_prom,

	VIDEO_TYPE_RASTER,
	0,
	zaxxon_vh_start,
	zaxxon_vh_stop,
	zaxxon_vh_screenrefresh,

	/* sound hardware */
	0,
	0,
	0,
	0
};



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( zaxxon_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "zaxxon.3", 0x0000, 0x2000, 0x5ab9126b )
	ROM_LOAD( "zaxxon.2", 0x2000, 0x2000, 0x37df69f1 )
	ROM_LOAD( "zaxxon.1", 0x4000, 0x1000, 0xba9f739d )

	ROM_REGION(0xd800)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "zaxxon.14", 0x0000, 0x0800, 0xec944b06 )	/* characters */
	ROM_LOAD( "zaxxon.15", 0x0800, 0x0800, 0x77cd19bf )
	/* 1000-17ff empty space to convert the characters as 3bpp instead of 2 */
	ROM_LOAD( "zaxxon.6",  0x1800, 0x2000, 0x27b35545 )	/* background tiles */
	ROM_LOAD( "zaxxon.5",  0x3800, 0x2000, 0x1b73959d )
	ROM_LOAD( "zaxxon.4",  0x5800, 0x2000, 0x3deedf22 )
	ROM_LOAD( "zaxxon.11", 0x7800, 0x2000, 0x666b4c71 )	/* sprites */
	ROM_LOAD( "zaxxon.12", 0x9800, 0x2000, 0x4b4f1449 )
	ROM_LOAD( "zaxxon.13", 0xb800, 0x2000, 0xe2f87f5c )

	ROM_REGION(0x8000)	/* background graphics */
	ROM_LOAD( "zaxxon.8",  0x0000, 0x2000, 0x974ee47c )
	ROM_LOAD( "zaxxon.7",  0x2000, 0x2000, 0xa9dc2e00 )
	ROM_LOAD( "zaxxon.10", 0x4000, 0x2000, 0x46743110 )
	ROM_LOAD( "zaxxon.9",  0x6000, 0x2000, 0x6be85050 )
ROM_END

ROM_START( szaxxon_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "suzaxxon.3", 0x0000, 0x2000, 0x48c0840e )
	ROM_LOAD( "suzaxxon.2", 0x2000, 0x2000, 0xe25f4a85 )
	ROM_LOAD( "suzaxxon.1", 0x4000, 0x1000, 0xd996ee1a )

	ROM_REGION(0xd800)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "suzaxxon.14", 0x0000, 0x0800, 0xe48a4b06 )	/* characters */
	ROM_LOAD( "suzaxxon.15", 0x0800, 0x0800, 0xc6f7e543 )
	/* 1000-17ff empty space to convert the characters as 3bpp instead of 2 */
	ROM_LOAD( "suzaxxon.6",  0x1800, 0x2000, 0xd3a4530c )	/* background tiles */
	ROM_LOAD( "suzaxxon.5",  0x3800, 0x2000, 0x292e67e4 )
	ROM_LOAD( "suzaxxon.4",  0x5800, 0x2000, 0xe2ad3d31 )
	ROM_LOAD( "suzaxxon.11", 0x7800, 0x2000, 0x06ace32c )	/* sprites */
	ROM_LOAD( "suzaxxon.12", 0x9800, 0x2000, 0xc4a9c83d )
	ROM_LOAD( "suzaxxon.13", 0xb800, 0x2000, 0x4d02ddce )

	ROM_REGION(0x8000)	/* background graphics */
	ROM_LOAD( "suzaxxon.8",  0x0000, 0x2000, 0x1e73bfd5 )
	ROM_LOAD( "suzaxxon.7",  0x2000, 0x2000, 0x67046918 )
	ROM_LOAD( "suzaxxon.10", 0x4000, 0x2000, 0x9d8b2333 )
	ROM_LOAD( "suzaxxon.9",  0x6000, 0x2000, 0xb0d97103 )
ROM_END

ROM_START( futspy_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "fs_snd.u27", 0x0000, 0x2000, 0x3423c119 )
	ROM_LOAD( "fs_snd.u28", 0x2000, 0x2000, 0x785c0e84 )
	ROM_LOAD( "fs_snd.u29", 0x4000, 0x1000, 0x5448fa68 )

	ROM_REGION(0x13800)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "fs_snd.u68", 0x0000, 0x0800, 0x27ac8fb0 )	/* characters */
	ROM_LOAD( "fs_snd.u69", 0x0800, 0x0800, 0x47472653 )
	/* 1000-17ff empty space to convert the characters as 3bpp instead of 2 */
	ROM_LOAD( "fs_vid.113", 0x1800, 0x2000, 0x8fc6d336 )	/* background tiles */
	ROM_LOAD( "fs_vid.112", 0x3800, 0x2000, 0x1fdb0ad1 )
	ROM_LOAD( "fs_vid.111", 0x5800, 0x2000, 0xd739fde1 )
/* Future Spy has 128 sprites, this is different from Zaxxon */
	ROM_LOAD( "fs_vid.u77", 0x7800, 0x4000, 0x0c546754 )	/* sprites */
	ROM_LOAD( "fs_vid.u78", 0xb800, 0x4000, 0x65348620 )
	ROM_LOAD( "fs_vid.u79", 0xf800, 0x4000, 0x9df771f5 )

	ROM_REGION(0x8000)	/* background graphics */
	ROM_LOAD( "fs_vid.u91", 0x0000, 0x2000, 0x74c8b47e )
	ROM_LOAD( "fs_vid.u90", 0x2000, 0x2000, 0x794d9111 )
	ROM_LOAD( "fs_vid.u93", 0x4000, 0x2000, 0xa5cb5131 )
	ROM_LOAD( "fs_vid.u92", 0x6000, 0x2000, 0xc7693001 )
ROM_END



static void szaxxon_decode(void)
{
/*
	the values vary, but the translation mask is always layed out like this:

	  0 1 2 3 4 5 6 7 8 9 a b c d e f
	0 A A A A A A A A B B B B B B B B
	1 A A A A A A A A B B B B B B B B
	2 C C C C C C C C D D D D D D D D
	3 C C C C C C C C D D D D D D D D
	4 A A A A A A A A B B B B B B B B
	5 A A A A A A A A B B B B B B B B
	6 C C C C C C C C D D D D D D D D
	7 C C C C C C C C D D D D D D D D
	8 D D D D D D D D C C C C C C C C
	9 D D D D D D D D C C C C C C C C
	a B B B B B B B B A A A A A A A A
	b B B B B B B B B A A A A A A A A
	c D D D D D D D D C C C C C C C C
	d D D D D D D D D C C C C C C C C
	e B B B B B B B B A A A A A A A A
	f B B B B B B B B A A A A A A A A

	(e.g. 0xc0 is XORed with D)
	therefore in the following tables we only keep track of A, B, C and D.
*/

	/* THE TABLES ARE NOT ACCURATELY VERIFIED - SOME VALUES COULD BE WRONG */
	static const unsigned char data_xortable[16][4] =
	{
		{ 0x28,0x28,0x88,0x88 },	/* ...0...0...0...0 */
		{ 0x88,0x88,0x28,0x28 },	/* ...0...0...0...1 */
		{ 0x20,0xa8,0x20,0xA8 },	/* ...0...0...1...0 */
		{ 0x28,0x28,0x88,0x88 },	/* ...0...0...1...1 */
		{ 0x88,0x88,0x28,0x28 },	/* ...0...1...0...0 */
		{ 0x28,0x28,0x88,0x88 },	/* ...0...1...0...1 */
		{ 0x20,0xA8,0x20,0xA8 },	/* ...0...1...1...0 */
		{ 0x88,0x88,0x28,0x28 },	/* ...0...1...1...1 */
		{ 0x88,0x88,0x28,0x28 },	/* ...1...0...0...0 */
		{ 0x28,0x28,0x88,0x88 },	/* ...1...0...0...1 */
		{ 0x28,0x28,0x88,0x88 },	/* ...1...0...1...0 */
		{ 0x20,0xa8,0x20,0xa8 },	/* ...1...0...1...1 */
		{ 0x20,0xa8,0x20,0xa8 },	/* ...1...1...0...0 */
		{ 0x20,0xa8,0x20,0xa8 },	/* ...1...1...0...1 */
		{ 0x88,0x88,0x28,0x28 },	/* ...1...1...1...0 */
		{ 0x28,0x28,0x88,0x88 }		/* ...1...1...1...1 */
	};
	static const unsigned char opcode_xortable[16][4] =
	{
		{ 0x88,0xA0,0xA0,0x88 },	/* ...0...0...0...0 */
		{ 0x08,0x20,0xA8,0x80 },	/* ...0...0...0...1 */
		{ 0xA8,0x20,0x80,0x08 },	/* ...0...0...1...0 */
		{ 0x88,0xA0,0xA0,0x88 },	/* ...0...0...1...1 */
		{ 0x08,0x20,0xA8,0x80 },	/* ...0...1...0...0 */
//                       ^^^^ at 0x010A, this should be 0x28
		{ 0x88,0xA0,0xA0,0x88 },	/* ...0...1...0...1 */
		{ 0xA8,0x20,0x80,0x08 },	/* ...0...1...1...0 */
		{ 0x08,0x20,0xA8,0x80 },	/* ...0...1...1...1 */
		{ 0x08,0x20,0xA8,0x80 },	/* ...1...0...0...0 */
//                       ^^^^ at 0x1020 and 0x1022, this should be 0x28
		{ 0x88,0xA0,0xA0,0x88 },	/* ...1...0...0...1 */
		{ 0x88,0xA0,0xA0,0x88 },	/* ...1...0...1...0 */
		{ 0xA8,0x20,0x80,0x08 },	/* ...1...0...1...1 */
		{ 0xA8,0x20,0x80,0x08 },	/* ...1...1...0...0 */
		{ 0xA8,0x20,0x80,0x08 },	/* ...1...1...0...1 */
		{ 0x08,0x20,0xA8,0x80 },	/* ...1...1...1...0 */
		{ 0x88,0xA0,0xA0,0x88 }		/* ...1...1...1...1 */
	};
	int A;


	for (A = 0x0000;A < 0x5000;A++)
	{
		int i,j;
		unsigned char src;


		src = RAM[A];

		/* pick the translation table from bits 0, 4, 8 and 12 of the address */
		i = (A & 1) + (((A >> 4) & 1) << 1) + (((A >> 8) & 1) << 2) + (((A >> 12) & 1) << 3);

		/* pick the offset in the table from bits 1 and 3 of the source data */
		j = ((src >> 3) & 1) + (((src >> 5) & 1) << 1);

		/* the bottom half of the translation table is the mirror image of the top */
		if (src & 0x80) j = 3 - j;

		/* decode the ROM data */
		RAM[A] = src ^ data_xortable[i][j];

		/* now decode the opcodes */
		ROM[A] = src ^ opcode_xortable[i][j];
	}
}



static int hiload(void)
{
	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0x6110],"\x00\x89\x00",3) == 0 &&
			memcmp(&RAM[0x6179],"\x00\x37\x00",3) == 0)
	{
		void *f;


		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x6100],21*6);
			RAM[0x6038] = RAM[0x6110];
			RAM[0x6039] = RAM[0x6111];
			RAM[0x603a] = RAM[0x6112];
			osd_fclose(f);
		}

		return 1;
	}
	else return 0;	/* we can't load the hi scores yet */
}



static void hisave(void)
{
	/* make sure that the high score table is still valid (entering the */
	/* test mode corrupts it) */
	if (memcmp(&RAM[0x6110],"\x00\x00\x00",3) != 0)
	{
		void *f;


		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
		{
			osd_fwrite(f,&RAM[0x6100],21*6);
			osd_fclose(f);
		}
	}
}



struct GameDriver zaxxon_driver =
{
	"Zaxxon",
	"zaxxon",
	"Mirko Buffoni (MAME driver)\nNicola Salmoria (MAME driver)\nTim Lindquist (color info)",
	&machine_driver,

	zaxxon_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	0/*TBR*/,input_ports,0/*TBR*/,0/*TBR*/,0/*TBR*/,

	szaxxon_color_prom, 0, 0,
	ORIENTATION_DEFAULT,

	hiload, hisave
};

/* Super Zaxxon runs on hardware similar to Zaxxon, but the program ROMs are encrypted */
/* so it isn't working. */
struct GameDriver szaxxon_driver =
{
	"Super Zaxxon",
	"szaxxon",
	"Mirko Buffoni (MAME driver)\nNicola Salmoria (MAME driver)\nTim Lindquist (encryption and color info)",
	&machine_driver,

	szaxxon_rom,
	0, szaxxon_decode,
	0,
	0,	/* sound_prom */

	0/*TBR*/,input_ports,0/*TBR*/,0/*TBR*/,0/*TBR*/,

	szaxxon_color_prom, 0, 0,
	ORIENTATION_DEFAULT,

	hiload, hisave
};

/* Future Spy runs on hardware similar to Zaxxon, but the program ROMs are encrypted */
/* so it isn't working. */
struct GameDriver futspy_driver =
{
	"Future Spy",
	"futspy",
	"Nicola Salmoria",
	&machine_driver,

	futspy_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	0/*TBR*/,input_ports,0/*TBR*/,0/*TBR*/,0/*TBR*/,

	szaxxon_color_prom, 0, 0,
	ORIENTATION_ROTATE_180,

	0, 0
};

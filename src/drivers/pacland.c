/***************************************************************************

PacLand memory map (preliminary)

Ernesto Corvi
ernesto@imagina.com

Main CPU:
0000-1fff Character ram ( video ram ) + colorram
2000-3000 spriteram?
3000-3800 RAM
3800-3801 Background 1 scrolling
3a00-3a01 Background 2 scrolling
3c00-3c00 Bank and ROM Selector
4000-6000 Banked ROMs
6800-6bff Shared RAM with the MCU
8000-ffff ROM

MCU:
0000-0027 Internal registers, timers and ports.
0040-13ff RAM
1000-13ff Shared RAM with the Main CPU
8000-a000 MCU external ROM
c000-cfff Namco Sound ?
d000-d003 Dip Switches/Joysticks
f000-ffff MCU internal ROM

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "cpu/m6800/m6800.h"


static unsigned char *sharedram1;

void pacland_scroll0_w(int offset,int data);
void pacland_scroll1_w(int offset,int data);
void pacland_bankswitch_w(int offset,int data);
void pacland_vh_convert_color_prom(unsigned char *palette, unsigned short *colortable,const unsigned char *color_prom);
int pacland_vh_start(void);
void pacland_vh_stop(void);
void pacland_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh);


static int sharedram1_r( int offset )
{
	return sharedram1[offset];
}

static void sharedram1_w( int offset, int val )
{
	sharedram1[offset] = val;
}

static void pacland_halt_mcu_w( int offset, int data )
{
	if ( offset == 0 ) {
		cpu_reset( 1 );

		cpu_halt( 1, 1 );
	} else {
		cpu_halt( 1, 0 );
	}
}


/* Stubs to pass the correct Dip Switch setup to the MCU */
static int dsw_r0( int offset )
{
	/* Hi 4 bits = DSWA Hi 4 bits */
	/* Lo 4 bits = DSWB Hi 4 bits */
	int r = readinputport( 0 );
	r &= 0xf0;
	r |= ( readinputport( 1 ) >> 4 ) & 0x0f;
	return ~r; /* Active Low */
}

static int dsw_r1( int offset )
{
	/* Hi 4 bits = DSWA Lo 4 bits */
	/* Lo 4 bits = DSWB Lo 4 bits */
	int r = ( readinputport( 0 ) & 0x0f ) << 4;
	r |= readinputport( 1 ) & 0x0f;
	return ~r; /* Active Low */
}

static void pacland_coin_w(int offset,int data)
{
	coin_lockout_w(0,data & 1);
	coin_lockout_w(1,data & 1);

	coin_counter_w(0,~data & 2);
	coin_counter_w(1,~data & 4);
}

static void pacland_led_w(int offset,int data)
{
	osd_led_w(0,data >> 3);
	osd_led_w(1,data >> 4);
}


static struct MemoryReadAddress readmem[] =
{
	{ 0x0000, 0x1fff, videoram_r },
	{ 0x2000, 0x37ff, MRA_RAM },
	{ 0x4000, 0x5fff, MRA_BANK1 },
	{ 0x6800, 0x6bff, sharedram1_r },
	{ 0x7800, 0x7800, MRA_NOP },	/* ??? */
	{ 0x8000, 0xffff, MRA_ROM },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress writemem[] =
{
	{ 0x0000, 0x2000, videoram_w, &videoram, &videoram_size },
	{ 0x2000, 0x37ff, MWA_RAM },
	{ 0x2700, 0x27ff, MWA_RAM, &spriteram, &spriteram_size },
	{ 0x2f00, 0x2fff, MWA_RAM, &spriteram_2 },
	{ 0x3700, 0x37ff, MWA_RAM, &spriteram_3 },
	{ 0x3800, 0x3801, pacland_scroll0_w },
	{ 0x3a00, 0x3a01, pacland_scroll1_w },
	{ 0x3c00, 0x3c00, pacland_bankswitch_w },
	{ 0x4000, 0x5fff, MWA_ROM },
	{ 0x6800, 0x6bff, sharedram1_w, &sharedram1 },
	{ 0x7000, 0x7000, MWA_NOP },	/* ??? */
	{ 0x7800, 0x7800, MWA_NOP },	/* ??? */
	{ 0x8000, 0x8800, pacland_halt_mcu_w },
	{ 0x9800, 0x9800, MWA_NOP },	/* ??? */
	{ 0x8000, 0xffff, MWA_ROM },
	{ -1 }	/* end of table */
};

static struct MemoryReadAddress mcu_readmem[] =
{
	{ 0x0000, 0x001f, hd63701_internal_registers_r },
	{ 0x0080, 0x00ff, MRA_RAM },
	{ 0x1100, 0x113f, MRA_RAM, &namco_soundregs }, /* PSG device */
	{ 0x1000, 0x13ff, sharedram1_r },
	{ 0x8000, 0x9fff, MRA_ROM },
	{ 0xc000, 0xc800, MRA_RAM },
	{ 0xd000, 0xd000, dsw_r0 },
	{ 0xd000, 0xd001, dsw_r1 },
	{ 0xd000, 0xd002, input_port_2_r },
	{ 0xd000, 0xd003, input_port_3_r },
	{ 0xf000, 0xffff, MRA_ROM },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress mcu_writemem[] =
{
	{ 0x0000, 0x001f, hd63701_internal_registers_w },
	{ 0x0080, 0x00ff, MWA_RAM },
	{ 0x1100, 0x113f, namcos1_sound_w }, /* PSG device */
	{ 0x1000, 0x13ff, sharedram1_w },
	{ 0x2000, 0x2000, MWA_NOP }, // ???? (w)
	{ 0x4000, 0x4000, MWA_NOP }, // ???? (w)
	{ 0x6000, 0x6000, MWA_NOP }, // ???? (w)
	{ 0x8000, 0x9fff, MWA_ROM },
	{ 0xc000, 0xc7ff, MWA_RAM },
	{ 0xf000, 0xffff, MWA_ROM },
	{ -1 }	/* end of table */
};

static struct IOReadPort mcu_readport[] =
{
	{ HD63701_PORT1, HD63701_PORT1, input_port_4_r },
	{ -1 }	/* end of table */
};

static struct IOWritePort mcu_writeport[] =
{
	{ HD63701_PORT1, HD63701_PORT1, pacland_coin_w },
	{ HD63701_PORT2, HD63701_PORT2, pacland_led_w },
	{ -1 }	/* end of table */
};



INPUT_PORTS_START( input_ports )
	PORT_START      /* DSWA */
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x60, 0x00, DEF_STR( Lives ) )
	PORT_DIPSETTING(    0x20, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x40, "4" )
	PORT_DIPSETTING(    0x60, "5" )
	PORT_DIPNAME( 0x80, 0x00, "Test Mode" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	PORT_START      /* DSWB */
	PORT_DIPNAME( 0x01, 0x00, "Start Level Select" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, "Freeze" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_BITX(    0x04, 0x00, IPT_DIPSWITCH_NAME | IPF_CHEAT, "Round Select", IP_KEY_NONE, IP_JOY_NONE )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x18, 0x00, DEF_STR( Difficulty ) )
	PORT_DIPSETTING(    0x08, "Easy" )
	PORT_DIPSETTING(    0x00, "Medium" )
	PORT_DIPSETTING(    0x10, "Hard" )
	PORT_DIPSETTING(    0x18, "Hardest" )
	PORT_DIPNAME( 0xe0, 0x00, DEF_STR( Bonus_Life ) )
	PORT_DIPSETTING(    0x00, "30K,80K,130K,300K,500K,1M" )
	PORT_DIPSETTING(    0x20, "30K,100K,200K,400K,600K,1M" )
	PORT_DIPSETTING(    0x40, "40K,100K,180K,300K,500K,1M" )
	PORT_DIPSETTING(    0x60, "30K,80K,Every 100K" )
	PORT_DIPSETTING(    0x80, "50K,150K,Every 200K" )
	PORT_DIPSETTING(    0xa0, "30K,80K,150K" )
	PORT_DIPSETTING(    0xc0, "40K,100K,200K" )
	PORT_DIPSETTING(    0xe0, "40K" )

	PORT_START	/* Memory Mapped Port */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BITX(    0x40, 0x40, IPT_DIPSWITCH_NAME | IPF_TOGGLE, DEF_STR( Service_Mode ), OSD_KEY_F2, IP_JOY_NONE )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )

	PORT_START	/* Memory Mapped Port */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START	/* MCU Input Port */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_COCKTAIL )
INPUT_PORTS_END



static struct GfxLayout spritelayout =
{
	16,16,       	/* 16*16 sprites */
	256,           	/* 256 sprites */
	4,              /* 4 bits per pixel */
	{ 0, 4, 16384*8, 16384*8+4 },
	{ 0, 1, 2, 3, 8*8, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8    /* every sprite takes 256 bytes */
};

static struct GfxLayout charlayout =
{
	8,8,	/* 8*8 characters */
	512,	/* 512 characters */
	2,	/* 2 bits per pixel */
	{ 0, 4 },	/* the bitplanes are packed in the same byte */
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8	/* every char takes 16 consecutive bytes */
};

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x00000, &charlayout,              0, 256 },
	{ 1, 0x02000, &charlayout,          256*4, 256 },
	{ 1, 0x10000, &spritelayout,  256*4+256*4, 3*64 },
	{ 1, 0x18000, &spritelayout,  256*4+256*4, 3*64 },
	{ -1 } /* end of array */
};


static struct namco_interface namco_interface =
{
	23920,	/* sample rate (approximate value) */
	8,		/* number of voices */
	100,	/* playback volume */
	4,		/* memory region */
	0		/* stereo */
};


static struct MachineDriver machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_M6809,
			1500000,	/* 1.500 Mhz (?) */
			0,
			readmem,writemem,0,0,
			interrupt,1
		},
		{
			CPU_HD63701,	/* or compatible 6808 with extra instructions */
//			6000000/4,		/* ??? */
			6000000/3.9,		/* ??? */
			3,
			mcu_readmem,mcu_writemem,mcu_readport,mcu_writeport,
			interrupt,1
		},
	},
	60,DEFAULT_REAL_60HZ_VBLANK_DURATION,
	100,	/* we need heavy synching between the MCU and the CPU */
	0,

	/* video hardware */
	42*8, 32*8, { 3*8, 39*8-1, 2*8, 30*8-1 },
	gfxdecodeinfo,
	256,256*4+256*4+3*64*16,
	pacland_vh_convert_color_prom,

	VIDEO_TYPE_RASTER|VIDEO_MODIFIES_PALETTE,
	0,
	pacland_vh_start,
	pacland_vh_stop,
	pacland_vh_screenrefresh,

	/* sound hardware */
	0,0,0,0,
	{
		{
			SOUND_NAMCO,
			&namco_interface
		}
	}
};


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pacland_rom )
	ROM_REGION(0x20000)	/* 128k for code */
	ROM_LOAD( "pl5_01b.bin",  0x08000, 0x4000, 0xb0ea7631 )
	ROM_LOAD( "pl5_02.bin",   0x0C000, 0x4000, 0xd903e84e )
	/* all the following are banked at 0x4000-0x5fff */
	ROM_LOAD( "pl1-3",        0x10000, 0x4000, 0xaa9fa739 )
	ROM_LOAD( "pl1-4",        0x14000, 0x4000, 0x2b895a90 )
	ROM_LOAD( "pl1-5",        0x18000, 0x4000, 0x7af66200 )
	ROM_LOAD( "pl3_06.bin",   0x1c000, 0x4000, 0x2ffe3319 )

	ROM_REGION_DISPOSE(0x20000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "pl2_12.bin",   0x00000, 0x2000, 0xa63c8726 )	/* chars */
	ROM_LOAD( "pl4_13.bin",   0x02000, 0x2000, 0x3ae582fd )
	ROM_LOAD( "pl1-9",        0x10000, 0x4000, 0xf5d5962b )	/* sprites */
	ROM_LOAD( "pl1-10",       0x14000, 0x4000, 0xc7cf1904 )
	ROM_LOAD( "pl1-8",        0x18000, 0x4000, 0xa2ebfa4a )
	ROM_LOAD( "pl1-11",       0x1c000, 0x4000, 0x6621361a )

	ROM_REGION(0x1400)	/* color PROMs */
	ROM_LOAD( "pl1-2.bin",    0x0000, 0x0400, 0x472885de )	/* red and green component */
	ROM_LOAD( "pl1-1.bin",    0x0400, 0x0400, 0xa78ebdaf )	/* blue component */
	ROM_LOAD( "pl1-3.bin",    0x0800, 0x0400, 0x80558da8 )	/* sprites lookup table */
	ROM_LOAD( "pl1-5.bin",    0x0c00, 0x0400, 0x4b7ee712 )	/* foreground lookup table */
	ROM_LOAD( "pl1-4.bin",    0x1000, 0x0400, 0x3a7be418 )	/* background lookup table */

	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "pl1-7",        0x8000, 0x2000, 0x8c5becae ) /* sub program for the mcu */
	ROM_LOAD( "pl1-mcu.bin",  0xf000, 0x1000, 0x6ef08fb3 ) /* microcontroller */

	ROM_REGION(0x200)	/* sound prom */
	ROM_LOAD( "pl-snd",  0x0000, 0x0200, 0xd3aff2df )
ROM_END

ROM_START( pacland2_rom )
	ROM_REGION(0x20000)	/* 128k for code */
	ROM_LOAD( "pl6_01.bin",   0x08000, 0x4000, 0x4c96e11c )
	ROM_LOAD( "pl6_02.bin",   0x0C000, 0x4000, 0x8cf5bd8d )
	/* all the following are banked at 0x4000-0x5fff */
	ROM_LOAD( "pl1-3",        0x10000, 0x4000, 0xaa9fa739 )
	ROM_LOAD( "pl1-4",        0x14000, 0x4000, 0x2b895a90 )
	ROM_LOAD( "pl1-5",        0x18000, 0x4000, 0x7af66200 )
	ROM_LOAD( "pl1-6",        0x1c000, 0x4000, 0xb01e59a9 )

	ROM_REGION_DISPOSE(0x20000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "pl0_12.bin",   0x00000, 0x2000, 0xc8cb61ab )	/* chars */
	ROM_LOAD( "pl1-13",       0x02000, 0x2000, 0x6c5ed9ae )
	ROM_LOAD( "pl1_09b.bin",  0x10000, 0x4000, 0x80768a87 )	/* sprites */
	ROM_LOAD( "pl1_10b.bin",  0x14000, 0x4000, 0xffd9d66e )
	ROM_LOAD( "pl1_08.bin",   0x18000, 0x4000, 0x2b20e46d )
	ROM_LOAD( "pl1_11.bin",   0x1c000, 0x4000, 0xc59775d8 )

	ROM_REGION(0x1400)	/* color PROMs */
	ROM_LOAD( "pl1-2.bin",    0x0000, 0x0400, 0x472885de )	/* red and green component */
	ROM_LOAD( "pl1-1.bin",    0x0400, 0x0400, 0xa78ebdaf )	/* blue component */
	ROM_LOAD( "pl1-3.bin",    0x0800, 0x0400, 0x80558da8 )	/* sprites lookup table */
	ROM_LOAD( "pl1-5.bin",    0x0c00, 0x0400, 0x4b7ee712 )	/* foreground lookup table */
	ROM_LOAD( "pl1-4.bin",    0x1000, 0x0400, 0x3a7be418 )	/* background lookup table */

	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "pl1-7",        0x8000, 0x2000, 0x8c5becae ) /* sub program for the mcu */
	ROM_LOAD( "pl1-mcu.bin",  0xf000, 0x1000, 0x6ef08fb3 ) /* microcontroller */

	ROM_REGION(0x200)	/* sound prom */
	ROM_LOAD( "pl-snd",  0x0000, 0x0200, 0xd3aff2df )
ROM_END

ROM_START( pacland3_rom )
	ROM_REGION(0x20000)	/* 128k for code */
	ROM_LOAD( "pln1-1",       0x08000, 0x4000, 0xf729fb94 )
	ROM_LOAD( "pln1-2",       0x0C000, 0x4000, 0x5c66eb6f )
	/* all the following are banked at 0x4000-0x5fff */
	ROM_LOAD( "pl1-3",        0x10000, 0x4000, 0xaa9fa739 )
	ROM_LOAD( "pl1-4",        0x14000, 0x4000, 0x2b895a90 )
	ROM_LOAD( "pl1-5",        0x18000, 0x4000, 0x7af66200 )
	ROM_LOAD( "pl1-6",        0x1c000, 0x4000, 0xb01e59a9 )

	ROM_REGION_DISPOSE(0x20000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "pl1-12",       0x00000, 0x2000, 0xc159fbce )	/* chars */
	ROM_LOAD( "pl1-13",       0x02000, 0x2000, 0x6c5ed9ae )
	ROM_LOAD( "pl1_09b.bin",  0x10000, 0x4000, 0x80768a87 )	/* sprites */
	ROM_LOAD( "pl1_10b.bin",  0x14000, 0x4000, 0xffd9d66e )
	ROM_LOAD( "pl1_08.bin",   0x18000, 0x4000, 0x2b20e46d )
	ROM_LOAD( "pl1_11.bin",   0x1c000, 0x4000, 0xc59775d8 )

	ROM_REGION(0x1400)	/* color PROMs */
	ROM_LOAD( "pl1-2.bin",    0x0000, 0x0400, 0x472885de )	/* red and green component */
	ROM_LOAD( "pl1-1.bin",    0x0400, 0x0400, 0xa78ebdaf )	/* blue component */
	ROM_LOAD( "pl1-3.bin",    0x0800, 0x0400, 0x80558da8 )	/* sprites lookup table */
	ROM_LOAD( "pl1-5.bin",    0x0c00, 0x0400, 0x4b7ee712 )	/* foreground lookup table */
	ROM_LOAD( "pl1-4.bin",    0x1000, 0x0400, 0x3a7be418 )	/* background lookup table */

	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "pl1-7",        0x8000, 0x2000, 0x8c5becae ) /* sub program for the mcu */
	ROM_LOAD( "pl1-mcu.bin",  0xf000, 0x1000, 0x6ef08fb3 ) /* microcontroller */

	ROM_REGION(0x200)	/* sound prom */
	ROM_LOAD( "pl-snd",  0x0000, 0x0200, 0xd3aff2df )
ROM_END

ROM_START( paclandm_rom )
	ROM_REGION(0x20000)	/* 128k for code */
	ROM_LOAD( "pl1-1",        0x08000, 0x4000, 0xa938ae99 )
	ROM_LOAD( "pl1-2",        0x0C000, 0x4000, 0x3fe43bb5 )
	/* all the following are banked at 0x4000-0x5fff */
	ROM_LOAD( "pl1-3",        0x10000, 0x4000, 0xaa9fa739 )
	ROM_LOAD( "pl1-4",        0x14000, 0x4000, 0x2b895a90 )
	ROM_LOAD( "pl1-5",        0x18000, 0x4000, 0x7af66200 )
	ROM_LOAD( "pl1-6",        0x1c000, 0x4000, 0xb01e59a9 )

	ROM_REGION_DISPOSE(0x20000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "pl1-12",       0x00000, 0x2000, 0xc159fbce )	/* chars */
	ROM_LOAD( "pl1-13",       0x02000, 0x2000, 0x6c5ed9ae )
	ROM_LOAD( "pl1-9",        0x10000, 0x4000, 0xf5d5962b )	/* sprites */
	ROM_LOAD( "pl1-10",       0x14000, 0x4000, 0xc7cf1904 )
	ROM_LOAD( "pl1-8",        0x18000, 0x4000, 0xa2ebfa4a )
	ROM_LOAD( "pl1-11",       0x1c000, 0x4000, 0x6621361a )

	ROM_REGION(0x1400)	/* color PROMs */
	ROM_LOAD( "pl1-2.bin",    0x0000, 0x0400, 0x472885de )	/* red and green component */
	ROM_LOAD( "pl1-1.bin",    0x0400, 0x0400, 0xa78ebdaf )	/* blue component */
	ROM_LOAD( "pl1-3.bin",    0x0800, 0x0400, 0x80558da8 )	/* sprites lookup table */
	ROM_LOAD( "pl1-5.bin",    0x0c00, 0x0400, 0x4b7ee712 )	/* foreground lookup table */
	ROM_LOAD( "pl1-4.bin",    0x1000, 0x0400, 0x3a7be418 )	/* background lookup table */

	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "pl1-7",        0x8000, 0x2000, 0x8c5becae ) /* sub program for the mcu */
	ROM_LOAD( "pl1-mcu.bin",  0xf000, 0x1000, 0x6ef08fb3 ) /* microcontroller */

	ROM_REGION(0x200)	/* sound prom */
	ROM_LOAD( "pl-snd",  0x0000, 0x0200, 0xd3aff2df )
ROM_END

ROM_START( skykid_rom )
	ROM_REGION(0x20000)	/* 128k for code */
	ROM_LOAD( "sk2-6c.bin",   0x08000, 0x4000, 0xea8a5822 )	/* ? */
	ROM_LOAD( "sk1-6b.bin",   0x0c000, 0x4000, 0x7abe6c6c )
	/* all the following are banked at 0x4000-0x5fff */
	ROM_LOAD( "sk3-6d.bin",   0x10000, 0x4000, 0x314b8765 )	/* ? */
//	ROM_LOAD( "pl1-4",        0x14000, 0x4000, 0x2b895a90 )
//	ROM_LOAD( "pl1-5",        0x18000, 0x4000, 0x7af66200 )
//	ROM_LOAD( "pl3_06.bin",   0x1c000, 0x4000, 0x2ffe3319 )

	ROM_REGION_DISPOSE(0x20000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "sk6-6l.bin",   0x00000, 0x2000, 0x58b731b9 )	/* chars */
	ROM_LOAD( "sk5-7e.bin",   0x02000, 0x2000, 0xc33a498e )
	ROM_LOAD( "sk9-10n.bin",  0x10000, 0x4000, 0x44bb7375 )	/* sprites */
	ROM_LOAD( "sk7-10m.bin",  0x18000, 0x4000, 0x3454671d )
//	ROM_LOAD( "pl1-8",        0x18000, 0x4000, 0xa2ebfa4a )
//	ROM_LOAD( "pl1-11",       0x1c000, 0x4000, 0x6621361a )

	ROM_REGION(0x1400)	/* color PROMs */
	ROM_LOAD( "sk2-2p.bin",   0x0000, 0x0100, 0xfc0d5b85 )	/* red and green component */
	ROM_LOAD( "sk1-2n.bin",   0x0400, 0x0100, 0x0218e726 )	/* blue component */
	ROM_LOAD( "sk3-2r.bin",   0x0800, 0x0100, 0xd06b620b )	/* sprites lookup table */
	ROM_LOAD( "sk-6n.bin",    0x0c00, 0x0200, 0x161514a4 )	/* foreground lookup table */
	ROM_LOAD( "sk-5n.bin",    0x1000, 0x0200, 0xc697ac72 )	/* background lookup table */

	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "sk4-3c.bin",   0x8000, 0x2000, 0xa460d0e0 ) /* sub program for the mcu */
	ROM_LOAD( "sk1-mcu.bin",  0xf000, 0x1000, 0x6ef08fb3 ) /* microcontroller */

	ROM_REGION(0x200)	/* sound prom */
	ROM_LOAD( "skysnd.bin",   0x0000, 0x0200, 0x19e2d4b9 )
ROM_END


static int hiload(void)
{
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	if  (memcmp(&RAM[0x2140],"\x00\x08\x00",3) == 0 &&
			memcmp(&RAM[0x02187],"\xE6\xE6\xE6",3) == 0 )
	{
		void *f;

		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x2140],74);
			RAM[0x205D] = RAM[0x2140];
			RAM[0x205E] = RAM[0x2141];
			RAM[0x205F] = RAM[0x2142];

			osd_fclose(f);
		}

		return 1;
	}
	else return 0;   /* we can't load the hi scores yet */
}

static void hisave(void)
{
	void *f;
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x2140],74);
		osd_fclose(f);
	}
}



struct GameDriver pacland_driver =
{
	__FILE__,
	0,
	"pacland",
	"Pac-Land (set 1)",
	"1984",
	"Namco",
	"Ernesto Corvi\nMirko Buffoni\nNicola Salmoria",
	0,
	&machine_driver,
	0,

	pacland_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_DEFAULT,

	hiload, hisave
};

struct GameDriver pacland2_driver =
{
	__FILE__,
	&pacland_driver,
	"pacland2",
	"Pac-Land (set 2)",
	"1984",
	"Namco",
	"Ernesto Corvi\nMirko Buffoni\nNicola Salmoria",
	0,
	&machine_driver,
	0,

	pacland2_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_DEFAULT,

	hiload, hisave
};

struct GameDriver pacland3_driver =
{
	__FILE__,
	&pacland_driver,
	"pacland3",
	"Pac-Land (set 3)",
	"1984",
	"Namco",
	"Ernesto Corvi\nMirko Buffoni\nNicola Salmoria",
	0,
	&machine_driver,
	0,

	pacland3_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_DEFAULT,

	hiload, hisave
};

struct GameDriver paclandm_driver =
{
	__FILE__,
	&pacland_driver,
	"paclandm",
	"Pac-Land (Midway)",
	"1984",
	"[Namco] (Bally Midway license)",
	"Ernesto Corvi\nMirko Buffoni\nNicola Salmoria",
	0,
	&machine_driver,
	0,

	paclandm_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_DEFAULT,

	hiload, hisave
};

struct GameDriver skykid_driver =
{
	__FILE__,
	0,
	"skykid",
	"Sky Kid",
	"1985",
	"Namco",
	"Ernesto Corvi\nMirko Buffoni\nNicola Salmoria",
	0,
	&machine_driver,
	0,

	skykid_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	input_ports,

	PROM_MEMORY_REGION(2), 0, 0,
	ORIENTATION_DEFAULT,

	0, 0
};

/***************************************************************************

Based on drivers from Juno First emulator by Chris Hardy (chrish@kcbbs.gen.nz)

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "sndhrdw/generic.h"
#include "sndhrdw/8910intf.h"
#include "M6809.h"



void rocnrope_flipscreen_w(int offset,int data);
void rocnrope_vh_convert_color_prom(unsigned char *palette, unsigned char *colortable,const unsigned char *color_prom);
void rocnrope_vh_screenrefresh(struct osd_bitmap *bitmap);

void rocnrope_sh_irqtrigger_w(int offset,int data);
int rocnrope_sh_start(void);

unsigned char KonamiDecode( unsigned char opcode, unsigned short address );

void rocnrope_init_machine(void)
{
	/* Set optimization flags for M6809 */
	m6809_Flags = M6809_FAST_S | M6809_FAST_U;
}

/* Roc'n'Rope has the IRQ vectors in RAM. The rom contains $FFFF at this address! */
void rocnrope_interrupt_vector_w(int offset, int data)
{
	RAM[0xFFF2+offset] = data;
}



static struct MemoryReadAddress readmem[] =
{
	{ 0x3080, 0x3080, input_port_0_r }, /* IO Coin */
	{ 0x3081, 0x3081, input_port_1_r }, /* P1 IO */
	{ 0x3082, 0x3082, input_port_2_r }, /* P2 IO */
	{ 0x3083, 0x3083, input_port_3_r }, /* DSW 0 */
	{ 0x3000, 0x3000, input_port_4_r }, /* DSW 1 */
	{ 0x3100, 0x3100, input_port_5_r }, /* DSW 2 */
	{ 0x4000, 0x5fff, MRA_RAM },
	{ 0x6000, 0xffff, MRA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress writemem[] =
{
	{ 0x4000, 0x403f, MWA_RAM, &spriteram_2 },
	{ 0x4040, 0x43ff, MWA_RAM },
	{ 0x4400, 0x443f, MWA_RAM, &spriteram, &spriteram_size },
	{ 0x4440, 0x47ff, MWA_RAM },
	{ 0x4800, 0x4bff, colorram_w, &colorram },
	{ 0x4c00, 0x4fff, videoram_w, &videoram, &videoram_size },
	{ 0x5000, 0x5fff, MWA_RAM },
	{ 0x8000, 0x8000, watchdog_reset_w },
	{ 0x8080, 0x8080, rocnrope_flipscreen_w },
	{ 0x8081, 0x8081, rocnrope_sh_irqtrigger_w },  /* cause interrupt on audio CPU */
	{ 0x8082, 0x8082, MWA_NOP },	/* interrupt acknowledge??? */
	{ 0x8083, 0x8083, MWA_NOP },	/* Coin counter 1 */
	{ 0x8084, 0x8084, MWA_NOP },	/* Coin counter 2 */
	{ 0x8087, 0x8087, interrupt_enable_w },
	{ 0x8100, 0x8100, soundlatch_w },
	{ 0x8182, 0x818d, rocnrope_interrupt_vector_w },
	{ 0x6000, 0xffff, MWA_ROM },
	{ -1 }  /* end of table */
};

static struct MemoryReadAddress sound_readmem[] =
{
	{ 0x0000, 0x1fff, MRA_ROM },
	{ 0x3000, 0x33ff, MRA_RAM },
	{ 0x4000, 0x4000, AY8910_read_port_0_r },
	{ 0x6000, 0x6000, AY8910_read_port_1_r },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress sound_writemem[] =
{
	{ 0x0000, 0x1fff, MWA_ROM },
	{ 0x3000, 0x33ff, MWA_RAM },
	{ 0x4000, 0x4000, AY8910_write_port_0_w },
	{ 0x5000, 0x5000, AY8910_control_port_0_w },
	{ 0x6000, 0x6000, AY8910_write_port_1_w },
	{ 0x7000, 0x7000, AY8910_control_port_1_w },
	{ 0x8000, 0x8000, MWA_NOP }, /* ??? */
	{ -1 }	/* end of table */
};



INPUT_PORTS_START( input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_4WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_4WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_4WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 | IPF_COCKTAIL )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* DSW0 */
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
	PORT_DIPSETTING(    0x00, "Disabled" )
/* 0x00 disables Coin 2. It still accepts coins and makes the sound, but
   it doesn't give you any credit */

	PORT_START      /* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "255", IP_KEY_NONE, IP_JOY_NONE, 0 )
	PORT_DIPNAME( 0x04, 0x00, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Upright" )
	PORT_DIPSETTING(    0x04, "Cocktail" )
	PORT_DIPNAME( 0x78, 0x78, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x78, "Easy 1" )
	PORT_DIPSETTING(    0x70, "Easy 2" )
	PORT_DIPSETTING(    0x68, "Easy 3" )
	PORT_DIPSETTING(    0x60, "Easy 4" )
	PORT_DIPSETTING(    0x58, "Normal 1" )
	PORT_DIPSETTING(    0x50, "Normal 2" )
	PORT_DIPSETTING(    0x48, "Normal 3" )
	PORT_DIPSETTING(    0x40, "Normal 4" )
	PORT_DIPSETTING(    0x38, "Normal 5" )
	PORT_DIPSETTING(    0x30, "Normal 6" )
	PORT_DIPSETTING(    0x28, "Normal 7" )
	PORT_DIPSETTING(    0x20, "Normal 8" )
	PORT_DIPSETTING(    0x18, "Difficult 1" )
	PORT_DIPSETTING(    0x10, "Difficult 2" )
	PORT_DIPSETTING(    0x08, "Difficult 3" )
	PORT_DIPSETTING(    0x00, "Difficult 4" )
	PORT_DIPNAME( 0x80, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )

	PORT_START      /* DSW2 */
	PORT_DIPNAME( 0x07, 0x07, "First Bonus", IP_KEY_NONE )
	PORT_DIPSETTING(    0x07, "20000" )
	PORT_DIPSETTING(    0x05, "30000" )
	PORT_DIPSETTING(    0x04, "40000" )
	PORT_DIPSETTING(    0x03, "50000" )
	PORT_DIPSETTING(    0x02, "60000" )
	PORT_DIPSETTING(    0x01, "70000" )
	PORT_DIPSETTING(    0x00, "80000" )
	/* 0x06 gives 20000 */
	PORT_DIPNAME( 0x38, 0x38, "Repeated Bonus", IP_KEY_NONE )
	PORT_DIPSETTING(    0x38, "40000" )
	PORT_DIPSETTING(    0x18, "50000" )
	PORT_DIPSETTING(    0x10, "60000" )
	PORT_DIPSETTING(    0x08, "70000" )
	PORT_DIPSETTING(    0x00, "80000" )
	/* 0x20, 0x28 and 0x30 all gives 40000 */
	PORT_DIPNAME( 0x40, 0x00, "Grant Repeated Bonus", IP_KEY_NONE )
	PORT_DIPSETTING(    0x40, "No" )
	PORT_DIPSETTING(    0x00, "Yes" )
	PORT_DIPNAME( 0x80, 0x00, "Unknown DSW 8", IP_KEY_NONE )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
INPUT_PORTS_END



static struct GfxLayout charlayout =
{
	8,8,	/* 8*8 sprites */
	512,	/* 512 characters */
	4,	/* 4 bits per pixel */
	{ 0x2000*8+4, 0x2000*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8	/* every sprite takes 64 consecutive bytes */
};

static struct GfxLayout spritelayout =
{
	16,16,	/* 16*16 sprites */
	256,	/* 256 sprites */
	4,	/* 4 bits per pixel */
	{ 256*64*8+4, 256*64*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8	/* every sprite takes 64 consecutive bytes */
};

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x0000, &charlayout,   0, 16 },
	{ 1, 0x4000, &spritelayout, 0, 16 },
	{ -1 } /* end of array */
};


/* these PROMs come from a bootleg board - they don't completely match the original */
static unsigned char color_prom[] =
{
	0x00,0x07,0x38,0x3F,0xC0,0x77,0x1D,0x88,0xE0,0xF8,0x14,0xAD,0x13,0x48,0x26,0xFF,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* character and sprite lookup table */
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x00,0x03,0x05,0x04,0x02,0x05,0x0A,0x07,0x08,0x01,0x0A,0x0B,0x0C,0x0D,0x0E,0x02,
	0x00,0x0F,0x02,0x03,0x04,0x05,0x06,0x03,0x08,0x01,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x00,0x05,0x02,0x03,0x0F,0x05,0x06,0x09,0x08,0x03,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x09,0x08,0x01,0x0A,0x0B,0x0C,0x0D,0x0E,0x03,
	0x00,0x01,0x02,0x03,0x04,0x01,0x06,0x07,0x08,0x03,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x00,0x05,0x04,0x0B,0x05,0x09,0x0A,0x0B,0x0C,0x04,0x0E,0x0F,0x0C,0x02,0x03,0x0F,
	0x00,0x03,0x05,0x01,0x06,0x05,0x0E,0x0A,0x0D,0x08,0x04,0x03,0x0C,0x07,0x04,0x02,
	0x00,0x05,0x03,0x04,0x05,0x09,0x0A,0x0B,0x0C,0x04,0x0E,0x0F,0x0C,0x02,0x03,0x0F,
	0x00,0x05,0x01,0x0F,0x05,0x05,0x0A,0x0B,0x0C,0x04,0x0E,0x0F,0x0C,0x02,0x03,0x0F,
	0x00,0x05,0x04,0x02,0x05,0x05,0x0A,0x0B,0x0C,0x04,0x0E,0x0F,0x0C,0x02,0x03,0x01,
	0x00,0x05,0x03,0x0B,0x05,0x05,0x0A,0x0B,0x0C,0x04,0x0E,0x0F,0x0C,0x02,0x03,0x0F,
	0x00,0x02,0x05,0x03,0x05,0x05,0x02,0x01,0x0E,0x0A,0x0B,0x07,0x0C,0x02,0x03,0x0F,
	0x00,0x03,0x06,0x0F,0x06,0x02,0x0D,0x0C,0x0B,0x05,0x07,0x09,0x0C,0x01,0x0F,0x03,
	0x00,0x02,0x04,0x06,0x03,0x05,0x08,0x09,0x07,0x0F,0x0E,0x0A,0x0C,0x07,0x07,0x0F,
	0x00,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0F,0x0E,0x0F,0x0F,0x0F,0x0C,0x08,0x08,0x0F
};



static struct MachineDriver machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_M6809,
			2048000,        /* 2 Mhz */
			0,
			readmem,writemem,0,0,
			interrupt,1
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			14318180/4,	/* Z80 Clock is derived from a 14.31818 Mhz crystal */
			2,	/* memory region #2 */
			sound_readmem,sound_writemem,0,0,
			ignore_interrupt,1	/* interrupts are triggered by the main CPU */
		}
	},
	60,
	10,	/* 10 CPU slices per frame - enough for the sound CPU to read all commands */
	rocnrope_init_machine,

	/* video hardware */
	32*8, 32*8, { 0*8, 32*8-1, 2*8, 30*8-1 },
	gfxdecodeinfo,
	32,16*16,
	rocnrope_vh_convert_color_prom,

	VIDEO_TYPE_RASTER,
	0,
	generic_vh_start,
	generic_vh_stop,
	rocnrope_vh_screenrefresh,

	/* sound hardware */
	0,
	rocnrope_sh_start,
	AY8910_sh_stop,
	AY8910_sh_update
};

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( rocnrope_rom )
	ROM_REGION(0x10000)     /* 64k for code */
	ROM_LOAD( "rnr_h1.vid", 0x6000, 0x2000, 0x431c1ca0 )
	ROM_LOAD( "rnr_h2.vid", 0x8000, 0x2000, 0x2e1a952e )
	ROM_LOAD( "rnr_h3.vid", 0xA000, 0x2000, 0x6fae29fa )
	ROM_LOAD( "rnr_h4.vid", 0xC000, 0x2000, 0x1a2852cc )
	ROM_LOAD( "rnr_h5.vid", 0xE000, 0x2000, 0x474255f6 )

	ROM_REGION(0xc000)    /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "rnr_h12.vid", 0x0000, 0x2000, 0xc1895bf3 )
	ROM_LOAD( "rnr_h11.vid", 0x2000, 0x2000, 0x854f7d6d )
	ROM_LOAD( "rnr_a11.vid", 0x4000, 0x2000, 0xccd353a1 )
	ROM_LOAD( "rnr_a12.vid", 0x6000, 0x2000, 0x7918ecd6 )
	ROM_LOAD( "rnr_a9.vid",  0x8000, 0x2000, 0xbbdb0eef )
	ROM_LOAD( "rnr_a10.vid", 0xa000, 0x2000, 0xa087b117 )

	ROM_REGION(0x10000)	/* 64k for the audio CPU */
	ROM_LOAD( "rnr_7a.snd", 0x0000, 0x1000, 0x2c7ea8d8 )
	ROM_LOAD( "rnr_8a.snd", 0x1000, 0x1000, 0x172f0eab )
ROM_END

ROM_START( ropeman_rom )
	ROM_REGION(0x10000)     /* 64k for code */
	ROM_LOAD( "j01_rm01.bin", 0x6000, 0x2000, 0x64a5efa5 )
	ROM_LOAD( "j02_rm02.bin", 0x8000, 0x2000, 0xfebda671 )
	ROM_LOAD( "j03_rm03.bin", 0xA000, 0x2000, 0xbb34d17a )
	ROM_LOAD( "j04_rm04.bin", 0xC000, 0x2000, 0x0d68c368 )
	ROM_LOAD( "j05_rm05.bin", 0xE000, 0x2000, 0x474255f6 )

	ROM_REGION(0xc000)    /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "j12_rm07.bin", 0x0000, 0x2000, 0x9dd6694a )
	ROM_LOAD( "j11_rm06.bin", 0x2000, 0x2000, 0x60afcded )
	ROM_LOAD( "a11_rm10.bin", 0x4000, 0x2000, 0xccd353a1 )
	ROM_LOAD( "a12_rm11.bin", 0x6000, 0x2000, 0x7918ecd6 )
	ROM_LOAD( "a09_rm08.bin", 0x8000, 0x2000, 0xbbdb0eef )
	ROM_LOAD( "a10_rm09.bin", 0xa000, 0x2000, 0xa087b117 )

	ROM_REGION(0x10000)	/* 64k for the audio CPU */
	ROM_LOAD( "a07_rm12.bin", 0x0000, 0x1000, 0x2c7ea8d8 )
	ROM_LOAD( "a08_rm13.bin", 0x1000, 0x1000, 0x172f0eab )
ROM_END

static void rocnrope_decode(void)
{
	int A;


	for (A = 0x6000;A < 0x10000;A++)
	{
		ROM[A] = KonamiDecode(RAM[A],A);
	}
}



static int hiload(void)
{
	/* get RAM pointer (this game is multiCPU, we can't assume the global */
	/* RAM pointer is pointing to the right place) */
	unsigned char *RAM = Machine->memory_region[0];

	if (memcmp(&RAM[0x5160],"\x01\x00\x00",3) == 0 &&
		memcmp(&RAM[0x50A6],"\x01\x00\x00",3) == 0)
	{
		void *f;


		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x5160],0x40);
			RAM[0x50A6] = RAM[0x5160];
			RAM[0x50A7] = RAM[0x5161];
			RAM[0x50A8] = RAM[0x5162];
			osd_fclose(f);
		}

		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}

static void hisave(void)
{
	/* get RAM pointer (this game is multiCPU, we can't assume the global */
	/* RAM pointer is pointing to the right place) */
	unsigned char *RAM = Machine->memory_region[0];
	void *f;

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x5160],0x40);
		osd_fclose(f);
	}
}



struct GameDriver rocnrope_driver =
{
	"Rock'n'Rope",
	"rocnrope",
	"Chris Hardy (MAME driver)\nPaul Swan (color info)\nValerio Verrando (high score save)",
	&machine_driver,

	rocnrope_rom,
	0, rocnrope_decode,
	0,
	0,	/* sound_prom */

	0/*TBR*/,input_ports,0/*TBR*/,0/*TBR*/,0/*TBR*/,

	color_prom, 0, 0,
	ORIENTATION_ROTATE_270,

	hiload, hisave
};

struct GameDriver ropeman_driver =
{
	"Rope Man",
	"ropeman",
	"Chris Hardy (MAME driver)\nPaul Swan (color info)\nValerio Verrando (high score save)",
	&machine_driver,

	ropeman_rom,
	0, rocnrope_decode,
	0,
	0,	/* sound_prom */

	0/*TBR*/,input_ports,0/*TBR*/,0/*TBR*/,0/*TBR*/,

	color_prom, 0, 0,
	ORIENTATION_ROTATE_270,

	hiload, hisave
};

/***************************************************************************

Exerion by Jaleco

Exerion is a unique driver in that it has idiosyncracies that are straight
out of Bizarro World. I submit for your approval:

* The mystery reads from $d802 - timer-based protection?
* The freakish graphics encoding scheme, which no other MAME-supported game uses
* The sprite-ram, and all the funky parameters that go along with it
* The unusual parallaxed background. Is it controlled by the 2nd CPU?

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "sndhrdw/generic.h"
#include "sndhrdw/8910intf.h"

void amidar_vh_convert_color_prom(unsigned char *palette, unsigned char *colortable,const unsigned char *color_prom);

int exerion_vh_start (void);
void exerion_vh_stop (void);
void exerion_vh_screenrefresh (struct osd_bitmap *bitmap);

extern void exerion_videoreg_w (int offset,int data);   /* JB 971127 */

/* This is the first of many Exerion "features." No clue if it's */
/* protection or some sort of timer. */
int exerion_d802_r (int offset) {

	static char val = 0xbe;

	val ^= 0x40;
	return val;
	}

static struct AY8910interface exerion_interface =
{
	2,	/* 2 chips */
	1832727,	/* 1.832727040 MHZ?????? */
	{ 0x20ff, 0x20ff },
	{ 0 },
	{ 0 },
	{ 0 },
	{ 0 }
};


int exerion_sh_start(void)
{
	return AY8910_sh_start(&exerion_interface);
}

static struct MemoryReadAddress readmem[] =
{
	{ 0x0000, 0x5fff, MRA_ROM },
	{ 0x6000, 0x67ef, MRA_RAM },
	{ 0x67f0, 0x67ff, MRA_RAM },
	{ 0x8000, 0x8bff, MRA_RAM },
	{ 0xa000, 0xa000, input_port_0_r },
	{ 0xa800, 0xa800, input_port_1_r },
	{ 0xb000, 0xb000, input_port_2_r },
	{ 0xd802, 0xd802, exerion_d802_r },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress writemem[] =
{
	{ 0x0000, 0x5fff, MWA_ROM },
	{ 0x6000, 0x67ef, MWA_RAM },
	{ 0x67f0, 0x67ff, MWA_RAM }, /* I believe this is somehow the parallaxed background */
	{ 0x8000, 0x87ff, MWA_RAM, &videoram, &videoram_size },
	{ 0x8800, 0x887f, MWA_RAM, &spriteram, &spriteram_size },   /* JB 971127 */
	{ 0x8800, 0x8bff, MWA_RAM },
	{ 0xc000, 0xc7ff, exerion_videoreg_w }, /* JB 971127 */
	{ 0xc800, 0xc800, sound_command_w },
	{ 0xd000, 0xd000, AY8910_control_port_0_w },
	{ 0xd001, 0xd001, AY8910_write_port_0_w },
	{ 0xd800, 0xd800, AY8910_control_port_1_w },
	{ 0xd801, 0xd801, AY8910_write_port_1_w },
	{ -1 }  /* end of table */
};


#ifdef TRYSOUND
static struct MemoryReadAddress sound_readmem[] =
{
	{ 0x0000, 0x1fff, MRA_ROM },
	{ 0x4001, 0x4001, MRA_RAM },
	{ 0x6000, 0x6000, sound_command_latch_r },
	{ 0xa000, 0xa000, MRA_NOP }, /* ??? */
	{ 0x4000, 0x47ff, MRA_RAM },
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress sound_writemem[] =
{
	{ 0x0000, 0x1fff, MWA_ROM },
	{ 0x4000, 0x47ff, MWA_RAM },
//	{ 0x8000, 0x800f, MWA_RAM },
//	{ 0x8001, 0x8001, MWA_RAM },
//	{ 0x8003, 0x8003, MWA_RAM },
//	{ 0x8005, 0x8005, MWA_RAM },
//	{ 0x8007, 0x8007, MWA_RAM },
	{ -1 }  /* end of table */
};

#endif

/* Exerion triggers NMIs on vblank */
int exerion_interrupt(void)
{
	if (readinputport(3) & 1)
		return nmi_interrupt();
	else return ignore_interrupt();
}

INPUT_PORTS_START( exerion_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START      /* DSW0 */
	PORT_DIPNAME( 0x07, 0x02, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "1" )
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x02, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x04, "5" )
	PORT_DIPSETTING(    0x07, "Infinite" )
	PORT_DIPNAME( 0x18, 0x00, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "10000" )
	PORT_DIPSETTING(    0x08, "20000" )
	PORT_DIPSETTING(    0x10, "30000" )
	PORT_DIPSETTING(    0x18, "40000" )
	PORT_DIPNAME( 0x20, 0x20, "Coinage", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x20, "1 Coin/1 Credit" )
	PORT_DIPNAME( 0x40, 0x00, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Easy" )
	PORT_DIPSETTING(    0x40, "Hard" )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START      /* VBLANK */
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_VBLANK )

	PORT_START      /* FAKE */
	/* The coin slots are not memory mapped. */
	/* This fake input port is used by the interrupt */
	/* handler to be notified of coin insertions. We use IPF_IMPULSE to */
	/* trigger exactly one interrupt, without having to check when the */
	/* user releases the key. */
	PORT_BITX(0x01, IP_ACTIVE_HIGH, IPT_COIN1 | IPF_IMPULSE,
			IP_NAME_DEFAULT, IP_KEY_DEFAULT, IP_JOY_DEFAULT, 1 )
INPUT_PORTS_END

/* JB 971124 */
static struct GfxLayout charlayout =
{
	8,8,            /* 8*8 characters */
	512*5,          /* total number of chars */
	2,              /* 2 bits per pixel (# of planes) */
	{ 0, 4 },       /* start of every bitplane */
	{ 16*7, 16*6, 16*5, 16*4, 16*3, 16*2, 16*1, 16*0 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	16*8            /* every char takes 16 consecutive bytes */
};

#if 0
/* just leave as 8 x 8 characters */
/* JB 971124 */
static struct GfxLayout spritelayout =
{
	8,8,            /* 8*8 sprites */
	512*2,          /* total number of sprites in the rom */
	2,              /* 2 bits per pixel (# of planes) */
	{ 0, 4 },       /* start of every bitplane */
	{ 16*7, 16*6, 16*5, 16*4, 16*3, 16*2, 16*1, 16*0 },
	{ 3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0 },
	16*8            /* every char takes 16 consecutive bytes */
};
#endif

#if 0
/* try to form 16 x 16 characters without reorganizing bytes in exerion_decode() */
/* JB 971127 */
static struct GfxLayout spritelayout =
{
	16,16,          /* 16*16 sprites */
	128*2,          /* total number of sprites in the rom */
	2,              /* 2 bits per pixel (# of planes) */
	{ 0, 4 },       /* start of every bitplane */
	{ 2048+16*7, 2048+16*6, 2048+16*5, 2048+16*4, 2048+16*3, 2048+16*2, 2048+16*1, 2048+16*0,
	  16*7, 16*6, 16*5, 16*4, 16*3, 16*2, 16*1, 16*0 },
	{  3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0,
	  128+3, 128+2, 128+1, 128+0, 128+8+3, 128+8+2, 128+8+1, 128+8+0 },
	32*8            /* every sprite takes 32 consecutive bytes */
};
#endif

#if 1
/* 16 x 16 characters after reorganizing bytes in exerion_decode() */
/* JB 971127 */
static struct GfxLayout spritelayout =
{
	16,16,          /* 16*16 sprites */
	128*2,          /* total number of sprites in the rom */
	2,              /* 2 bits per pixel (# of planes) */
	{ 0, 4 },       /* start of every bitplane */
	{ 256+16*7, 256+16*6, 256+16*5, 256+16*4, 256+16*3, 256+16*2, 256+16*1, 256+16*0,
	  16*7, 16*6, 16*5, 16*4, 16*3, 16*2, 16*1, 16*0 },
	{  3, 2, 1, 0, 8+3, 8+2, 8+1, 8+0,
	  128+3, 128+2, 128+1, 128+0, 128+8+3, 128+8+2, 128+8+1, 128+8+0 },
	64*8            /* every sprite takes 32 consecutive bytes */
};
#endif

static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x0000, &charlayout,  0, 8 },
	{ 1, 0xa000, &spritelayout,0, 8 },/* JB 971124 */
	{ -1 } /* end of array */
};


/* this is NOT the original color PROM */
static unsigned char exerion_color_prom[] =
{
	/* palette */
	0x00,0x07,0xC0,0xB6,0x00,0x38,0xC5,0x67,0x00,0x30,0x07,0x3F,0x00,0x07,0x30,0x3F,
	0x00,0x3F,0x30,0x07,0x00,0x38,0x67,0x3F,0x00,0xFF,0x07,0xDF,0x00,0xF8,0x07,0xFF
};


static struct MachineDriver machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			3072000,        /* 3.072 Mhz */
			0,
			readmem,writemem,0,0,
			exerion_interrupt,1
		},
#ifdef TRYSOUND
		{
			CPU_Z80,
			3072000,        /* 2 Mhz?????? */
			2,      /* memory region #2 */
			sound_readmem,sound_writemem,0,0,
			ignore_interrupt,1
		}
#endif
	},
	60,
	10,
	0,

	/* video hardware */
/////// 28*8, 36*8, { 0*8, 28*8-1, 0*8, 36*8-1 },
	28*8, 40*8, { 0*8, 28*8-1, 0*8, 40*8-1 },       /* JB 971127 */
	gfxdecodeinfo,
	32,32,
	amidar_vh_convert_color_prom,

	VIDEO_TYPE_RASTER|VIDEO_SUPPORTS_DIRTY,
	0,
	exerion_vh_start,
	exerion_vh_stop,
	exerion_vh_screenrefresh,

	/* sound hardware */
	0,
	exerion_sh_start,
	AY8910_sh_stop,
	AY8910_sh_update
};


/* JB 971127 */
static void exerion_decode(void)
{
	int i, x, offs;
	unsigned char *RAM;
	unsigned char *src;

	/*
		The gfx ROMs are stored in a half-assed fashion that makes using them with
		MAME challenging. We shuffle the bytes around to make it easier.

		They have been loaded at 0xe000, we write them at 0x0000.
	*/

	RAM = Machine->memory_region[1];
	src = RAM + 0xe000;

	for (i=0; i<(512*7); i++)
	{
		offs = (i/16)*2048 + (i%16)*16;

		for (x=0; x<8; x++)
		{
			int d, s;

			d = i*16 + x*2;
			s = (offs + 256*x) / 8;

			RAM[d] = src[s];
			RAM[d+1] = src[s+1];
		}
	}

	/* now reorganize sprites */

	/* get a fresh copy to work with */
	memcpy (&RAM[0x18000], &RAM[0xa000], 0x4000);

	/* for each of the 256 sprites, make the 4 bytes contiguous */
	for (i=0; i<(128*2); i++)
	{
		int d, s;

		/* this assumes that sprite 1 is offset by 32 characters (16*32 bytes) from sprite 0 */
		offs = 16 * ((i/32)*2 + (i%32)*32);     // byte offset of sprite

		/* this assumes that sprite 1 is right next to sprite 0 */
		//offs = 16 * i * 2; // byte offset of sprite

		d = 0xa000 + i*64;
		s = 0x18000 + offs;

		memcpy (&RAM[d], &RAM[s], 32);  // first 32 bytes
		memcpy (&RAM[d+32], &RAM[s+256], 32); // next 32 bytes
	}
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( exerion_rom )
	ROM_REGION(0x10000)     /* 64k for code */
	ROM_LOAD( "exerion.07", 0x0000, 0x2000, 0x9b868f9c )
	ROM_LOAD( "exerion.08", 0x2000, 0x2000, 0x61dec9ec )
	ROM_LOAD( "exerion.09", 0x4000, 0x2000, 0x64e47d18 )

	ROM_REGION(0x1c000)     /* temporary space for graphics (disposed after conversion) */
	/* Set aside 0xe000 for the unscrambled graphics */
	ROM_LOAD( "exerion.06",  0x0e000, 0x2000, 0xcd93f589 )
	ROM_LOAD( "exerion.01",  0x10000, 0x2000, 0x47c3cac1 )
	ROM_LOAD( "exerion.02",  0x12000, 0x2000, 0xc4fe0d02 )
	ROM_LOAD( "exerion.03",  0x14000, 0x2000, 0xa32c1e5a )
	ROM_LOAD( "exerion.04",  0x16000, 0x2000, 0xf7c668aa )
	/* sprites */
	ROM_LOAD( "exerion.11",  0x18000, 0x2000, 0xece32a3b )
	ROM_LOAD( "exerion.10",  0x1a000, 0x2000, 0xff808bd0 )

	ROM_REGION(0x10000)     /* 64k for the audio CPU */
	ROM_LOAD( "exerion.05", 0x0000, 0x2000, 0x16090891 )
ROM_END


static int hiload(void)
{
	/* get RAM pointer (this game is multiCPU, we can't assume the global */
	/* RAM pointer is pointing to the right place) */
	unsigned char *RAM = Machine->memory_region[0];

	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0x6600],"\x00\x00",2) == 0)
	{
		void *f;


		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x6600],10);
			osd_fread(f,&RAM[0x6700],40);
			osd_fclose(f);
		}

		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}


static void hisave(void)
{
	void *f;
	unsigned char *RAM = Machine->memory_region[0];

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x6600],10);
		osd_fwrite(f,&RAM[0x6700],40);
		osd_fclose(f);
	}
}


struct GameDriver exerion_driver =
{
	"Exerion",
	"exerion",
	"Brad Oliver\nJohn Butler\nValerio Verrando (high score saving)",
	&machine_driver,

	exerion_rom,
	exerion_decode, 0,
	0,
	0,      /* sound_prom */

	0/*TBR*/,exerion_input_ports,0/*TBR*/,0/*TBR*/,0/*TBR*/,

	exerion_color_prom, 0, 0,
	ORIENTATION_DEFAULT,

	hiload, hisave
};

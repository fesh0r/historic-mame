/***************************************************************************

Make Trax driver

This is very much like the drivers in pacman.c.  I split it out because
of the ugly way I deal with the protection.  If the Special chip ever
gets emulated correctly, this could probably get merged back into
pacman.c.

Make Trax protection description:

Make Trax has a "Special" chip that it uses for copy protection.
The following chart shows when reads and writes may occur:

AAAAAAAA AAAAAAAA
11111100 00000000  <- address bits
54321098 76543210
xxx1xxxx 01xxxxxx - read data bits 4 and 7
xxx1xxxx 10xxxxxx - read data bits 6 and 7
xxx1xxxx 11xxxxxx - read data bits 0 through 5

xxx1xxxx 00xxx100 - write to Special
xxx1xxxx 00xxx101 - write to Special
xxx1xxxx 00xxx110 - write to Special
xxx1xxxx 00xxx111 - write to Special

In practical terms, it reads from Special when it reads from
location $5040-$50FF.  Note that these locations overlap our
inputs and Dip Switches.  Yuk.

I don't bother trapping the writes right now, because I don't
know how to interpret them.  However, comparing against Crush
Roller gives most of the values necessary on the reads.

Instead of always reading from $5040, $5080, and $50C0, the Make
Trax programmers chose to read from a wide variety of locations,
probably to make debugging easier.  To us, it means that for the most
part we can just assign a specific value to return for each address and
we'll be OK.  This falls apart for the following addresses:  $50C0, $508E,
$5090, and $5080.  These addresses should return multiple values.  The other
ugly thing happening is in the ROMs at $3AE5.  It keeps checking for
different values of $50C0 and $5080, and weird things happen if it gets
the wrong values.  The only way I've found around these is to patch the
ROMs using the same patches Crush Roller uses.  Even this wouldn't be
too bad, except we have to patch sometime after initialization or else
it will fail the ROM checksums.

Incidentally, there are extremely few differences between Crush Roller
and Make Trax.  About 98% of the differences appear to be either unused
bytes, the name of the game, or code related to the protection.  I've
only spotted two or three actual differences in the games, and they all
seem minor.

If anybody cares, here's a list of disassembled addresses for every
read and write to the Special chip (not all of the reads are
specifically for checking the Special bits, some are for checking
player inputs and Dip Switches):

Writes: $0084, $012F, $0178, $023C, $0C4C, $1426, $1802, $1817,
	$280C, $2C2E, $2E22, $3205, $3AB7, $3ACC, $3F3D, $3F40,
	$3F4E, $3F5E
Reads:  $01C8, $01D2, $0260, $030E, $040E, $0416, $046E, $0474,
	$0560, $0568, $05B0, $05B8, $096D, $0972, $0981, $0C27,
	$0C2C, $0F0A, $10B8, $10BE, $111F, $1127, $1156, $115E,
	$11E3, $11E8, $18B7, $18BC, $18CA, $1973, $197A, $1BE7,
	$1C06, $1C9F, $1CAA, $1D79, $213D, $2142, $2389, $238F,
	$2AAE, $2BF4, $2E0A, $39D5, $39DA, $3AE2, $3AEA, $3EE0,
	$3EE9, $3F07, $3F0D
***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"



void pacman_init_machine(void);
int pacman_interrupt(void);

void pengo_vh_convert_color_prom(unsigned char *palette, unsigned char *colortable,const unsigned char *color_prom);
void pengo_flipscreen_w(int offset,int data);
int pacman_vh_start(void);
void pengo_vh_screenrefresh(struct osd_bitmap *bitmap);

extern unsigned char *pengo_soundregs;
void pengo_sound_enable_w(int offset,int data);
void pengo_sound_w(int offset,int data);
int pengo_sh_start(void);
void waveform_sh_stop(void);
void waveform_sh_update(void);


int maketrax_special_port2_r(int offset)
{
	int data;
	int pc;

	pc=cpu_getpreviouspc();

	data=input_port_2_r(offset);

	if (pc==0x1973)
		return data|0x40;
	else if (pc==0x2389)
		return data|0x40;

	switch (offset)
	{
		case 0x01:
		case 0x04:
			data|=0x40;     break;
		case 0x05:
			data|=0xC0;     break;
		default:
			data&=0x3F;     break;
	}

	return data;
}

int maketrax_special_r(int offset)
{
	int pc;

	pc=cpu_getpreviouspc();

	if (pc==0x40E)
		return 0x20;
	else if (pc==0x115E)
		return 0x00;
	else if (pc==0x3AE2)
		return 0x01;

	switch (offset)
	{
		case 0x00:
			return 0x1F;
		case 0x09:
			return 0x30;
		case 0x0C:
			return 0x00;
		default:
			return 0x20;
	}
}

/* We can't use this because the ROM checksum fails */
static void maketrax_rom_decode(void)
{
	ROM[0x0415]=0xC9;
	ROM[0x1978]=0x18;
	ROM[0x238E]=0xC9;
	ROM[0x3AE5]=0xE6;
	ROM[0x3AE6]=0x00;
	ROM[0x3AE7]=0x00;
	ROM[0x3AE8]=0xC9;
	ROM[0x3AED]=0x86;
	ROM[0x3AEE]=0xC0;
	ROM[0x3AEF]=0xB0;

}

/* This is where it gets *really* ugly.  Here's where we do our ROM patches. */
static int maketrax_hiload(void)
{
	static int resetcount;
	void *f;


	/* during a reset, leave time to the game to clear the screen */
	if (++resetcount < 60) return 0;

	/* wait for "HI SCORE" to be on screen */
	if (memcmp(&RAM[0x43d0],"\x53\x40\x49\x48",2) == 0)
	{
		resetcount = 0;

		ROM[0x0415]=0xC9;
		ROM[0x1978]=0x18;
		ROM[0x238E]=0xC9;
		ROM[0x3AE5]=0xE6;
		ROM[0x3AE6]=0x00;
		ROM[0x3AE7]=0x00;
		ROM[0x3AE8]=0xC9;
		ROM[0x3AED]=0x86;
		ROM[0x3AEE]=0xC0;
		ROM[0x3AEF]=0xB0;

		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
                        osd_fread(f,&RAM[0x4E40],30);

                        RAM[0x4c80] = RAM[0x4e43];
                        RAM[0x4c81] = RAM[0x4e44];
                        RAM[0x4c82] = RAM[0x4e45];

			osd_fclose(f);
		}

		return 1;
	}
	else return 0;  /* we can't load the hi scores yet */
}



static void maketrax_hisave(void)
{
	void *f;


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
                osd_fwrite(f,&RAM[0x4E40],30);
		osd_fclose(f);
	}

}


static struct MemoryReadAddress maketrax_readmem[] =
{
	{ 0x0000, 0x3fff, MRA_ROM },
	{ 0x4000, 0x47ff, MRA_RAM },    /* video and color RAM */
	{ 0x4c00, 0x4fff, MRA_RAM },    /* including sprite codes at 4ff0-4fff */
	{ 0x5000, 0x503f, input_port_0_r },     /* IN0 */
	{ 0x5040, 0x507f, input_port_1_r },     /* IN1 */ /* Don't need special here because bit 7=0 always works */
	{ 0x5080, 0x50bf, maketrax_special_port2_r },     /* DSW */
	{ 0x50c0, 0x50ff, maketrax_special_r },
	{ 0x8000, 0x9fff, MRA_ROM },    /* Ms. Pac Man only */
	{ -1 }  /* end of table */
};

static struct MemoryWriteAddress maketrax_writemem[] =
{
	{ 0x0000, 0x3fff, MWA_ROM },
	{ 0x4000, 0x43ff, videoram_w, &videoram, &videoram_size },
	{ 0x4400, 0x47ff, colorram_w, &colorram },
	{ 0x4c00, 0x4fef, MWA_RAM },
	{ 0x4ff0, 0x4fff, MWA_RAM, &spriteram, &spriteram_size },
	{ 0x5000, 0x5000, interrupt_enable_w },
	{ 0x5001, 0x5001, pengo_sound_enable_w },
	{ 0x5002, 0x5002, MWA_NOP },
	{ 0x5003, 0x5003, pengo_flipscreen_w },
	{ 0x5040, 0x505f, pengo_sound_w, &pengo_soundregs },
	{ 0x5060, 0x506f, MWA_RAM, &spriteram_2 },
	{ 0x50c0, 0x50c0, MWA_NOP },
	{ 0x8000, 0x9fff, MWA_ROM },    /* Ms. Pac Man only */
	{ 0xc000, 0xc3ff, videoram_w }, /* mirror address for video ram, */
	{ 0xc400, 0xc7ef, colorram_w }, /* used to display HIGH SCORE and CREDITS */
	{ -1 }  /* end of table */
};



static struct IOWritePort writeport[] =
{
	{ 0, 0, interrupt_vector_w },   /* Pac Man only */
	{ -1 }  /* end of table */
};



INPUT_PORTS_START( maketrax_input_ports )
	PORT_START      /* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_4WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_4WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_4WAY )
	PORT_DIPNAME( 0x10, 0x00, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Upright" )
	PORT_DIPSETTING(    0x10, "Cocktail" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN3 )

	PORT_START      /* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_4WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* Checked as a part of the protection */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* Checked as a part of the protection */

	PORT_START      /* DSW */
	PORT_DIPNAME( 0x03, 0x01, "Coinage", IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "2 Coins/1 Credit" )
	PORT_DIPSETTING(    0x01, "1 Coin/1 Credit" )
	PORT_DIPSETTING(    0x02, "1 Coin/2 Credits" )
	PORT_DIPSETTING(    0x00, "Free Play" )
	PORT_DIPNAME( 0x0c, 0x00, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x08, "5" )
	PORT_DIPSETTING(    0x0c, "6" )
	PORT_DIPNAME( 0x10, 0x10, "First Pattern", IP_KEY_NONE )
	PORT_DIPSETTING(    0x10, "Easy" )
	PORT_DIPSETTING(    0x00, "Hard" )
	PORT_DIPNAME( 0x20, 0x20, "Teleport holes", IP_KEY_NONE )
	PORT_DIPSETTING(    0x20, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* Checked as a part of the protection */
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN ) /* Checked as a part of the protection */
INPUT_PORTS_END



static struct GfxLayout charlayout =
{
	8,8,    /* 8*8 characters */
	256,    /* 256 characters */
	2,      /* 2 bits per pixel */
	{ 0, 4},        /* the two bitplanes for 4 pixels are packed into one byte */
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 }, /* characters are rotated 90 degrees */
	{ 8*8+0, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 },     /* bits are packed in groups of four */
	16*8    /* every char takes 16 bytes */
};
static struct GfxLayout spritelayout =
{
	16,16,  /* 16*16 sprites */
	64,     /* 64 sprites */
	2,      /* 2 bits per pixel */
	{ 0, 4 },       /* the two bitplanes for 4 pixels are packed into one byte */
	{ 39*8, 38*8, 37*8, 36*8, 35*8, 34*8, 33*8, 32*8,
			7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
			24*8+0, 24*8+1, 24*8+2, 24*8+3, 0, 1, 2, 3 },
	64*8    /* every sprite takes 64 bytes */
};



static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x0000, &charlayout,   0, 32 },
	{ 1, 0x1000, &spritelayout, 0, 32 },
	{ -1 } /* end of array */
};


static unsigned char maketrax_color_prom[] =
{
	/* palette */
	0x00,0x07,0x66,0xEF,0x00,0xF8,0xEA,0x6F,0x00,0x3F,0x00,0xC9,0x38,0xAA,0xAF,0xF6,
	/* color lookup table */
	0x00,0x00,0x00,0x00,0x00,0x0f,0x0b,0x01,0x00,0x0f,0x0b,0x03,0x00,0x0f,0x0b,0x0f,
	0x00,0x0f,0x0b,0x07,0x00,0x0f,0x0b,0x05,0x00,0x0f,0x0b,0x0c,0x00,0x0f,0x0b,0x09,
	0x00,0x05,0x0b,0x07,0x00,0x0b,0x01,0x09,0x00,0x05,0x0b,0x01,0x00,0x02,0x05,0x01,
	0x00,0x02,0x0b,0x01,0x00,0x05,0x0b,0x09,0x00,0x0c,0x01,0x07,0x00,0x01,0x0c,0x0f,
	0x00,0x0f,0x00,0x0b,0x00,0x0c,0x05,0x0f,0x00,0x0f,0x0b,0x0e,0x00,0x0f,0x0b,0x0d,
	0x00,0x01,0x09,0x0f,0x00,0x09,0x0c,0x09,0x00,0x09,0x05,0x0f,0x00,0x05,0x0c,0x0f,
	0x00,0x01,0x07,0x0b,0x00,0x0f,0x0b,0x00,0x00,0x0f,0x00,0x0b,0x00,0x0b,0x05,0x09,
	0x00,0x0b,0x0c,0x0f,0x00,0x0b,0x07,0x09,0x00,0x02,0x0b,0x00,0x00,0x02,0x0b,0x07
};



/* waveforms for the audio hardware */
static unsigned char sound_prom[] =
{
	0x07,0x09,0x0A,0x0B,0x0C,0x0D,0x0D,0x0E,0x0E,0x0E,0x0D,0x0D,0x0C,0x0B,0x0A,0x09,
	0x07,0x05,0x04,0x03,0x02,0x01,0x01,0x00,0x00,0x00,0x01,0x01,0x02,0x03,0x04,0x05,
	0x07,0x0C,0x0E,0x0E,0x0D,0x0B,0x09,0x0A,0x0B,0x0B,0x0A,0x09,0x06,0x04,0x03,0x05,
	0x07,0x09,0x0B,0x0A,0x08,0x05,0x04,0x03,0x03,0x04,0x05,0x03,0x01,0x00,0x00,0x02,
	0x07,0x0A,0x0C,0x0D,0x0E,0x0D,0x0C,0x0A,0x07,0x04,0x02,0x01,0x00,0x01,0x02,0x04,
	0x07,0x0B,0x0D,0x0E,0x0D,0x0B,0x07,0x03,0x01,0x00,0x01,0x03,0x07,0x0E,0x07,0x00,
	0x07,0x0D,0x0B,0x08,0x0B,0x0D,0x09,0x06,0x0B,0x0E,0x0C,0x07,0x09,0x0A,0x06,0x02,
	0x07,0x0C,0x08,0x04,0x05,0x07,0x02,0x00,0x03,0x08,0x05,0x01,0x03,0x06,0x03,0x01,
	0x00,0x08,0x0F,0x07,0x01,0x08,0x0E,0x07,0x02,0x08,0x0D,0x07,0x03,0x08,0x0C,0x07,
	0x04,0x08,0x0B,0x07,0x05,0x08,0x0A,0x07,0x06,0x08,0x09,0x07,0x07,0x08,0x08,0x07,
	0x07,0x08,0x06,0x09,0x05,0x0A,0x04,0x0B,0x03,0x0C,0x02,0x0D,0x01,0x0E,0x00,0x0F,
	0x00,0x0F,0x01,0x0E,0x02,0x0D,0x03,0x0C,0x04,0x0B,0x05,0x0A,0x06,0x09,0x07,0x08,
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x0F,0x0E,0x0D,0x0C,0x0B,0x0A,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00,
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F
};



static struct MachineDriver maketrax_machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			3072000,        /* 3.072 Mhz */
			0,
			maketrax_readmem,maketrax_writemem,0,writeport,
			pacman_interrupt,1
		}
	},
	60,
	1,      /* single CPU, no need for interleaving */
	pacman_init_machine,

	/* video hardware */
	28*8, 36*8, { 0*8, 28*8-1, 0*8, 36*8-1 },
	gfxdecodeinfo,
	16,4*32,
	pengo_vh_convert_color_prom,

	VIDEO_TYPE_RASTER|VIDEO_SUPPORTS_DIRTY,
	0,
	pacman_vh_start,
	generic_vh_stop,
	pengo_vh_screenrefresh,

	/* sound hardware */
	0,
	pengo_sh_start,
	waveform_sh_stop,
	waveform_sh_update
};



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( maketrax_rom )
	ROM_REGION(0x10000)     /* 64k for code */
	ROM_LOAD( "maketrax.6e", 0x0000, 0x1000, 0xb25ba5a5 )
	ROM_LOAD( "maketrax.6f", 0x1000, 0x1000, 0xbe0c6060 )
	ROM_LOAD( "maketrax.6h", 0x2000, 0x1000, 0x59fca6a6 )
	ROM_LOAD( "maketrax.6j", 0x3000, 0x1000, 0xc5ad1d1d )

	ROM_REGION(0x2000)      /* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "maketrax.5e", 0x0000, 0x1000, 0xf4366ee0 )
	ROM_LOAD( "maketrax.5f", 0x1000, 0x1000, 0x4a383bfc )
ROM_END



struct GameDriver maketrax_driver =
{
	"Make Trax",
	"maketrax",
        "Allard van der Bas (original code)\nNicola Salmoria (MAME driver)\nGary Walton (color info)\nSimon Walls (color info)\nJohn Bowes(info)\nMike Balfour\nValerio Verrando (high score saving)",
	&maketrax_machine_driver,

	maketrax_rom,
	0, 0,
	0,
	sound_prom,     /* sound_prom */

	0/*TBR*/,maketrax_input_ports,0/*TBR*/,0/*TBR*/,0/*TBR*/,

	maketrax_color_prom, 0, 0,
	ORIENTATION_ROTATE_180,

	maketrax_hiload, maketrax_hisave /* hiload hisave */
};

/***************************************************************************


***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"



extern unsigned char *galaxian_attributesram;
extern unsigned char *galaxian_bulletsram;
extern int galaxian_bulletsram_size;
void galaxian_vh_convert_color_prom(unsigned char *palette, unsigned char *colortable,const unsigned char *color_prom);
void galaxian_attributes_w(int offset,int data);
void galaxian_stars_w(int offset,int data);
void mooncrst_gfxextend_w(int offset,int data);
int galaxian_vh_start(void);
void galaxian_vh_screenrefresh(struct osd_bitmap *bitmap);
int galaxian_vh_interrupt(void);

void mooncrst_sound_freq_w(int offset,int data);
void mooncrst_noise_w(int offset,int data);
void mooncrst_background_w(int offset,int data);
void mooncrst_shoot_w(int offset,int data);
void mooncrst_lfo_freq_w(int offset,int data);
void mooncrst_sound_freq_sel_w(int offset,int data);
int mooncrst_sh_init(const char *gamename);
int mooncrst_sh_start(void);
void mooncrst_sh_stop(void);
void mooncrst_sh_update(void);



static struct MemoryReadAddress readmem[] =
{
	{ 0x8000, 0x83ff, MRA_RAM },
	{ 0x9000, 0x9fff, MRA_RAM },	/* video RAM, screen attributes, sprites, bullets */
	{ 0x0000, 0x3fff, MRA_ROM },
	{ 0xa000, 0xa000, input_port_0_r },	/* IN0 */
	{ 0xa800, 0xa800, input_port_1_r },	/* IN1 */
	{ 0xb000, 0xb000, input_port_2_r },	/* DSW (coins per play) */
	{ 0xb800, 0xb800, MRA_NOP },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress writemem[] =
{
	{ 0x8000, 0x83ff, MWA_RAM },
	{ 0x9000, 0x93ff, videoram_w, &videoram, &videoram_size },
	{ 0x9800, 0x983f, galaxian_attributes_w, &galaxian_attributesram },
	{ 0x9840, 0x985f, MWA_RAM, &spriteram, &spriteram_size },
	{ 0x9860, 0x9880, MWA_RAM, &galaxian_bulletsram, &galaxian_bulletsram_size },
	{ 0xb000, 0xb000, interrupt_enable_w },
	{ 0xb800, 0xb800, mooncrst_sound_freq_w },
	{ 0xa800, 0xa800, mooncrst_background_w },
	{ 0xa803, 0xa803, mooncrst_noise_w },
	{ 0xa805, 0xa805, mooncrst_shoot_w },
	{ 0xa000, 0xa002, mooncrst_gfxextend_w },
	{ 0xa806, 0xa807, mooncrst_sound_freq_sel_w },
	{ 0xa004, 0xa007, mooncrst_lfo_freq_w },
	{ 0xb004, 0xb004, galaxian_stars_w },
	{ 0x0000, 0x3fff, MWA_ROM },
	{ -1 }	/* end of table */
};



static struct InputPort input_ports[] =
{
	{	/* IN0 */
		0x00,
		{ OSD_KEY_3, 0, OSD_KEY_LEFT, OSD_KEY_RIGHT,
				OSD_KEY_LCONTROL, 0, 0, 0 },
		{ 0, 0, OSD_JOY_LEFT, OSD_JOY_RIGHT,
				OSD_JOY_FIRE, 0, 0, 0 }
	},
	{	/* IN1 */
		0x80,
		{ OSD_KEY_1, OSD_KEY_2, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{	/* DSW */
		0x00,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{ -1 }	/* end of table */
};



static struct KEYSet keys[] =
{
        { 0, 2, "MOVE LEFT"  },
        { 0, 3, "MOVE RIGHT" },
        { 0, 4, "FIRE" },
        { -1 }
};


static struct DSW dsw[] =
{
	{ 1, 0x80, "LANGUAGE", { "JAPANESE", "ENGLISH" } },
	{ 1, 0x40, "BONUS", { "30000", "50000" } },
	{ -1 }
};



static struct GfxLayout charlayout =
{
	8,8,	/* 8*8 characters */
	512,	/* 512 characters */
	2,	/* 2 bits per pixel */
	{ 0, 512*8*8 },	/* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8	/* every char takes 8 consecutive bytes */
};
static struct GfxLayout spritelayout =
{
	16,16,	/* 16*16 sprites */
	128,	/* 128 sprites */
	2,	/* 2 bits per pixel */
	{ 0, 128*16*16 },	/* the two bitplanes are separated */
	{ 0, 1, 2, 3, 4, 5, 6, 7,
			8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 },
	32*8	/* every sprite takes 32 consecutive bytes */
};
static struct GfxLayout bulletlayout =
{
	/* there is no gfx ROM for this one, it is generated by the hardware */
	3,1,	/* 3*1 line */
	1,	/* just one */
	1,	/* 1 bit per pixel */
	{ 0 },
	{ 2, 2, 2 },	/* I "know" that this bit is 1 */
	{ 0 },	/* I "know" that this bit is 1 */
	0	/* no use */
};


static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x0000, &charlayout,     0, 8 },
	{ 1, 0x0000, &spritelayout,   0, 8 },
	{ 1, 0x0000, &bulletlayout, 8*4, 2 },
	{ -1 } /* end of array */
};



static unsigned char mooncrst_color_prom[] =
{
	/* palette */
	0x00,0x7a,0x36,0x07,0x00,0xf0,0x38,0x1f,0x00,0xc7,0xf0,0x3f,0x00,0xdb,0xc6,0x38,
	0x00,0x36,0x07,0xf0,0x00,0x33,0x3f,0xdb,0x00,0x3f,0x57,0xc6,0x00,0xc6,0x3f,0xff
};

/* this PROM was bad (bit 3 always set). I tried to "fix" it to get more reasonable */
/* colors, but it should not be considered correct. It's a bootleg anyway. */
static unsigned char fantazia_color_prom[] =
{
	/* palette */
	0x00,0x3B,0xCB,0xFE,0x00,0x1F,0xC0,0x3F,0x00,0xD8,0x07,0x3F,0x00,0xC0,0xCC,0x07,
	0x00,0xC0,0xB8,0x1F,0x00,0x1E,0x79,0x0F,0x00,0xFE,0x07,0xF8,0x00,0x7E,0x07,0xC6
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
			galaxian_vh_interrupt,1
		}
	},
	60,
	1,	/* single CPU, no need for interleaving */
	0,

	/* video hardware */
	32*8, 32*8, { 0*8, 32*8-1, 2*8, 30*8-1 },
	gfxdecodeinfo,
	32+64,8*4+2*2,	/* 32 for the characters, 64 for the stars */
	galaxian_vh_convert_color_prom,

	VIDEO_TYPE_RASTER,
	0,
	galaxian_vh_start,
	generic_vh_stop,
	galaxian_vh_screenrefresh,

	/* sound hardware */
	mooncrst_sh_init,
	mooncrst_sh_start,
	mooncrst_sh_stop,
	mooncrst_sh_update
};



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( mooncrst_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "mc1", 0x0000, 0x0800, 0x09517d4d )
	ROM_LOAD( "mc2", 0x0800, 0x0800, 0xc38036ea )
	ROM_LOAD( "mc3", 0x1000, 0x0800, 0x55850d07 )
	ROM_LOAD( "mc4", 0x1800, 0x0800, 0xce9fa607 )
	ROM_LOAD( "mc5", 0x2000, 0x0800, 0xbe597a3b )
	ROM_LOAD( "mc6", 0x2800, 0x0800, 0xccf35bef )
	ROM_LOAD( "mc7", 0x3000, 0x0800, 0x589bfa8f )
	ROM_LOAD( "mc8", 0x3800, 0x0800, 0xc2ca7a86 )

	ROM_REGION(0x2000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "mcb", 0x0000, 0x0800, 0x22bd9067 )
	ROM_LOAD( "mcd", 0x0800, 0x0200, 0xdfbc68ba )
	ROM_CONTINUE(    0x0c00, 0x0200 )	/* this version of the gfx ROMs has two */
	ROM_CONTINUE(    0x0a00, 0x0200 )	/* groups of 16 sprites swapped */
	ROM_CONTINUE(    0x0e00, 0x0200 )
	ROM_LOAD( "mca", 0x1000, 0x0800, 0xf93f9153 )
	ROM_LOAD( "mcc", 0x1800, 0x0200, 0xc1dc1cde )
	ROM_CONTINUE(    0x1c00, 0x0200 )
	ROM_CONTINUE(    0x1a00, 0x0200 )
	ROM_CONTINUE(    0x1e00, 0x0200 )
ROM_END

ROM_START( mooncrsg_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "EPR194", 0x0000, 0x0800, 0x719fde2d )
	ROM_LOAD( "EPR195", 0x0800, 0x0800, 0xb592b35a )
	ROM_LOAD( "EPR196", 0x1000, 0x0800, 0xa68c9f7e )
	ROM_LOAD( "EPR197", 0x1800, 0x0800, 0xdd96a2c2 )
	ROM_LOAD( "EPR198", 0x2000, 0x0800, 0xb3df4fd5 )
	ROM_LOAD( "EPR199", 0x2800, 0x0800, 0x4b7654e0 )
	ROM_LOAD( "EPR200", 0x3000, 0x0800, 0x765799c9 )
	ROM_LOAD( "EPR201", 0x3800, 0x0800, 0xb1cd92a3 )

	ROM_REGION(0x2000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "EPR203", 0x0000, 0x0800, 0xa27f5447 )
	ROM_LOAD( "EPR172", 0x0800, 0x0800, 0xdfbc68ba )
	ROM_LOAD( "EPR202", 0x1000, 0x0800, 0xec79cbdb )
	ROM_LOAD( "EPR171", 0x1800, 0x0800, 0xc1dc1cde )
ROM_END

ROM_START( mooncrsb_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "EPR194", 0x0000, 0x0800, 0x1c6e3b4a )
	ROM_LOAD( "EPR195", 0x0800, 0x0800, 0xa8901964 )
	ROM_LOAD( "EPR196", 0x1000, 0x0800, 0x3247a543 )
	ROM_LOAD( "EPR197", 0x1800, 0x0800, 0x8e22a4b2 )
	ROM_LOAD( "EPR198", 0x2000, 0x0800, 0x981d7a7f )
	ROM_LOAD( "EPR199", 0x2800, 0x0800, 0x3def1bab )
	ROM_LOAD( "EPR200", 0x3000, 0x0800, 0xb31bfacf )
	ROM_LOAD( "EPR201", 0x3800, 0x0800, 0xe0a15117 )

	ROM_REGION(0x2000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "EPR203", 0x0000, 0x0800, 0xa27f5447 )
	ROM_LOAD( "EPR172", 0x0800, 0x0800, 0xdfbc68ba )
	ROM_LOAD( "EPR202", 0x1000, 0x0800, 0xec79cbdb )
	ROM_LOAD( "EPR171", 0x1800, 0x0800, 0xc1dc1cde )
ROM_END

ROM_START( fantazia_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "F01.bin", 0x0000, 0x0800, 0x22693d4d )
	ROM_LOAD( "F02.bin", 0x0800, 0x0800, 0xe65a30ae )
	ROM_LOAD( "F03.bin", 0x1000, 0x0800, 0x3247a543 )
	ROM_LOAD( "F04.bin", 0x1800, 0x0800, 0x8e22a4b2 )
	ROM_LOAD( "F09.bin", 0x2000, 0x0800, 0x730c6f5a )
	ROM_LOAD( "F10.bin", 0x2800, 0x0800, 0x50694b53 )
	ROM_LOAD( "F11.bin", 0x3000, 0x0800, 0x932f1ab9 )
	ROM_LOAD( "F12.bin", 0x3800, 0x0800, 0xdc786302 )

	ROM_REGION(0x2000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "1h_1_10.bin", 0x0000, 0x0800, 0x22bd9067 )
	ROM_LOAD( "1k_2_12.bin", 0x0800, 0x0800, 0xdfbc68ba )
	ROM_LOAD( "1k_1_11.bin", 0x1000, 0x0800, 0xf93f9153 )
	ROM_LOAD( "1h_2_09.bin", 0x1800, 0x0800, 0xc1dc1cde )
ROM_END



static const char *mooncrst_sample_names[] =
{
	"*galaxian",
	"shot.sam",
	"death.sam",
	0	/* end of array */
};



static void moonqsr_decode(void)
{
int A;


for (A = 0;A < 0x10000;A++)
{
	static unsigned char evetab[] =
	{
		0x00,0x01,0x06,0x07,0x40,0x41,0x46,0x47,0x08,0x09,0x0e,0x0f,0x48,0x49,0x4e,0x4f,
		0x10,0x11,0x16,0x17,0x50,0x51,0x56,0x57,0x18,0x19,0x1e,0x1f,0x58,0x59,0x5e,0x5f,
		0x60,0x61,0x66,0x67,0x20,0x21,0x26,0x27,0x68,0x69,0x6e,0x6f,0x28,0x29,0x2e,0x2f,
		0x70,0x71,0x76,0x77,0x30,0x31,0x36,0x37,0x78,0x79,0x7e,0x7f,0x38,0x39,0x3e,0x3f,
		0x04,0x05,0x02,0x03,0x44,0x45,0x42,0x43,0x0c,0x0d,0x0a,0x0b,0x4c,0x4d,0x4a,0x4b,
		0x14,0x15,0x12,0x13,0x54,0x55,0x52,0x53,0x1c,0x1d,0x1a,0x1b,0x5c,0x5d,0x5a,0x5b,
		0x64,0x65,0x62,0x63,0x24,0x25,0x22,0x23,0x6c,0x6d,0x6a,0x6b,0x2c,0x2d,0x2a,0x2b,
		0x74,0x75,0x72,0x73,0x34,0x35,0x32,0x33,0x7c,0x7d,0x7a,0x7b,0x3c,0x3d,0x3a,0x3b,
		0x80,0x81,0x86,0x87,0xc0,0xc1,0xc6,0xc7,0x88,0x89,0x8e,0x8f,0xc8,0xc9,0xce,0xcf,
		0x90,0x91,0x96,0x97,0xd0,0xd1,0xd6,0xd7,0x98,0x99,0x9e,0x9f,0xd8,0xd9,0xde,0xdf,
		0xe0,0xe1,0xe6,0xe7,0xa0,0xa1,0xa6,0xa7,0xe8,0xe9,0xee,0xef,0xa8,0xa9,0xae,0xaf,
		0xf0,0xf1,0xf6,0xf7,0xb0,0xb1,0xb6,0xb7,0xf8,0xf9,0xfe,0xff,0xb8,0xb9,0xbe,0xbf,
		0x84,0x85,0x82,0x83,0xc4,0xc5,0xc2,0xc3,0x8c,0x8d,0x8a,0x8b,0xcc,0xcd,0xca,0xcb,
		0x94,0x95,0x92,0x93,0xd4,0xd5,0xd2,0xd3,0x9c,0x9d,0x9a,0x9b,0xdc,0xdd,0xda,0xdb,
		0xe4,0xe5,0xe2,0xe3,0xa4,0xa5,0xa2,0xa3,0xec,0xed,0xea,0xeb,0xac,0xad,0xaa,0xab,
		0xf4,0xf5,0xf2,0xf3,0xb4,0xb5,0xb2,0xb3,0xfc,0xfd,0xfa,0xfb,0xbc,0xbd,0xba,0xbb
	};
	static unsigned char oddtab[] =
	{
		0x00,0x01,0x42,0x43,0x04,0x05,0x46,0x47,0x08,0x09,0x4a,0x4b,0x0c,0x0d,0x4e,0x4f,
		0x10,0x11,0x52,0x53,0x14,0x15,0x56,0x57,0x18,0x19,0x5a,0x5b,0x1c,0x1d,0x5e,0x5f,
		0x24,0x25,0x66,0x67,0x20,0x21,0x62,0x63,0x2c,0x2d,0x6e,0x6f,0x28,0x29,0x6a,0x6b,
		0x34,0x35,0x76,0x77,0x30,0x31,0x72,0x73,0x3c,0x3d,0x7e,0x7f,0x38,0x39,0x7a,0x7b,
		0x40,0x41,0x02,0x03,0x44,0x45,0x06,0x07,0x48,0x49,0x0a,0x0b,0x4c,0x4d,0x0e,0x0f,
		0x50,0x51,0x12,0x13,0x54,0x55,0x16,0x17,0x58,0x59,0x1a,0x1b,0x5c,0x5d,0x1e,0x1f,
		0x64,0x65,0x26,0x27,0x60,0x61,0x22,0x23,0x6c,0x6d,0x2e,0x2f,0x68,0x69,0x2a,0x2b,
		0x74,0x75,0x36,0x37,0x70,0x71,0x32,0x33,0x7c,0x7d,0x3e,0x3f,0x78,0x79,0x3a,0x3b,
		0x80,0x81,0xc2,0xc3,0x84,0x85,0xc6,0xc7,0x88,0x89,0xca,0xcb,0x8c,0x8d,0xce,0xcf,
		0x90,0x91,0xd2,0xd3,0x94,0x95,0xd6,0xd7,0x98,0x99,0xda,0xdb,0x9c,0x9d,0xde,0xdf,
		0xa4,0xa5,0xe6,0xe7,0xa0,0xa1,0xe2,0xe3,0xac,0xad,0xee,0xef,0xa8,0xa9,0xea,0xeb,
		0xb4,0xb5,0xf6,0xf7,0xb0,0xb1,0xf2,0xf3,0xbc,0xbd,0xfe,0xff,0xb8,0xb9,0xfa,0xfb,
		0xc0,0xc1,0x82,0x83,0xc4,0xc5,0x86,0x87,0xc8,0xc9,0x8a,0x8b,0xcc,0xcd,0x8e,0x8f,
		0xd0,0xd1,0x92,0x93,0xd4,0xd5,0x96,0x97,0xd8,0xd9,0x9a,0x9b,0xdc,0xdd,0x9e,0x9f,
		0xe4,0xe5,0xa6,0xa7,0xe0,0xe1,0xa2,0xa3,0xec,0xed,0xae,0xaf,0xe8,0xe9,0xaa,0xab,
		0xf4,0xf5,0xb6,0xb7,0xf0,0xf1,0xb2,0xb3,0xfc,0xfd,0xbe,0xbf,0xf8,0xf9,0xba,0xbb
	};


	if (A & 1) RAM[A] = oddtab[RAM[A]];
	else RAM[A] = evetab[RAM[A]];
}
}



static int hiload(void)
{
	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0x8042],"\x00\x50\x00",3) == 0 &&
			memcmp(&RAM[0x804e],"\x00\x50\x00",3) == 0)
	{
		void *f;


		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x8042],17*5);
			osd_fclose(f);
		}

		return 1;
	}
	else return 0;	/* we can't load the hi scores yet */
}

static void hisave(void)
{
	void *f;


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x8042],17*5);
		osd_fclose(f);
	}
}

static int mooncrsg_hiload(void)
{
	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0x8045],"\x00\x50\x00",3) == 0 &&
			memcmp(&RAM[0x8051],"\x00\x50\x00",3) == 0)
	{
		void *f;


		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x8045],17*5);
			osd_fclose(f);
		}

		return 1;
	}
	else return 0;	/* we can't load the hi scores yet */
}

static void mooncrsg_hisave(void)
{
	void *f;


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x8045],17*5);
		osd_fclose(f);
	}
}



struct GameDriver mooncrst_driver =
{
	"Moon Cresta (Nichibutsu)",
	"mooncrst",
	"ROBERT ANSCHUETZ\nNICOLA SALMORIA\nGARY WALTON\nSIMON WALLS\nANDREW SCOTT",
	&machine_driver,

	mooncrst_rom,
	moonqsr_decode, 0,
	mooncrst_sample_names,
	0,	/* sound_prom */

	input_ports, 0, 0/*TBR*/,dsw, keys,

	mooncrst_color_prom, 0, 0,
	ORIENTATION_ROTATE_90,

	hiload, hisave
};

struct GameDriver mooncrsg_driver =
{
	"Moon Cresta (Gremlin)",
	"mooncrsg",
	"ROBERT ANSCHUETZ\nNICOLA SALMORIA\nGARY WALTON\nSIMON WALLS\nANDREW SCOTT",
	&machine_driver,

	mooncrsg_rom,
	0, 0,
	mooncrst_sample_names,
	0,	/* sound_prom */

	input_ports, 0, 0/*TBR*/,dsw, keys,

	mooncrst_color_prom, 0, 0,
	ORIENTATION_ROTATE_90,

	mooncrsg_hiload, mooncrsg_hisave
};

struct GameDriver mooncrsb_driver =
{
	"Moon Cresta (bootleg)",
	"mooncrsb",
	"ROBERT ANSCHUETZ\nNICOLA SALMORIA\nGARY WALTON\nSIMON WALLS\nANDREW SCOTT",
	&machine_driver,

	mooncrsb_rom,
	0, 0,
	mooncrst_sample_names,
	0,	/* sound_prom */

	input_ports, 0, 0/*TBR*/,dsw, keys,

	mooncrst_color_prom, 0, 0,
	ORIENTATION_ROTATE_90,

	hiload, hisave
};

struct GameDriver fantazia_driver =
{
	"Fantazia",
	"fantazia",
	"ROBERT ANSCHUETZ\nNICOLA SALMORIA\nGARY WALTON\nSIMON WALLS\nANDREW SCOTT",
	&machine_driver,

	fantazia_rom,
	0, 0,
	mooncrst_sample_names,
	0,	/* sound_prom */

	input_ports, 0, 0/*TBR*/,dsw, keys,

	fantazia_color_prom, 0, 0,
	ORIENTATION_ROTATE_90,

	hiload, hisave
};

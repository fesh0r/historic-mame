/***************************************************************************

Loco-Motion memory map (preliminary)

CPU #1
0000-4fff ROM (empty space at 5000-5fff for diagnostic ROM)
8040-83ff video RAM (top 8 rows of screen)
8440-87ff video RAM
8840-8bff color RAM (top 8 rows of screen)
8c40-8fff color RAM
8000-803f sprite RAM #1
8800-883f sprite RAM #2
9800-9fff RAM

read:
a000      IN0
          bit 7 = ?
          bit 6 = ?
          bit 5 = RIGHT player 1
          bit 4 = LEFT player 1
          bit 3 = FIRE player 1
          bit 2 = COIN
          bit 1 = ?
          bit 0 = DOWN player 2 (COCKTAIL only)
a080      IN1
          bit 7 = START 1
          bit 6 = START 2
          bit 5 = RIGHT player 2 (COCKTAIL only)
          bit 4 = LEFT player 2 (COCKTAIL only)
          bit 3 = FIRE player 2 (COCKTAIL only)
          bit 2 = ?
          bit 1 = UP player 2 (COCKTAIL only)
          bit 0 = UP player 1
a100      IN2
          bit 7 = DOWN player 1
          bit 6 = ?
          bit 4-5 = lives
          bit 3 = UPRIGHT or COCKTAIL select (0 = UPRIGHT)
          bit 2 = ?
          bit 1 = ?
          bit 0 = attract mode sounds
a180      DSW
          bits 0-3 coins per play
          bits 4-7 ?

write:
a080      watchdog reset
a100      command for the audio CPU
a180      interrupt trigger on audio CPU
a181      interrupt enable


CPU #2:
2000-23ff RAM

read:
4000      8910 #0 read
6000      8910 #1 read

write:
5000      8910 #0 control
4000      8910 #0 write
7000      8910 #1 control
6000      8910 #1 write

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "sndhrdw/generic.h"
#include "sndhrdw/8910intf.h"



void locomotn_vh_screenrefresh(struct osd_bitmap *bitmap);

void locomotn_sh_irqtrigger_w(int offset,int data);
int locomotn_sh_start(void);



static struct MemoryReadAddress readmem[] =
{
	{ 0x9800, 0x9fff, MRA_RAM },
	{ 0x0000, 0x5fff, MRA_ROM },
	{ 0xa000, 0xa000, input_port_0_r },	/* IN0 */
	{ 0xa080, 0xa080, input_port_1_r },	/* IN1 */
	{ 0xa100, 0xa100, input_port_2_r },	/* IN2 */
	{ 0xa180, 0xa180, input_port_3_r },	/* DSW */
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress writemem[] =
{
	{ 0x9800, 0x9fff, MWA_RAM },
	{ 0x8040, 0x87ff, videoram_w, &videoram, &videoram_size },
	{ 0x8840, 0x8fff, colorram_w, &colorram },
	{ 0x8800, 0x883f, MWA_RAM, &spriteram, &spriteram_size },
	{ 0x8000, 0x803f, MWA_RAM, &spriteram_2 },
	{ 0xa080, 0xa080, MWA_NOP },
	{ 0xa181, 0xa181, interrupt_enable_w },
	{ 0x0000, 0x5fff, MWA_ROM },
	{ 0xa100, 0xa100, soundlatch_w },
	{ 0xa180, 0xa180, locomotn_sh_irqtrigger_w },
	{ -1 }	/* end of table */
};



static struct MemoryReadAddress sound_readmem[] =
{
	{ 0x2000, 0x23ff, MRA_RAM },
	{ 0x4000, 0x4000, AY8910_read_port_0_r },
	{ 0x6000, 0x6000, AY8910_read_port_1_r },
	{ 0x0000, 0x0fff, MRA_ROM },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress sound_writemem[] =
{
	{ 0x2000, 0x23ff, MWA_RAM },
	{ 0x5000, 0x5000, AY8910_control_port_0_w },
	{ 0x4000, 0x4000, AY8910_write_port_0_w },
	{ 0x7000, 0x7000, AY8910_control_port_1_w },
	{ 0x6000, 0x6000, AY8910_write_port_1_w },
	{ 0x0000, 0x0fff, MWA_ROM },
	{ -1 }	/* end of table */
};



static struct InputPort input_ports[] =
{
	{	/* IN0 */
		0xff,
		{ 0, 0, OSD_KEY_3, OSD_KEY_LCONTROL, OSD_KEY_LEFT, OSD_KEY_RIGHT, 0, 0 },
		{ 0, 0, 0, OSD_JOY_FIRE, OSD_JOY_LEFT, OSD_JOY_RIGHT, 0, 0 }
	},
	{	/* IN1 */
		0xff,
		{ OSD_KEY_UP, 0, 0, 0, 0, 0, OSD_KEY_2, OSD_KEY_1 },
		{ OSD_JOY_UP, 0, 0, 0, 0, 0, 0, 0 }
	},
	{	/* IN2 */
		0xf6,
		{ 0, 0, 0, 0, 0, 0, 0, OSD_KEY_DOWN },
		{ 0, 0, 0, 0, 0, 0, 0, OSD_JOY_DOWN }
	},
	{	/* DSW */
		0xff,
		{ 0, 0, 0, 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0, 0, 0 }
	},
	{ -1 }	/* end of table */
};



static struct KEYSet keys[] =
{
        { 1, 0, "MOVE UP" },
        { 0, 4, "MOVE LEFT" },
        { 0, 5, "MOVE RIGHT" },
        { 2, 7, "MOVE DOWN" },
        { 0, 3, "ACCELERATE" },
        { -1 }
};


static struct DSW dsw[] =
{
	{ 2, 0x30, "LIVES", { "255?", "5", "4", "3" }, 1 },
	{ 2, 0x01, "DEMO SOUNDS", { "ON", "OFF" }, 1 },
	{ 2, 0x02, "SWA1", { "0", "1" } },
	{ 2, 0x04, "SWA2", { "0", "1" } },
	{ 2, 0x40, "SWA6", { "0", "1" } },
	{ 3, 0x10, "SWB4", { "0", "1" } },
	{ 3, 0x20, "SWB5", { "0", "1" } },
	{ 3, 0x40, "SWB6", { "0", "1" } },
	{ 3, 0x80, "SWB7", { "0", "1" } },
	{ -1 }
};



static struct GfxLayout charlayout =
{
	8,8,	/* 8*8 characters */
	512,	/* 512 characters */
	2,	/* 2 bits per pixel */
	{ 4, 0 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	{ 8*8+0,8*8+1,8*8+2,8*8+3, 0, 1, 2, 3 },
	16*8	/* every char takes 16 consecutive bytes */
};
static struct GfxLayout spritelayout =
{
	16,16,	/* 16*16 sprites */
	64,	/* 64 sprites */
	2,	/* 2 bits per pixel */
	{ 4, 0 },
	{ 39*8, 38*8, 37*8, 36*8, 35*8, 34*8, 33*8, 32*8,
			7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3,
			24*8+0, 24*8+1, 24*8+2, 24*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3 },
	64*8	/* every sprite takes 64 consecutive bytes */
};



static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x0000, &charlayout,   0, 32 },
	{ 1, 0x1000, &spritelayout, 0, 32 },
	{ -1 } /* end of array */
};



static unsigned char palette[] =
{
	0x00,0x00,0x00,	/*  0 - BLACK */
	241,0,0, 		/*  1 - RED */
	0x00,0xff,0x00, /*  2 - GREEN */
	0x00,0x00,0xff, /*  3 - BLUE */
	0xff,0xff,0x00, /*  4 - YELLOW 				*/
	0xff,0x00,0xff, /*  5 - MAGENTA */
	0x00,0xff,0xff, /*  6 - CYAN */
	0xff,0xff,0xff, /*  7 - WHITE */
	0xE0,0xE0,0xE0, /*  8 - LTGRAY 				*/
	0xC0,0xC0,0xC0, /*  9 - DKGRAY 				*/
	0,146,214,		/* 10 - CELSETE 1			*/
	0xd0,0xa0,0x60,	/* 11 - BROWN0 */
	0xc0,0x90,0x50,	/* 12 - BROWN1 */
	179,179,93,		/* 13 - MARRON 2			*/
	206,157,92,		/* 14 - MARRON 1			*/
	161,117,56,		/* 15 - MARRON 3			*/
	35,49,19, 		/* 16 - CELESTE 3 */
	0x00,0xa0,0x00, /* 17 - DKGREEN */
	0x00,0xe0,0x00, /* 18 - GRASSGREEN */
	0,134,0,		/* 19 - VERDE 1 			*/
	0x49,0xb6,0xdb,	/* 20 - DKCYAN */
	0xff,96,0x49,	/* 21 - DKORANGE */
	130,205,253,	/* 22 - CELESTE 2 			*/
	0xdb,0xdb,0xdb	/* 23 - GREY */
};


static unsigned char colortable[] =
{

	0,22,4,15,	/* Estacion				*/
	0,1,10,13,	/* Tren					*/
	0,4,0,1,	/* Tren Loco			*/
	0,4,5,6,
	0,1,8,1,	/* Loop Sweeper 2		*/
	0,2,2,2,
	0,3,3,3,
	0,8,9,7,	/* Loop Sweeper 1		*/


	0,6,7,6,	/* Cartel cuenta regr.	*/
	0,6,1,6,
	0,6,3,6,
	0,6,3,6,
	0,6,1,6,	/* Cartel de puntaje	*/
	0,6,1,6,
	0,6,4,6,	/* Cartel Deposit Coin	*/
	0,6,3,6,

	0,5,19,5,	/* Cartel de Hi Score 1	*/
	0,5,19,5,	/* Cartel de Hi Score 2	*/
	0,5,19,5,	/* Cartel de Hi Score 3	*/
	0,5,22,5,	/* Cartel Loco Motion	*/
	0,5,2,5,
	0,5,3,5,
	0,5,7,5,
	0,5,21,5,

	0,1,2,3,
	0,1,2,3,
	0,1,7,3,		/* Cartel 1UP High	*/
	0,1,2,3,
	0,10,14,4,		/* Curva BD			*/
	0,22,14,4,		/* Curva AC			*/
	0,19,14,4,		/* Cruce de via		*/
	0,4,5,6
};



static struct MachineDriver machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			3072000,	/* 3.072 Mhz (?) */
			0,
			readmem,writemem,0,0,
			nmi_interrupt,1
		},
		{
			CPU_Z80 | CPU_AUDIO_CPU,
			3072000,	/* 3.072 Mhz (?) */
			2,	/* memory region #2 */
			sound_readmem,sound_writemem,0,0,
			ignore_interrupt,1	/* interrupts are triggered by the main CPU */
		}
	},
	60,
	10,	/* 10 CPU slices per frame - enough for the sound CPU to read all commands */
	0,

	/* video hardware */
	28*8, 36*8, { 0*8, 28*8-1, 0*8, 36*8-1 },
	gfxdecodeinfo,
	sizeof(palette)/3,sizeof(colortable),
	0,

	VIDEO_TYPE_RASTER|VIDEO_SUPPORTS_DIRTY,
	0,
	generic_vh_start,
	generic_vh_stop,
	locomotn_vh_screenrefresh,

	/* sound hardware */
	0,
	locomotn_sh_start,
	AY8910_sh_stop,
	AY8910_sh_update
};



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( locomotn_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "1a.cpu", 0x0000, 0x1000, 0x35669192 )
	ROM_LOAD( "2a.cpu", 0x1000, 0x1000, 0x868e21f8 )
	ROM_LOAD( "3.cpu",  0x2000, 0x1000, 0xff78e538 )
	ROM_LOAD( "4.cpu",  0x3000, 0x1000, 0x549b7a73 )
	ROM_LOAD( "5.cpu",  0x4000, 0x1000, 0x75b145dd )

	ROM_REGION(0x2000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "c1.cpu", 0x0000, 0x1000, 0x3a668900 )
	ROM_LOAD( "c2.cpu", 0x1000, 0x1000, 0xfd68fc8c )

	ROM_REGION(0x10000)	/* 64k for the audio CPU */
	ROM_LOAD( "s1.snd", 0x0000, 0x1000, 0xf8aff0dd )
ROM_END



static int hiload(void)
{
	/* get RAM pointer (this game is multiCPU, we can't assume the global */
	/* RAM pointer is pointing to the right place) */
	unsigned char *RAM = Machine->memory_region[0];
	static unsigned int ROMloaded=0;

	/* read the top score into ROM to fix display at top of screen */
	/* (Does anybody have a cleaner way to do this?) */

	if (!ROMloaded)
	{
		void *f;

		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
                        osd_fread(f,&RAM[0x0931],3);
			osd_fclose(f);
		}
		ROMloaded=1;
	}

	/* check if the hi score table has already been initialized */
        if ((memcmp(&RAM[0x9F00],&RAM[0x0931],3) == 0) &&
		(memcmp(&RAM[0x9F75],"\x3E\x3E\x3E",3) == 0))
	{
		void *f;


		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
                        osd_fread(f,&RAM[0x9F00],12*10);
			osd_fclose(f);
		}

		return 1;
	}
	else return 0;	/* we can't load the hi scores yet */
}



static void hisave(void)
{
	void *f;

	/* get RAM pointer (this game is multiCPU, we can't assume the global */
	/* RAM pointer is pointing to the right place) */
	unsigned char *RAM = Machine->memory_region[0];


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
                osd_fwrite(f,&RAM[0x9F00],12*10);
		osd_fclose(f);
	}

}



struct GameDriver locomotn_driver =
{
	"Loco-Motion",
	"locomotn",
	"Nicola Salmoria\nLawnmower Man\nMike Balfour (high score save)\nGonzalo Casas (colors)",
	&machine_driver,

	locomotn_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	input_ports, 0, 0/*TBR*/,dsw, keys,

	0, palette, colortable,
	ORIENTATION_DEFAULT,

	hiload, hisave
};

/***************************************************************************

Qix Memory Map
--- ------ ---

Qix uses two 6809 CPUs:  one for data and sound and the other for video.
Communication between the two CPUs is done using a 4K RAM space at $8000
which both CPUs have direct access.  FIRQs (fast interrupts) are generated
by each CPU to interrupt the other at specific times.

A third CPU, a 6802, is used for sample playback.  It drives an 8-bit
DAC and according to the schematics a TMS5220 speech chip, which is never
accessed.  ROM u27 is the only code needed.  A sound command from the
data CPU causes an IRQ to fire on the 6802 and the sound playback is
started.

The coin door switches and player controls are connected to the CPUs by
Mototola 6821 PIAs.  These devices are memory mapped as shown below.

The screen is 256x256 with eight bit pixels (64K).  The screen is divided
into two halves each half mapped by the video CPU at $0000-$7FFF.  The
high order bit of the address latch at $9402 specifies which half of the
screen is being accessed.

Timing is critical in the hardware.  The data CPU must have an interrupt
signal generated externally at the right frequency to make the game play
correctly.

The address latch works as follows.  When the video CPU accesses $9400,
the screen address is computed by using the values at $9402 (high byte)
and $9403 (low byte) to get a value between $0000-$FFFF.  The value at
that location is either returned or written.

The scan line at $9800 on the video CPU records where the scan line is
on the display (0-255).  Several places in the ROM code wait until the
scan line reaches zero before continuing.

CPU #1 (Data/Sound):
    $8400 - $87FF:  Local Memory
    $8800        :  ACIA base address
    $8C00        :  Video FIRQ activation
    $8C01        :  Data FIRQ deactivation
    $9000        :  Sound PIA
    $9400        :  [76543210] Game PIA 1 (Port A)
                     o         Fast draw
                      o        1P button
                       o       2P button
                        o      Slow draw
                         o     Joystick Left
                          o    Joystick Down
                           o   Joystick Right
                            o  Joystick Up
    $9402        :  [76543210] Game PIA 1 (Port B)
                     o         Tilt
                      o        Coin sw      Unknown
                       o       Right coin
                        o      Left coin
                         o     Slew down
                          o    Slew up
                           o   Sub. test
                            o  Adv. test
    $9900        :  Game PIA 2
    $9C00        :  Game PIA 3

CPU #2 (Video):
    $0000 - $7FFF:  Video/Screen RAM
    $8400 - $87FF:  CMOS backup and local memory
    $8800        :  LED output and color RAM page select
    $8C00        :  Data FIRQ activation
    $8C01        :  Video FIRQ deactivation
    $9000        :  Color RAM
    $9400        :  Address latch screen location
    $9402        :  Address latch Hi-byte
    $9403        :  Address latch Lo-byte
    $9800        :  Scan line location
    $9C00        :  CRT controller base address

BOTH CPUS:
    $8000 - $83FF:  Dual port RAM accessible by both processors

NONVOLATILE CMOS MEMORY MAP (CPU #2 -- Video) $8400-$87ff
	$86A9 - $86AA:	When CMOS is valid, these bytes are $55AA
	$86AC - $86C3:	AUDIT TOTALS -- 4 4-bit BCD digits per setting
					(All totals default to: 0000)
					$86AC: TOTAL PAID CREDITS
					$86AE: LEFT COINS
					$86B0: CENTER COINS
					$86B2: RIGHT COINS
					$86B4: PAID CREDITS
					$86B6: AWARDED CREDITS
					$86B8: % FREE PLAYS
					$86BA: MINUTES PLAYED
					$86BC: MINUTES AWARDED
					$86BE: % FREE TIME
					$86C0: AVG. GAME [SEC]
					$86C2: HIGH SCORES
	$86C4 - $86FF:	High scores -- 10 scores/names, consecutive in memory
					Six 4-bit BCD digits followed by 3 ascii bytes
					(Default: 030000 QIX)
	$8700		 :	LANGUAGE SELECT (Default: $32)
					ENGLISH = $32  FRANCAIS = $33  ESPANOL = $34  DEUTSCH = $35
	$87D9 - $87DF:	COIN SLOT PROGRAMMING -- 2 4-bit BCD digits per setting
					$87D9: STANDARD COINAGE SETTING  (Default: 01)
					$87DA: COIN MULTIPLIERS LEFT (Default: 01)
					$87DB: COIN MULTIPLIERS CENTER (Default: 04)
					$87DC: COIN MULTIPLIERS RIGHT (Default: 01)
					$87DD: COIN UNITS FOR CREDIT (Default: 01)
					$87DE: COIN UNITS FOR BONUS (Default: 00)
					$87DF: MINIMUM COINS (Default: 00)
	$87E0 - $87EA:	LOCATION PROGRAMMING -- 2 4-bit BCD digits per setting
					$87E0: BACKUP HSTD [0000] (Default: 03)
					$87E1: MAXIMUM CREDITS (Default: 10)
					$87E2: NUMBER OF TURNS (Default: 03)
					$87E3: THRESHOLD (Default: 75)
					$87E4: TIME LINE (Default: 37)
					$87E5: DIFFICULTY 1 (Default: 01)
					$87E6: DIFFICULTY 2 (Default: 01)
					$87E7: DIFFICULTY 3 (Default: 01)
					$87E8: DIFFICULTY 4 (Default: 01)
					$87E9: ATTRACT SOUND (Default: 01)
					$87EA: TABLE MODE (Default: 00)

***************************************************************************/

#include "driver.h"
#include "machine/6821pia.h"
#include "sndhrdw/dac.h"

extern unsigned char *qix_sharedram;
int qix_scanline_r(int offset);
void qix_data_firq_w(int offset, int data);
void qix_video_firq_w(int offset, int data);


extern unsigned char *qix_paletteram,*qix_palettebank;
extern unsigned char *qix_videoaddress;

int qix_videoram_r(int offset);
void qix_videoram_w(int offset, int data);
int qix_addresslatch_r(int offset);
void qix_addresslatch_w(int offset, int data);
void qix_paletteram_w(int offset,int data);
void qix_palettebank_w(int offset,int data);

int qix_sharedram_r_1(int offset);
int qix_sharedram_r_2(int offset);
void qix_sharedram_w(int offset, int data);
int qix_interrupt_video(void);
void qix_vh_convert_color_prom(unsigned char *palette, unsigned char *colortable,const unsigned char *color_prom);
void qix_vh_screenrefresh(struct osd_bitmap *bitmap);
int qix_vh_start(void);
void qix_vh_stop(void);
void qix_init_machine(void);

int qix_data_io_r (int offset);
int qix_sound_io_r (int offset);
void qix_data_io_w (int offset, int data);
void qix_sound_io_w (int offset, int data);
int qix_sh_start (void);
void qix_sh_stop (void);
void qix_sh_update (void);


static struct MemoryReadAddress readmem_cpu_data[] =
{
	{ 0x8000, 0x83ff, qix_sharedram_r_1, &qix_sharedram },
	{ 0x8400, 0x87ff, MRA_RAM },
	{ 0x9000, 0x9003, pia_4_r },
	{ 0x9400, 0x9403, pia_1_r },
	{ 0x9900, 0x9903, pia_2_r },
	{ 0x9c00, 0x9c03, pia_3_r },
	{ 0xc000, 0xffff, MRA_ROM },
	{ -1 } /* end of table */
};

static struct MemoryReadAddress readmem_cpu_video[] =
{
	{ 0x0000, 0x7fff, qix_videoram_r },
	{ 0x8000, 0x83ff, qix_sharedram_r_2 },
	{ 0x8400, 0x87ff, MRA_RAM },
	{ 0x9400, 0x9400, qix_addresslatch_r },
	{ 0x9800, 0x9800, qix_scanline_r },
	{ 0xc000, 0xffff, MRA_ROM },
	{ -1 } /* end of table */
};

static struct MemoryReadAddress readmem_cpu_sound[] =
{
	{ 0x0000, 0x007f, MRA_RAM },
	{ 0x2000, 0x2003, pia_6_r },
	{ 0x4000, 0x4003, pia_5_r },
	{ 0xf800, 0xffff, MRA_ROM },
	{ -1 } /* end of table */
};

static struct MemoryWriteAddress writemem_cpu_data[] =
{
	{ 0x8000, 0x83ff, qix_sharedram_w },
	{ 0x8400, 0x87ff, MWA_RAM },
	{ 0x8c00, 0x8c00, qix_video_firq_w },
	{ 0x9000, 0x9003, pia_4_w },
	{ 0x9400, 0x9403, pia_1_w },
	{ 0x9900, 0x9903, pia_2_w },
	{ 0x9c00, 0x9c03, pia_3_w },
	{ 0xc000, 0xffff, MWA_ROM },
	{ -1 } /* end of table */
};

static struct MemoryWriteAddress writemem_cpu_video[] =
{
	{ 0x0000, 0x7fff, qix_videoram_w },
	{ 0x8000, 0x83ff, qix_sharedram_w },
	{ 0x8400, 0x87ff, MWA_RAM },
	{ 0x8800, 0x8800, qix_palettebank_w, &qix_palettebank },
	{ 0x8c00, 0x8c00, qix_data_firq_w },
	{ 0x9000, 0x93ff, qix_paletteram_w, &qix_paletteram },
	{ 0x9400, 0x9400, qix_addresslatch_w },
	{ 0x9402, 0x9403, MWA_RAM, &qix_videoaddress },
	{ 0xc000, 0xffff, MWA_ROM },
	{ -1 } /* end of table */
};

static struct MemoryWriteAddress writemem_cpu_sound[] =
{
	{ 0x0000, 0x007f, MWA_RAM },
	{ 0x2000, 0x2003, pia_6_w },
	{ 0x4000, 0x4003, pia_5_w },
	{ 0xf800, 0xffff, MWA_ROM },
	{ -1 } /* end of table */
};


INPUT_PORTS_START( input_ports )
	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_4WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_4WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_4WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_4WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON1 )

	PORT_START	/* IN1 */
	PORT_BITX(0x01, IP_ACTIVE_LOW, IPT_SERVICE, "Test Advance", OSD_KEY_F1, IP_JOY_DEFAULT, 0)
	PORT_BITX(0x02, IP_ACTIVE_LOW, IPT_SERVICE, "Test Next line", OSD_KEY_F2, IP_JOY_DEFAULT, 0)
	PORT_BITX(0x04, IP_ACTIVE_LOW, IPT_SERVICE, "Test Slew Up", OSD_KEY_F5, IP_JOY_DEFAULT, 0)
	PORT_BITX(0x08, IP_ACTIVE_LOW, IPT_SERVICE, "Test Slew Down", OSD_KEY_F6, IP_JOY_DEFAULT, 0)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN3 ) /* Coin switch */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_TILT )
INPUT_PORTS_END


static struct MachineDriver machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_M6809,
			1250000,		/* 1.25 Mhz */
			0,			/* memory region */
			readmem_cpu_data,	/* MemoryReadAddress */
			writemem_cpu_data,	/* MemoryWriteAddress */
			0,			/* IOReadPort */
			0,			/* IOWritePort */
			interrupt,
			1
		},
		{
			CPU_M6809,
			1250000,		/* 1.25 Mhz */
			2,			/* memory region #2 */
			readmem_cpu_video, writemem_cpu_video, 0, 0,
			ignore_interrupt,
			1
		},
		{
			CPU_M6802 | CPU_AUDIO_CPU,
			3680000/4,		/* 0.92 Mhz */
			3,			/* memory region #3 */
			readmem_cpu_sound, writemem_cpu_sound, 0, 0,
			ignore_interrupt,
			1
		}
	},
	60,					/* frames per second */
	100,	/* 100 CPU slices per frame - an high value to ensure proper */
			/* synchronization of the CPUs */
	qix_init_machine,			/* init machine routine */ /* JB 970526 */

	/* video hardware */
	256, 256,				/* screen_width, screen_height */
	/* this is the maximum viewable area, as shown by the test mode */
	{ 3, 253, 2, 252 },		        /* struct rectangle visible_area */
	0,				/* GfxDecodeInfo * */
	256,			/* total colors */
	0,			/* color table length */
	qix_vh_convert_color_prom,					/* convert color prom routine */

	VIDEO_TYPE_RASTER|VIDEO_MODIFIES_PALETTE|VIDEO_SUPPORTS_DIRTY,
	0,					/* vh_init routine */
	qix_vh_start,				/* vh_start routine */ /* JB 970524 */
	qix_vh_stop,				/* vh_stop routine */ /* JB 970524 */
	qix_vh_screenrefresh,		        /* vh_update routine */	/* JB 970524 */

	/* sound hardware */
	0,					/* sh_init routine */
	qix_sh_start,			/* sh_start routine */
	DAC_sh_stop,			/* sh_stop routine */
	DAC_sh_update			/* sh_update routine */
};



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( qix_rom )
	ROM_REGION(0x10000)	/* 64k for code for the first CPU (Data) */
	ROM_LOAD( "u12", 0xC000, 0x800, 0x87bd3a11 )
	ROM_LOAD( "u13", 0xC800, 0x800, 0x85586b74 )
	ROM_LOAD( "u14", 0xD000, 0x800, 0x541d5c6f )
	ROM_LOAD( "u15", 0xD800, 0x800, 0xcbd010de )
	ROM_LOAD( "u16", 0xE000, 0x800, 0xf9da5efe )
	ROM_LOAD( "u17", 0xE800, 0x800, 0x14c09e2a )
	ROM_LOAD( "u18", 0xF000, 0x800, 0x22ae35fa )
	ROM_LOAD( "u19", 0xF800, 0x800, 0x1bf904ff )

	ROM_REGION(0x800)
	/* empty memory region - not used by the game, but needed bacause the main */
	/* core currently always frees region #1 after initialization. */

	ROM_REGION(0x10000)	/* 64k for code for the second CPU (Video) */
	ROM_LOAD(  "u4", 0xC800, 0x800, 0x08bbfc51 )
	ROM_LOAD(  "u5", 0xD000, 0x800, 0xdd0f67b3 )
	ROM_LOAD(  "u6", 0xD800, 0x800, 0x37f8ce3c )
	ROM_LOAD(  "u7", 0xE000, 0x800, 0x733acfe0 )
	ROM_LOAD(  "u8", 0xE800, 0x800, 0xe1c7b84b )
	ROM_LOAD(  "u9", 0xF000, 0x800, 0xb662095a )
	ROM_LOAD( "u10", 0xF800, 0x800, 0x559ebf32 )

	ROM_REGION(0x10000) 	/* 64k for code for the third CPU (sound) */
	ROM_LOAD( "u27", 0xF800, 0x800, 0xdc9c8536 )
ROM_END

ROM_START( qix2_rom )
	ROM_REGION(0x10000)	/* 64k for code for the first CPU (Data) */
	ROM_LOAD( "u12.rmb", 0xC000, 0x800, 0x2ff446f6 )
	ROM_LOAD( "u13.rmb", 0xC800, 0x800, 0x51a32aeb )
	ROM_LOAD( "u14.rmb", 0xD000, 0x800, 0xa887b715 )
	ROM_LOAD( "u15.rmb", 0xD800, 0x800, 0x0c84a5e8 )
	ROM_LOAD( "u16.rmb", 0xE000, 0x800, 0xcf49e3e5 )
	ROM_LOAD( "u17.rmb", 0xE800, 0x800, 0x026e58b0 )
	ROM_LOAD( "u18.rmb", 0xF000, 0x800, 0x5be9ed5f )
	ROM_LOAD( "u19.rmb", 0xF800, 0x800, 0x83908386 )

	ROM_REGION(0x800)
	/* empty memory region - not used by the game, but needed bacause the main */
	/* core currently always frees region #1 after initialization. */

	ROM_REGION(0x10000)	/* 64k for code for the second CPU (Video) */
	ROM_LOAD(  "u3.rmb", 0xC000, 0x800, 0xfae6cc6e )
	ROM_LOAD(  "u4.rmb", 0xC800, 0x800, 0xfa03efcb )
	ROM_LOAD(  "u5.rmb", 0xD000, 0x800, 0x55b90e87 )
	ROM_LOAD(  "u6.rmb", 0xD800, 0x800, 0xdfabdc37 )
	ROM_LOAD(  "u7.rmb", 0xE000, 0x800, 0x11800d28 )
	ROM_LOAD(  "u8.rmb", 0xE800, 0x800, 0x57303416 )
	ROM_LOAD(  "u9.rmb", 0xF000, 0x800, 0xf875b473 )
	ROM_LOAD( "u10.rmb", 0xF800, 0x800, 0xd6a50cbb )

	ROM_REGION(0x10000) 	/* 64k for code for the third CPU (sound) */
	ROM_LOAD( "u27.rmb", 0xF800, 0x800, 0xdc9c8536 )
ROM_END




/* Loads high scores and all other CMOS settings */
static int hiload(void)
{
	/* get RAM pointer (data is in second CPU's memory region) */
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[1].memory_region];
	void *f;


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
	{
		osd_fread(f,&RAM[0x8400],0x400);
		osd_fclose(f);
	}

	return 1;
}



static void hisave(void)
{
	/* get RAM pointer (data is in second CPU's memory region) */
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[1].memory_region];
	void *f;


	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0x8400],0x400);
		osd_fclose(f);
	}
}



struct GameDriver qix_driver =
{
	"Qix",
	"qix",
	"John Butler\nEd Mueller\nAaron Giles\nMarco Cassili",
	&machine_driver,

	qix_rom,
	0, 0,   /* ROM decode and opcode decode functions */
	0,      /* Sample names */
	0,	/* sound_prom */

	0/*TBR*/, input_ports, 0/*TBR*/, 0/*TBR*/, 0/*TBR*/,

	0, 0, 0,   /* colors, palette, colortable */
	ORIENTATION_DEFAULT,

	hiload, hisave	       /* High score load and save */
};

struct GameDriver qix2_driver =
{
	"Qix II (Tournament)",
	"qix2",
	"John Butler\nEd Mueller\nAaron Giles\nMarco Cassili",
	&machine_driver,

	qix2_rom,
	0, 0,   /* ROM decode and opcode decode functions */
	0,      /* Sample names */
	0,	/* sound_prom */

	0/*TBR*/, input_ports, 0/*TBR*/, 0/*TBR*/, 0/*TBR*/,

	0, 0, 0,   /* colors, palette, colortable */
	ORIENTATION_DEFAULT,

	hiload, hisave	       /* High score load and save */
};


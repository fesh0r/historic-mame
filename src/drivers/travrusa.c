/****************************************************************************
Traverse USA Driver

Lee Taylor
John Clegg

Loosely based on the our previous 10 Yard Fight driver.

Know problems :-
	Wrong Colours!!! Need Color Proms!

****************************************************************************/

#include "driver.h"
#include "Z80/Z80.h"
#include "sndhrdw/8910intf.h"
#include "vidhrdw/generic.h"


extern unsigned char *trace_scroll_x_low;
extern unsigned char *trace_scroll_x_high;

void trace_vh_convert_color_prom(unsigned char *palette, unsigned char *colortable,const unsigned char *color_prom);
int trace_vh_start(void);
void trace_vh_stop(void);
void trace_vh_screenrefresh(struct osd_bitmap *bitmap);

/* JB 971012 ... */
extern unsigned char *mpatrol_io_ram;
extern unsigned char *mpatrol_sample_data;
extern unsigned char *mpatrol_sample_table;
int mpatrol_sh_init(const char *);
int mpatrol_sh_start(void);
int mpatrol_sh_interrupt(void);
void mpatrol_io_w(int offset, int value);
int mpatrol_io_r(int offset);
void mpatrol_sample_trigger_w(int offset,int value);
void mpatrol_sound_cmd_w(int offset, int value);
/* ... JB 971012 */

static struct MemoryReadAddress readmem[] =
{
	{ 0x0000, 0x7fff, MRA_ROM },
	{ 0x8000, 0x8fff, MRA_RAM },         /* Video and Color ram */
	{ 0xd000, 0xd000, input_port_0_r },	/* IN0 */
	{ 0xd001, 0xd001, input_port_1_r },	/* IN1 */
	{ 0xd002, 0xd002, input_port_2_r },	/* IN2 */
	{ 0xd003, 0xd003, input_port_3_r },	/* DSW1 */
	{ 0xd004, 0xd004, input_port_4_r },	/* DSW2 */
	{ 0xe000, 0xefff, MRA_RAM },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress writemem[] =
{
	{ 0x0000, 0x7fff, MWA_ROM },
	{ 0x8000, 0x8fff, videoram_w, &videoram, &videoram_size },
	{ 0x9000, 0x9000, MWA_RAM, &trace_scroll_x_low },
	{ 0xa000, 0xa000, MWA_RAM, &trace_scroll_x_high },
	{ 0xc800, 0xc9ff, MWA_RAM, &spriteram, &spriteram_size },
	{ 0xd000, 0xd001, mpatrol_sound_cmd_w }, /* JB 971012 */
	{ 0xe000, 0xefff, MWA_RAM },
	{ -1 }	/* end of table */
};

static struct MemoryReadAddress sound_readmem[] =
{
	{ 0x0000, 0x001f, mpatrol_io_r, &mpatrol_io_ram },
	{ 0x0080, 0x00ff, MRA_RAM },
	{ 0x8000, 0xffff, MRA_ROM },
	{ 0xe780, 0xe7bf, MRA_ROM, &mpatrol_sample_table },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress sound_writemem[] =
{
	{ 0x0000, 0x001f, mpatrol_io_w },
	{ 0x00c4, 0x00cb, mpatrol_sample_trigger_w, &mpatrol_sample_data },
	{ 0x0080, 0x00ff, MWA_RAM },
	{ 0x0801, 0x0802, MWA_NOP },
	{ 0x8000, 0xffff, MWA_ROM },
	{ -1 }	/* end of table */
};



INPUT_PORTS_START( input_ports )
	PORT_START	/* IN0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	/* coin input must be active for 19 frames to be consistently recognized */
	PORT_BITX(0x04, IP_ACTIVE_LOW, IPT_COIN1 | IPF_IMPULSE,
		"Coin", IP_KEY_DEFAULT, IP_JOY_DEFAULT, 19 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* IN1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT | IPF_8WAY )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT | IPF_8WAY )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_8WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_8WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BITX(0x20, IP_ACTIVE_LOW, IPT_BUTTON1, "Accelerate", IP_KEY_DEFAULT, IP_JOY_DEFAULT, 0 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BITX(0x80, IP_ACTIVE_LOW, IPT_BUTTON2, "Brake", IP_KEY_DEFAULT, IP_JOY_DEFAULT, 0 )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x03, 0x02, "Fuel Reduced on Collision", IP_KEY_NONE )
	PORT_DIPSETTING( 0x03, "Low" )
	PORT_DIPSETTING( 0x02, "Med" )
	PORT_DIPSETTING( 0x01, "Hi" )
	PORT_DIPSETTING( 0x00, "Max" )
	PORT_DIPNAME( 0x04, 0x04, "Fuel Consumption", IP_KEY_NONE )
	PORT_DIPSETTING( 0x04, "Low" )
	PORT_DIPSETTING( 0x00, "Hi" )
	PORT_DIPNAME( 0x08, 0x00, "Continue", IP_KEY_NONE )
	PORT_DIPSETTING( 0x08, "No" )
	PORT_DIPSETTING( 0x00, "Yes" )
	PORT_DIPNAME( 0xf0, 0xf0, "Coinage", IP_KEY_NONE )
	PORT_DIPSETTING( 0xf0, "1 Coin/1 Credit" )
	PORT_DIPSETTING( 0xe0, "2 Coins/1 Credit" )
	PORT_DIPSETTING( 0xd0, "3 Coins/1 Credit" )
	PORT_DIPSETTING( 0xc0, "4 Coins/1 Credit" )
	PORT_DIPSETTING( 0xb0, "5 Coins/1 Credit" )
	PORT_DIPSETTING( 0xa0, "6 Coins/1 Credit" )
	PORT_DIPSETTING( 0x70, "1 Coin/2 Credit" )
	PORT_DIPSETTING( 0x60, "1 Coins/3 Credit" )
	PORT_DIPSETTING( 0x50, "3 Coins/4 Credit" )
	PORT_DIPSETTING( 0x40, "4 Coins/5 Credit" )
	PORT_DIPSETTING( 0x30, "5 Coins/6 Credit" )
	PORT_DIPSETTING( 0x20, "UNKNOWN" )

	/* PORT_DIPSETTING( 0x10, "INVALID" ) */
	/* PORT_DIPSETTING( 0x00, "INVALID" ) */

	PORT_START	/* DSW2 */
	PORT_DIPNAME( 0x01, 0x01, "SW1B", IP_KEY_NONE )
	PORT_DIPSETTING( 0x01, "Off" )
	PORT_DIPSETTING( 0x00, "On" )
	PORT_DIPNAME( 0x02, 0x00, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING( 0x00, "Upright" )
	PORT_DIPSETTING( 0x02, "Cocktail" )
	PORT_DIPNAME( 0x04, 0x04, "Coin Shute B", IP_KEY_NONE )
	PORT_DIPSETTING( 0x04, "Off" )
	PORT_DIPSETTING( 0x00, "On" )
	PORT_DIPNAME( 0x08, 0x08, "Speed Type", IP_KEY_NONE )
	PORT_DIPSETTING( 0x08, "MP/H" )
	PORT_DIPSETTING( 0x00, "KM/H" )
	PORT_DIPNAME( 0x10, 0x10, "SW5B", IP_KEY_NONE )
	PORT_DIPSETTING( 0x10, "Off" )
	PORT_DIPSETTING( 0x00, "On" )
	PORT_DIPNAME( 0x20, 0x20, "SW6B", IP_KEY_NONE )
	PORT_DIPSETTING( 0x20, "Off" )
	PORT_DIPSETTING( 0x00, "On" )
	PORT_DIPNAME( 0x40, 0x40, "Collision Detection", IP_KEY_NONE )
	PORT_DIPSETTING( 0x00, "Off" )
	PORT_DIPSETTING( 0x40, "On" )
	PORT_BITX(    0x80, 0x80, IPT_DIPSWITCH_NAME | IPF_TOGGLE, "Service Mode", OSD_KEY_F2, IP_JOY_NONE, 0 )
	PORT_DIPSETTING(    0x80, "Off" )
	PORT_DIPSETTING(    0x00, "On" )
INPUT_PORTS_END



static struct GfxLayout charlayout =
{
	8,8,	/* 8*8 characters */
	1024,	/* 1024 characters */
	3,	/* 3 bits per pixel */
	{ 2*1024*8*8, 1024*8*8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 8*0, 8*1, 8*2, 8*3, 8*4, 8*5, 8*6, 8*7 },
	8*8	/* every char takes 8 consecutive bytes */
};
static struct GfxLayout spritelayout =
{
	16,16,	/* 16*16 sprites */
	256,	/* 256 sprites */
	3,	/* 3 bits per pixel */
	{ 2*256*16*16, 256*16*16, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7, 16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+4, 16*8+5, 16*8+6, 16*8+7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	32*8	/* every sprite takes 32 consecutive bytes */
};


static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x00000, &charlayout,      0, 16 },
	{ 1, 0x06000, &spritelayout, 16*8, 16 },
	{ -1 } /* end of array */
};



/* these are NOT the original PROMs - they are from 10 Yard Fight */
static unsigned char color_prom[] =
{
	/* mmi6349.ij - character palette */
	/* it should be 512x8 (?), but only the first 128 bytes are used. */
	0x00,0x05,0x07,0xA6,0xF0,0xB8,0xC0,0xE2,0x00,0xFF,0xFF,0x00,0x00,0x00,0x07,0xF8,
	0x5B,0xFF,0x27,0x28,0x20,0x00,0x9A,0xE8,0xEC,0xFF,0x27,0x28,0x20,0x00,0x9A,0xE8,
	0x00,0xFF,0xC0,0xA6,0x03,0xE0,0xF8,0xE7,0x00,0xC0,0xE5,0xFF,0xB7,0x07,0x04,0x63,
	0x00,0xF0,0xE3,0xFF,0xB7,0x07,0xA7,0x60,0x00,0xFF,0x18,0x07,0xEC,0xB8,0xE3,0x88,
	0xE0,0xFF,0x5B,0xF8,0x04,0x20,0x9B,0x00,0x5B,0x5B,0xFF,0xC0,0xEC,0x00,0xC7,0xFF,
	0xE0,0xFF,0x5B,0xFF,0x00,0x00,0x38,0x00,0x5B,0xFF,0x5B,0xFF,0xEC,0x00,0x3F,0x00,
	0xB8,0x00,0xE0,0xF8,0x05,0xE2,0xC0,0x80,0xB8,0xFF,0x5B,0xF8,0x04,0x20,0x9B,0x00,
	0xF8,0x00,0xB8,0x98,0xEC,0x00,0x00,0x00,0x5B,0xFF,0xB8,0x98,0xEC,0x00,0x60,0xE2,
	/* tbp18s.2 - sprite palette */
	0x00,0xC0,0xF8,0x2F,0x01,0x07,0x38,0xFF,0xD8,0x98,0x88,0x5A,0x9C,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	/* tbp24s10.3 - sprite lookup table */
	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x00,0x02,0x0C,0x05,0x07,0x05,0x04,0x07,
	0x04,0x06,0x05,0x0E,0x06,0x04,0x0F,0x04,0x04,0x04,0x0D,0x0E,0x05,0x06,0x05,0x04,
	0x04,0x07,0x07,0x05,0x05,0x05,0x05,0x05,0x05,0x0D,0x0F,0x05,0x05,0x0D,0x07,0x07,
	0x05,0x05,0x0D,0x07,0x07,0x05,0x05,0x07,0x05,0x07,0x0D,0x07,0x07,0x05,0x05,0x07,
	0x05,0x07,0x0D,0x07,0x07,0x05,0x05,0x07,0x05,0x0D,0x0D,0x05,0x07,0x05,0x05,0x07,
	0x05,0x07,0x0D,0x05,0x07,0x05,0x05,0x07,0x05,0x07,0x07,0x0D,0x05,0x05,0x05,0x07,
	0x05,0x07,0x07,0x0D,0x05,0x05,0x05,0x07,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
	0x05,0x05,0x07,0x07,0x05,0x05,0x07,0x07,0x05,0x07,0x07,0x07,0x05,0x05,0x07,0x07,
	0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
	0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
	0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
	0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
	0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
	0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
	0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
	0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,0x05,
};


static struct MachineDriver machine_driver =
{
	/* basic machine hardware */
	{
		{
			CPU_Z80,
			4000000,	/* 4 Mhz (?) */
			0,
			readmem,writemem,0,0,
			interrupt,1
		},
		{
			CPU_M6802 | CPU_AUDIO_CPU,
			1000000,	/* 1.0 Mhz ? */
			2,
			sound_readmem,sound_writemem,0,0,
			nmi_interrupt,68	/* 68 ints per frame = 4080 ints/sec */
		}
	},
	60,
	10,	/* 10 CPU slices per frame - enough for the sound CPU to read all commands */ /* JB 971012 */
	0,

	/* video hardware */
	32*8, 32*8, { 1*8, 31*8-1, 0*8, 32*8-1 },
	gfxdecodeinfo,
	128+32, 16*8+16*8,
	trace_vh_convert_color_prom,

	VIDEO_TYPE_RASTER,
	0,
	trace_vh_start,
	trace_vh_stop,
	trace_vh_screenrefresh,

	/* sound hardware */
	mpatrol_sh_init,
	mpatrol_sh_start,
	AY8910_sh_stop,
	AY8910_sh_update
};


/***************************************************************************

  Game driver(s)

***************************************************************************/


ROM_START( travrusa_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "ZIPPYRAC.000", 0x0000, 0x2000, 0x25468936 )
	ROM_LOAD( "ZIPPYRAC.005", 0x2000, 0x2000, 0x772f4069 )
	ROM_LOAD( "ZIPPYRAC.006", 0x4000, 0x2000, 0xd92b9f27 )
	ROM_LOAD( "ZIPPYRAC.007", 0x6000, 0x2000, 0x1ebe2522 )

	ROM_REGION(0x12000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ZIPPYRAC.001", 0x00000, 0x2000, 0x080c2eea )
	ROM_LOAD( "ZIPPYRAC.002", 0x02000, 0x2000, 0x822f3c4d )
	ROM_LOAD( "ZIPPYRAC.003", 0x04000, 0x2000, 0x4dff1a15 )
	ROM_LOAD( "ZIPPYRAC.008", 0x06000, 0x2000, 0xd6c5bd0d )
	ROM_LOAD( "ZIPPYRAC.009", 0x08000, 0x2000, 0x6372812a )
	ROM_LOAD( "ZIPPYRAC.010", 0x0a000, 0x2000, 0x76e982c9 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	/* should the samples be loaded here ??? */
	ROM_LOAD( "ZIPPYRAC.004", 0xf000, 0x1000, 0x91c75683 )		/*Code*/
ROM_END

#if 0
ROM_START( motorace_rom )
	ROM_REGION(0x10000)	/* 64k for code */
//	ROM_LOAD( "ZIPPYRAC.000", 0x0000, 0x2000, 0x25468936 )
	ROM_LOAD( "mr13l", 0x2000, 0x2000, 0x33a87d00 )
	ROM_LOAD( "mr23k", 0x4000, 0x2000, 0x7897130f )
	ROM_LOAD( "mr33j", 0x6000, 0x2000, 0x78dfc871 )

	ROM_REGION(0x12000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "mr73e", 0x00000, 0x2000, 0x78bc5e5a )
	ROM_LOAD( "mr83c", 0x02000, 0x2000, 0x822f3c4d )
	ROM_LOAD( "mr93a", 0x04000, 0x2000, 0x4dff1a15 )
	ROM_LOAD( "mr43n", 0x06000, 0x2000, 0x9227b5d3 )
	ROM_LOAD( "mr53m", 0x08000, 0x2000, 0x46da07c2 )
	ROM_LOAD( "mr63k", 0x0a000, 0x2000, 0xe3678ca5 )

	ROM_REGION(0x10000)	/* 64k for sound cpu */
	/* should the samples be loaded here ??? */
	ROM_LOAD( "mr101a", 0xf000, 0x1000, 0x91c75683 )		/*Code*/
ROM_END
#endif


static int hiload(void)
{
	/* get RAM pointer (this game is multiCPU, we can't assume the global */
	/* RAM pointer is pointing to the right place) */
	unsigned char *RAM = Machine->memory_region[0];

	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0xe080],"\x49\x45\x4f",3) == 0)
	{
		void *f;

		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0xe07c],74);
			osd_fclose(f);
		}

		return 1;
	}
	else return 0;	/* we can't load the hi scores yet */
}

static void hisave(void)
{	void *f;
	/* get RAM pointer (this game is multiCPU, we can't assume the global */
	/* RAM pointer is pointing to the right place) */
	unsigned char *RAM = Machine->memory_region[0];

	if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,1)) != 0)
	{
		osd_fwrite(f,&RAM[0xe07c],74);
		osd_fclose(f);
	}
	memset(&RAM[0xe07c], 0, 74);	/* LT 13-11-97 */
}



struct GameDriver travrusa_driver =
{
	"Traverse USA",
	"travrusa",
	"Lee Taylor (Driver Code)\nJohn Clegg (Graphics Code)\nAaron Giles (sound)\nThierry Lescot (color info)",
	&machine_driver,

	travrusa_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	0/*TBR*/,input_ports,0/*TBR*/,0/*TBR*/,0/*TBR*/,

	color_prom, 0, 0,
	ORIENTATION_ROTATE_270,

	hiload, hisave
};

#if 0
struct GameDriver motorace_driver =
{
	"Motorace USA",
	"motorace",
	"Lee Taylor (Driver Code)\nJohn Clegg (Graphics Code)\nAaron Giles (sound)\nThierry Lescot (color info)",
	&machine_driver,

	motorace_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	0/*TBR*/,input_ports,0/*TBR*/,0/*TBR*/,0/*TBR*/,

	color_prom, 0, 0,
	ORIENTATION_ROTATE_270,

	hiload, hisave
};
#endif

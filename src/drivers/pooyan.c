/***************************************************************************

Pooyan memory map (preliminary)

Thanks must go to Mike Cuddy for providing information on this one.

Sound processor memory map.
0x3000-0x33ff RAM.
AY-8910 #1 : reg 0x5000
	     wr  0x4000
             rd  0x4000

AY-8910 #2 : reg 0x7000
	     wr  0x6000
             rd  0x6000

Main processor memory map.
0000-7fff ROM
8000-83ff color RAM
8400-87ff video RAM
8800-8fff RAM
9000-97ff sprite RAM (only areas 0x9010 and 0x9410 are used).

memory mapped ports:

read:
0xA000	Dipswitch 2 adddbtll
        a = attract mode
        ddd = difficulty 0=easy, 7=hardest.
        b = bonus setting (easy/hard)
        t = table / upright
        ll = lives: 11=3, 10=4, 01=5, 00=255.

0xA0E0  llllrrrr
        l == left coin mech, r = right coinmech.

0xA080	IN0 Port
0xA0A0	IN1 Port
0xA0C0	IN2 Port

write:
0xA100	command for the audio CPU.
0xA180	NMI enable. (0xA180 == 1 = deliver NMI to CPU).

0xA181	interrupt trigger on audio CPU.

0xA183	maybe reset sound cpu?

0xA184	????

0xA187	Flip screen

interrupts:
standard NMI at 0x66

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"
#include "sndhrdw/generic.h"
#include "sndhrdw/8910intf.h"



void pooyan_flipscreen_w(int offset,int data);
void pooyan_vh_convert_color_prom(unsigned char *palette, unsigned char *colortable,const unsigned char *color_prom);
void pooyan_vh_screenrefresh(struct osd_bitmap *bitmap);

void pooyan_sh_irqtrigger_w(int offset,int data);
int pooyan_sh_start(void);



static struct MemoryReadAddress readmem[] =
{
	{ 0x0000, 0x7fff, MRA_ROM },
	{ 0x8000, 0x8fff, MRA_RAM },	/* color and video RAM */
	{ 0xa000, 0xa000, input_port_4_r },	/* DSW2 */
	{ 0xa080, 0xa080, input_port_0_r },	/* IN0 */
	{ 0xa0a0, 0xa0a0, input_port_1_r },	/* IN1 */
	{ 0xa0c0, 0xa0c0, input_port_2_r },	/* IN2 */
	{ 0xa0e0, 0xa0e0, input_port_3_r },	/* DSW1 */
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress writemem[] =
{
	{ 0x0000, 0x7fff, MWA_ROM },
	{ 0x8000, 0x83ff, colorram_w, &colorram },
	{ 0x8400, 0x87ff, videoram_w, &videoram, &videoram_size },
	{ 0x8800, 0x8fff, MWA_RAM },
	{ 0x9010, 0x903f, MWA_RAM, &spriteram, &spriteram_size },
	{ 0x9410, 0x943f, MWA_RAM, &spriteram_2 },
	{ 0xa000, 0xa000, MWA_NOP },	/* watchdog reset? */
	{ 0xa100, 0xa100, soundlatch_w },
	{ 0xa180, 0xa180, interrupt_enable_w },
	{ 0xa181, 0xa181, pooyan_sh_irqtrigger_w },
	{ 0xa187, 0xa187, pooyan_flipscreen_w },
	{ -1 }	/* end of table */
};



static struct MemoryReadAddress sound_readmem[] =
{
	{ 0x3000, 0x33ff, MRA_RAM },
	{ 0x4000, 0x4000, AY8910_read_port_0_r },
	{ 0x6000, 0x6000, AY8910_read_port_1_r },
	{ 0x0000, 0x1fff, MRA_ROM },
	{ -1 }	/* end of table */
};

static struct MemoryWriteAddress sound_writemem[] =
{
	{ 0x3000, 0x33ff, MWA_RAM },
	{ 0x5000, 0x5000, AY8910_control_port_0_w },
	{ 0x4000, 0x4000, AY8910_write_port_0_w },
	{ 0x7000, 0x7000, AY8910_control_port_1_w },
	{ 0x6000, 0x6000, AY8910_write_port_1_w },
	{ 0x0000, 0x1fff, MWA_ROM },
	{ -1 }	/* end of table */
};


INPUT_PORTS_START( input_ports )
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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_2WAY )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_2WAY )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* IN2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_UP | IPF_2WAY | IPF_COCKTAIL )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN | IPF_2WAY | IPF_COCKTAIL )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 | IPF_COCKTAIL )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START	/* DSW0 */
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
	PORT_DIPSETTING(    0x00, "Attract Mode - No Play" )
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

	PORT_START	/* DSW1 */
	PORT_DIPNAME( 0x03, 0x03, "Lives", IP_KEY_NONE )
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_BITX( 0,       0x00, IPT_DIPSWITCH_SETTING | IPF_CHEAT, "255", IP_KEY_NONE, IP_JOY_NONE, 0 )
	PORT_DIPNAME( 0x04, 0x00, "Cabinet", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "Upright" )
	PORT_DIPSETTING(    0x04, "Cocktail" )
	PORT_DIPNAME( 0x08, 0x08, "Bonus Life", IP_KEY_NONE )
	PORT_DIPSETTING(    0x08, "50000 80000" )
	PORT_DIPSETTING(    0x00, "30000 70000" )
	PORT_DIPNAME( 0x70, 0x70, "Difficulty", IP_KEY_NONE )
	PORT_DIPSETTING(    0x70, "Easiest" )
	PORT_DIPSETTING(    0x60, "Easier" )
	PORT_DIPSETTING(    0x50, "Easy" )
	PORT_DIPSETTING(    0x40, "Normal" )
	PORT_DIPSETTING(    0x30, "Medium" )
	PORT_DIPSETTING(    0x20, "Difficult" )
	PORT_DIPSETTING(    0x10, "Hard" )
	PORT_DIPSETTING(    0x00, "Hardest" )
	PORT_DIPNAME( 0x80, 0x00, "Demo Sounds", IP_KEY_NONE )
	PORT_DIPSETTING(    0x00, "On" )
	PORT_DIPSETTING(    0x80, "Off" )
INPUT_PORTS_END



static struct GfxLayout charlayout =
{
	8,8,	/* 8*8 characters */
	256,	/* 256 characters */
	4,	/* 4 bits per pixel */
	{ 0x1000*8+4, 0x1000*8+0, 4, 0 },
	{ 0, 1, 2, 3, 8*8+0,8*8+1,8*8+2,8*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8	/* every char takes 16 consecutive bytes */
};
static struct GfxLayout spritelayout =
{
	16,16,	/* 16*16 sprites */
	64,	/* 64 sprites */
	4,	/* 4 bits per pixel */
	{ 0x1000*8+4, 0x1000*8+0, 4, 0 },
	{ 0, 1, 2, 3,  8*8+0, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3,  24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8	/* every sprite takes 64 consecutive bytes */
};



static struct GfxDecodeInfo gfxdecodeinfo[] =
{
	{ 1, 0x0000, &charlayout,       0, 16 },
	{ 1, 0x2000, &spritelayout, 16*16, 16 },
	{ -1 } /* end of array */
};



/* these are NOT the original color PROMs */
static unsigned char color_prom[] =
{
	/* char palette */
	0x0F,0xE1,0x1C,0x03,0xFF,0xF7,0xD0,0x88,0xB0,0xD4,0x1C,0x10,0xFD,0x41,0x92,0xFB,
	0x0F,0x01,0x02,0x03,0x08,0x10,0x1C,0x88,0xB0,0xD4,0x1C,0x10,0xFC,0x49,0x92,0xFF,
	0x0F,0xE1,0x02,0x03,0x08,0xA0,0xD0,0x88,0xB0,0xD4,0x1C,0x10,0xFC,0x69,0x92,0xFB,
	0x0F,0x01,0x02,0x03,0xFC,0x10,0x3C,0x88,0xB0,0xD4,0x1C,0xB0,0xFC,0x49,0x92,0xFF,
	0x0F,0x01,0x02,0x03,0x08,0x10,0x1C,0x88,0xB0,0xD4,0x1C,0x90,0xFC,0x49,0x92,0xFF,
	0x0F,0x1C,0x02,0x03,0x08,0x10,0x1C,0x88,0xB0,0xD4,0x1C,0x90,0xFC,0xFD,0x92,0xFF,
	0x0F,0x01,0x02,0x03,0x08,0x10,0x1C,0x88,0xB0,0xD4,0x1C,0x90,0xFC,0x49,0x92,0xFF,
	0x0F,0x81,0x02,0x1C,0xE8,0xFC,0x1C,0x88,0xB0,0xD4,0x1C,0x90,0xFC,0xE0,0x90,0xFF,
	0x0F,0x01,0x02,0x03,0x08,0x10,0x1C,0x88,0xB0,0xD4,0x1C,0x90,0xFC,0x49,0x92,0xFF,
	0x0F,0x01,0x02,0x03,0x08,0x10,0x1C,0x88,0xB0,0xD4,0x1C,0x90,0xFC,0x49,0x92,0xFF,
	0x0F,0x01,0x02,0x03,0x08,0x10,0x1C,0x88,0xB0,0xD4,0x1C,0x90,0xFC,0x49,0x92,0xFF,
	0x0F,0xFC,0x02,0x03,0xE1,0x10,0x1C,0x88,0xB0,0xD4,0x1C,0x90,0xFC,0x49,0x92,0xFF,
	0x0F,0x01,0x02,0x03,0x08,0x10,0x1C,0x88,0xB0,0xD4,0x1C,0x90,0xFC,0x49,0x92,0xFF,
	0x0F,0x01,0x02,0x03,0xE0,0x10,0x1C,0x88,0xB0,0xD4,0x1C,0x90,0xFC,0x49,0x92,0xFF,
	0x0F,0xDC,0x06,0x1C,0xFF,0xE0,0x1C,0x88,0xB0,0xD4,0x1C,0x90,0xFC,0xFD,0x92,0xFF,
	0x0F,0xDC,0x02,0x03,0xFF,0x10,0x1C,0x88,0xB0,0xD4,0x1C,0x90,0xFC,0x49,0x92,0xFB,

	/* Sprite palette */
	0x69,0xCD,0xB0,0x03,0xC1,0xF7,0xD4,0xE9,0x6E,0xD0,0x5D,0x30,0xFF,0x00,0xE3,0xFB,
	0x48,0xCC,0x02,0x03,0xC0,0x10,0xFC,0x09,0xD0,0x1F,0xDE,0x90,0xFC,0x49,0x92,0xFF,
	0x48,0x01,0x22,0xFF,0x08,0x10,0x0F,0xFF,0x12,0x17,0xDF,0xFF,0xFC,0x09,0xFF,0x0F,
	0x00,0x01,0x21,0x0F,0x08,0x10,0x0F,0xFF,0x12,0x1F,0x0F,0xFF,0xFC,0x29,0x6F,0xFF,
	0x48,0xD6,0x02,0x03,0xC0,0x10,0xFC,0x09,0xB0,0x1F,0xDE,0x90,0xFC,0x49,0x92,0xFF,
	0x48,0x01,0x02,0x03,0xD7,0x10,0x1C,0x09,0x12,0x1F,0xDE,0x90,0xFC,0xD8,0x92,0xFF,
	0x48,0xDB,0xA0,0x03,0xE0,0xB7,0xDA,0x09,0x14,0x1C,0xDE,0x90,0x98,0x49,0x92,0xFF,
	0x48,0xDB,0xB6,0x03,0xE0,0x10,0xA1,0x80,0x12,0x1C,0xDE,0x92,0xB5,0x49,0x92,0xFF,
	0x48,0x01,0x02,0x0F,0x08,0x10,0xFF,0xFF,0x12,0x1F,0xDE,0xFF,0xFC,0x49,0x0F,0xFF,
	0x48,0xF8,0x02,0x03,0xE0,0x10,0xFC,0x09,0xF4,0x1F,0xDE,0x90,0xFC,0x49,0x92,0xFF,
	0x48,0xEF,0x02,0x01,0x08,0x10,0x1C,0x09,0x12,0x1F,0xDE,0x90,0xFC,0x49,0x92,0xFF,
	0x48,0xFC,0x02,0x03,0x08,0x10,0x1C,0x09,0x12,0x1F,0xDE,0x90,0xFC,0x49,0x92,0xFF,
	0x48,0xE0,0x02,0x03,0x08,0x10,0x1C,0x09,0x12,0x1F,0xDE,0x90,0xFC,0x48,0x92,0xFF,
	0x48,0x01,0x02,0x03,0x08,0x10,0x1C,0x09,0x12,0x1F,0xDE,0x90,0xFC,0x48,0x92,0xFF,
	0x48,0xE3,0x02,0x03,0x08,0x10,0x1C,0x09,0x12,0x1F,0xDE,0x90,0xFC,0x49,0x92,0xFF,
	0x48,0x1B,0x02,0x03,0xA8,0xF7,0x1C,0x09,0x12,0x1F,0x00,0x90,0xFC,0x49,0x92,0xFF
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
			14318180/4,	/* ? */
			2,	/* memory region #2 */
			sound_readmem,sound_writemem,0,0,
			ignore_interrupt,1	/* interrupts are triggered by the main CPU */
		}
	},
	60,
	10,	/* 10 CPU slices per frame - enough for the sound CPU to read all commands */
	0,

	/* video hardware */
	32*8, 32*8, { 0*8, 32*8-1, 2*8, 30*8-1 },
	gfxdecodeinfo,
	256,32*16,
	pooyan_vh_convert_color_prom,

	VIDEO_TYPE_RASTER|VIDEO_SUPPORTS_DIRTY,
	0,
	generic_vh_start,
	generic_vh_stop,
	pooyan_vh_screenrefresh,

	/* sound hardware */
	0,
	pooyan_sh_start,
	AY8910_sh_stop,
	AY8910_sh_update
};



/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( pooyan_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "ic22_a4.cpu",  0x0000, 0x2000, 0x8906608a )
	ROM_LOAD( "ic23_a5.cpu",  0x2000, 0x2000, 0x26eff7e3 )
	ROM_LOAD( "ic24_a6.cpu",  0x4000, 0x2000, 0x4d5af9a8 )
	ROM_LOAD( "ic25_a7.cpu",  0x6000, 0x2000, 0xe8a37e2f )

	ROM_REGION(0x4000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "ic13_g10.cpu", 0x0000, 0x1000, 0x70837e21 )
	ROM_LOAD( "ic14_g9.cpu",  0x1000, 0x1000, 0x10588368 )
	ROM_LOAD( "ic16_a8.cpu",  0x2000, 0x1000, 0x18d9a3fb )
	ROM_LOAD( "ic15_a9.cpu",  0x3000, 0x1000, 0xf00ec45a )

	ROM_REGION(0x10000)	/* 64k for the audio CPU */
	ROM_LOAD( "sd01_a7.snd",  0x0000, 0x1000, 0x41dc452c )
	ROM_LOAD( "sd02_a8.snd",  0x1000, 0x1000, 0xe108928c )
ROM_END

ROM_START( pootan_rom )
	ROM_REGION(0x10000)	/* 64k for code */
	ROM_LOAD( "poo_ic22.bin",  0x0000, 0x2000, 0x1fc0a5aa )
	ROM_LOAD( "poo_ic23.bin",  0x2000, 0x2000, 0x39faab9a )
	ROM_LOAD( "poo_ic24.bin",  0x4000, 0x2000, 0xd72a8cea )
	ROM_LOAD( "poo_ic25.bin",  0x6000, 0x2000, 0xb83a5d06 )

	ROM_REGION(0x4000)	/* temporary space for graphics (disposed after conversion) */
	ROM_LOAD( "poo_ic13.bin",  0x0000, 0x1000, 0x1e020240 )
	ROM_LOAD( "poo_ic14.bin",  0x1000, 0x1000, 0x7c58b368 )
	ROM_LOAD( "poo_ic16.bin",  0x2000, 0x1000, 0x18d9a3fb )
	ROM_LOAD( "poo_ic15.bin",  0x3000, 0x1000, 0xf00ec45a )

	ROM_REGION(0x10000)	/* 64k for the audio CPU */
	ROM_LOAD( "poo_a1.bin",   0x0000, 0x1000, 0x41dc452c )
	ROM_LOAD( "poo_a2.bin",   0x1000, 0x1000, 0xe108928c )
ROM_END


static int hiload(void)
{
	/* get RAM pointer (this game is multiCPU, we can't assume the global */
	/* RAM pointer is pointing to the right place) */
	unsigned char *RAM = Machine->memory_region[0];


	/* check if the hi score table has already been initialized */
	if (memcmp(&RAM[0x8a00],"\x00\x00\x01",3) == 0 &&
			memcmp(&RAM[0x8a1b],"\x00\x00\x01",3) == 0)
	{
		void *f;


		if ((f = osd_fopen(Machine->gamedrv->name,0,OSD_FILETYPE_HIGHSCORE,0)) != 0)
		{
			osd_fread(f,&RAM[0x89c0],3*10);
			osd_fread(f,&RAM[0x8a00],3*10);
			osd_fread(f,&RAM[0x8e00],3*10);
			RAM[0x88a8] = RAM[0x8a00];
			RAM[0x88a9] = RAM[0x8a01];
			RAM[0x88aa] = RAM[0x8a02];
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
		osd_fwrite(f,&RAM[0x89c0],3*10);
		osd_fwrite(f,&RAM[0x8a00],3*10);
		osd_fwrite(f,&RAM[0x8e00],3*10);
		osd_fclose(f);
	}
}



struct GameDriver pooyan_driver =
{
	"Pooyan",
	"pooyan",
	"Mike Cuddy (hardware info)\nAllard Van Der Bas (Pooyan emulator)\nNicola Salmoria (MAME driver)\nMarco Cassili",
	&machine_driver,

	pooyan_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	0/*TBR*/, input_ports, 0/*TBR*/, 0/*TBR*/, 0/*TBR*/,

	color_prom, 0, 0,
	ORIENTATION_ROTATE_270,

	hiload, hisave
};

struct GameDriver pootan_driver =
{
	"Pootan",
	"pootan",
	"Mike Cuddy (hardware info)\nAllard Van Der Bas (Pooyan emulator)\nNicola Salmoria (MAME driver)\nMarco Cassili",
	&machine_driver,

	pootan_rom,
	0, 0,
	0,
	0,	/* sound_prom */

	0/*TBR*/, input_ports, 0/*TBR*/, 0/*TBR*/, 0/*TBR*/,

	color_prom, 0, 0,
	ORIENTATION_ROTATE_270,

	hiload, hisave
};


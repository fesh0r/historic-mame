/***************************************************************************

  Dec0 Video emulation - Bryan McPhail, mish@tendril.force9.net

*********************************************************************
	Sprite data:  The unknown bits seem to be unused.

	Byte 0:
		Bit 0 : Y co-ord hi bit
    Bit 1,2: ?
		Bit 3,4 : Sprite height (1x, 2x, 4x, 8x)
		Bit 5  - X flip
		Bit 6  - Y flip
		Bit 7  - Only display Sprite if set
	Byte 1: Y-coords
	Byte 2:
		Bit 0,1,2,3: Hi bits of sprite number
		Bit 4,5,6,7: ?????
	Byte 3: Low bits of sprite number
	Byte 4:
		Bit 0 : X co-ords hi bit
		Bit 1,2,3: ? (can be set when enemy is killed - flash?)
		Bit 4,5,6,7:  - Colour
	Byte 5: X-coords

**********************************************************************

  Palette data

    0x000 - character palettes (Sprites on Midnight R)
    0x200 - sprite palettes (Characters on Midnight R)
    0x400 - tiles 1
  	0x600 - tiles 2

	  Bad Dudes, Robocop, Heavy Barrel, Hippodrome - 24 bit rgb
 		Sly Spy, Midnight Resistance - 12 bit rgb

  Tile data

  	4 bit palette select, 12 bit tile select

**********************************************************************

 Playfield 1 - 8*8 tiles
 Playfield 2 - 16*16 tiles
 Playfield 3 - 16*16 tiles

 Playfield control registers:
   bank 0:
   0: mostly unknown (82, 86, 8e...)
		bit 3 (0x4) set enables rowscroll (true for all games)
		bit 4 (0x8) set _disables_ colscroll!??! (see Heavy Barrel pf3)
		bit 8 (0x80) set in playfield 1 is reverse screen (set via dip-switch)
		bit 8 (0x80) in other playfields unknown
   2: unknown (00 in bg, 03 in fg+text - maybe controls pf transparency?)
   4: unknown (always 00)
   6: playfield shape: 00 = 4x1, 01 = 2x2, 02 = 1x4

   bank 1:
   0: horizontal scroll
   2: vertical scroll
   4: unknown (08 in Hippodrome, 05 in HB, 00 in the others)  (colscroll style?)
   6: Style of rowscroll (maybe only low 3 bits) (see below)

Rowscroll style - bank 1 register 6:
	0: 512 scroll registers (Robocop)
	3: 32 scroll registers (Heavy Barrel)
	4: 16 scroll registers (Bad Dudes, Sly Spy)
	5: ? (Hippodrome)
	7: 4 scroll registers (Heavy Barrel)
	8: 2 scroll registers (Heavy Barrel, used on other games but registers kept at 0)

Priority:
	Bit 0 set = Playfield 3 drawn over Playfield 2
			~ = Playfield 2 drawn over Playfield 3
	Bit 1 set = Sprites are drawn inbetween playfields
			~ = Sprites are on top of playfields
	Bit 2
	Bit 3 set = ...


***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"

//#define DE_DEBUG
//#define PRINT_PF_ATTRIBUTES

#define TEXTRAM_SIZE	0x2000	/* Size of text layer */
#define TILERAM_SIZE	0x800	/* Size of background and foreground */


/* Video */
unsigned char *dec0_sprite,*dec0_mem;
unsigned char *dec0_pf1_data,*dec0_pf2_data,*dec0_pf3_data;
static unsigned char *dec0_pf1_dirty,*dec0_pf3_dirty,*dec0_pf2_dirty;
static struct osd_bitmap *dec0_pf1_bitmap;
static int dec0_pf1_current_shape;
static struct osd_bitmap *dec0_pf2_bitmap;
static int dec0_pf2_current_shape;
static struct osd_bitmap *dec0_pf3_bitmap;
static int dec0_pf3_current_shape;

unsigned char *dec0_pf1_rowscroll,*dec0_pf2_rowscroll,*dec0_pf3_rowscroll;
unsigned char *dec0_pf1_colscroll,*dec0_pf2_colscroll,*dec0_pf3_colscroll;
static unsigned char dec0_pf1_control_0[8];
static unsigned char dec0_pf1_control_1[8];
static unsigned char dec0_pf2_control_0[8];
static unsigned char dec0_pf2_control_1[8];
static unsigned char dec0_pf3_control_0[8];
static unsigned char dec0_pf3_control_1[8];

static int dec0_pri;

/* Prototypes for this file */
void dec0_vh_stop (void);

int dec0_unknown1,dec0_unknown2; /* Temporary */

#ifdef PRINT_PF_ATTRIBUTES
static void printpfattributes(void)
{

	int i,j;
	char buf[20];
	int trueorientation;
	struct osd_bitmap *bitmap = Machine->scrbitmap;

	trueorientation = Machine->orientation;
	Machine->orientation = ORIENTATION_DEFAULT;

for (i = 0;i < 8;i+=2)
{
	sprintf(buf,"%04X",READ_WORD(&dec0_pf1_control_0[i]));
	for (j = 0;j < 4;j++)
		drawgfx(bitmap,Machine->uifont,buf[j],DT_COLOR_WHITE,0,0,3*8*i+8*j,8*2,0,TRANSPARENCY_NONE,0);
}
for (i = 0;i < 8;i+=2)
{
	sprintf(buf,"%04X",READ_WORD(&dec0_pf1_control_1[i]));
	for (j = 0;j < 4;j++)
		drawgfx(bitmap,Machine->uifont,buf[j],DT_COLOR_WHITE,0,0,3*8*i+8*j,8*3,0,TRANSPARENCY_NONE,0);
}
for (i = 0;i < 8;i+=2)
{
	sprintf(buf,"%04X",READ_WORD(&dec0_pf2_control_0[i]));
	for (j = 0;j < 4;j++)
		drawgfx(bitmap,Machine->uifont,buf[j],DT_COLOR_WHITE,0,0,3*8*i+8*j,8*5,0,TRANSPARENCY_NONE,0);
}
for (i = 0;i < 8;i+=2)
{
	sprintf(buf,"%04X",READ_WORD(&dec0_pf2_control_1[i]));
	for (j = 0;j < 4;j++)
		drawgfx(bitmap,Machine->uifont,buf[j],DT_COLOR_WHITE,0,0,3*8*i+8*j,8*6,0,TRANSPARENCY_NONE,0);
}
for (i = 0;i < 8;i+=2)
{
	sprintf(buf,"%04X",READ_WORD(&dec0_pf3_control_0[i]));
	for (j = 0;j < 4;j++)
		drawgfx(bitmap,Machine->uifont,buf[j],DT_COLOR_WHITE,0,0,3*8*i+8*j,8*8,0,TRANSPARENCY_NONE,0);
}
for (i = 0;i < 8;i+=2)
{
	sprintf(buf,"%04X",READ_WORD(&dec0_pf3_control_1[i]));
	for (j = 0;j < 4;j++)
		drawgfx(bitmap,Machine->uifont,buf[j],DT_COLOR_WHITE,0,0,3*8*i+8*j,8*9,0,TRANSPARENCY_NONE,0);
}
{
	sprintf(buf,"%04X",dec0_pri);
	for (j = 0;j < 4;j++)
		drawgfx(bitmap,Machine->uifont,buf[j],DT_COLOR_WHITE,0,0,8*j,8*11,0,TRANSPARENCY_NONE,0);
}
{
	sprintf(buf,"%04X",dec0_unknown1);
	for (j = 0;j < 4;j++)
		drawgfx(bitmap,Machine->uifont,buf[j],DT_COLOR_WHITE,0,0,8*j,8*12,0,TRANSPARENCY_NONE,0);
}
{
	sprintf(buf,"%04X",dec0_unknown2);
	for (j = 0;j < 4;j++)
		drawgfx(bitmap,Machine->uifont,buf[j],DT_COLOR_WHITE,0,0,8*j,8*13,0,TRANSPARENCY_NONE,0);
}
	Machine->orientation = trueorientation;

}
#endif





static void update_24bitcol(int offset)
{
	int r,g,b;


	r = (READ_WORD(&paletteram[offset]) >> 0) & 0xff;
	g = (READ_WORD(&paletteram[offset]) >> 8) & 0xff;
	b = (READ_WORD(&paletteram_2[offset]) >> 0) & 0xff;

	palette_change_color(offset / 2,r,g,b);
}

void dec0_paletteram_w_rg(int offset,int data)
{
	COMBINE_WORD_MEM(&paletteram[offset],data);
	update_24bitcol(offset);
}

void dec0_paletteram_w_b(int offset,int data)
{
	COMBINE_WORD_MEM(&paletteram_2[offset],data);
	update_24bitcol(offset);
}



/* pf23priority: 1 -> pf2 transparent, pf3 not transparent */
/*               0 -> pf2 not transparent, pf3 transparent */
static void dec0_update_palette(int pf23priority)
{
	int offs;
	int color,code,i;
	int colmask[16];
	int pal_base;


	memset(palette_used_colors,PALETTE_COLOR_UNUSED,Machine->drv->total_colors * sizeof(unsigned char));


	pal_base = Machine->drv->gfxdecodeinfo[0].color_codes_start;

	for (color = 0;color < 16;color++) colmask[color] = 0;

	for (offs = 0; offs < TEXTRAM_SIZE;offs += 2)
	{
		code = READ_WORD(&dec0_pf1_data[offs]);
		color = (code & 0xf000) >> 12;
		code &= 0x0fff;
		colmask[color] |= Machine->gfx[0]->pen_usage[code];
	}

	for (color = 0;color < 16;color++)
	{
		if (colmask[color] & (1 << 0))
			palette_used_colors[pal_base + 16 * color] = PALETTE_COLOR_TRANSPARENT;
		for (i = 1;i < 16;i++)
		{
			if (colmask[color] & (1 << i))
				palette_used_colors[pal_base + 16 * color + i] = PALETTE_COLOR_USED;
		}
	}


	pal_base = Machine->drv->gfxdecodeinfo[1].color_codes_start;

	for (color = 0;color < 16;color++) colmask[color] = 0;

	for (offs = 0; offs < TILERAM_SIZE;offs += 2)
	{
		code = READ_WORD(&dec0_pf2_data[offs]);
		color = (code & 0xf000) >> 12;
		code &= 0x0fff;
		colmask[color] |= Machine->gfx[1]->pen_usage[code];
	}

	for (color = 0;color < 16;color++)
	{
		if (colmask[color] & (1 << 0))
			palette_used_colors[pal_base + 16 * color] = pf23priority ? PALETTE_COLOR_USED : PALETTE_COLOR_TRANSPARENT;
		for (i = 1;i < 16;i++)
		{
			if (colmask[color] & (1 << i))
				palette_used_colors[pal_base + 16 * color + i] = PALETTE_COLOR_USED;
		}
	}


	pal_base = Machine->drv->gfxdecodeinfo[2].color_codes_start;

	for (color = 0;color < 16;color++) colmask[color] = 0;

	for (offs = 0; offs < TILERAM_SIZE;offs += 2)
	{
		code = READ_WORD(&dec0_pf3_data[offs]);
		color = (code & 0xf000) >> 12;
		code &= 0x0fff;
		colmask[color] |= Machine->gfx[2]->pen_usage[code];
	}

	for (color = 0;color < 16;color++)
	{
		if (colmask[color] & (1 << 0))
			palette_used_colors[pal_base + 16 * color] = pf23priority ? PALETTE_COLOR_TRANSPARENT : PALETTE_COLOR_USED;
		for (i = 1;i < 16;i++)
		{
			if (colmask[color] & (1 << i))
				palette_used_colors[pal_base + 16 * color + i] = PALETTE_COLOR_USED;
		}
	}


	pal_base = Machine->drv->gfxdecodeinfo[3].color_codes_start;

	for (color = 0;color < 16;color++) colmask[color] = 0;

	for (offs = 0;offs < 0x800;offs += 8)
	{
		int x,y,sprite,multi;


		y = READ_WORD(&dec0_sprite[offs]);
		if ((y&0x8000) == 0) continue;

		x = READ_WORD(&dec0_sprite[offs+4]);
		color = (x & 0xf000) >> 12;

		multi = (1 << ((y & 0x1800) >> 11)) - 1;	/* 1x, 2x, 4x, 8x height */
											/* multi = 0   1   3   7 */

		sprite = READ_WORD (&dec0_sprite[offs+2]) & 0x0fff;

		sprite &= ~multi;

		while (multi >= 0)
		{
			colmask[color] |= Machine->gfx[3]->pen_usage[sprite + multi];

			multi--;
		}
	}

	for (color = 0;color < 16;color++)
	{
		for (i = 1;i < 16;i++)
		{
			if (colmask[color] & (1 << i))
				palette_used_colors[pal_base + 16 * color + i] = PALETTE_COLOR_USED;
		}
	}


	if (palette_recalc())
	{
		memset(dec0_pf1_dirty,1,TEXTRAM_SIZE);
		memset(dec0_pf2_dirty,1,TILERAM_SIZE);
		memset(dec0_pf3_dirty,1,TILERAM_SIZE);
	}
}



static void dec0_drawsprites(struct osd_bitmap *bitmap,int pri_mask,int pri_val)
{
	int offs;

	for (offs = 0;offs < 0x800;offs += 8)
	{
		int x,y,sprite,colour,multi,fx,fy,inc;

		y = READ_WORD(&dec0_sprite[offs]);
		if ((y&0x8000) == 0) continue;

		x = READ_WORD(&dec0_sprite[offs+4]);
		colour = (x & 0xf000) >> 12;
		if ((colour & pri_mask) != pri_val) continue;

		fx = y & 0x2000;
		fy = y & 0x4000;
		multi = (1 << ((y & 0x1800) >> 11)) - 1;	/* 1x, 2x, 4x, 8x height */
											/* multi = 0   1   3   7 */

		sprite = READ_WORD (&dec0_sprite[offs+2]) & 0x0fff;

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 256) x -= 512;
		if (y >= 256) y -= 512;
		x = 240 - x;
		y = 240 - y;

		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		while (multi >= 0)
		{
			drawgfx(bitmap,Machine->gfx[3],
					sprite - multi * inc,
					colour,
					fx,fy,
					x,y - 16 * multi,
					&Machine->drv->visible_area,TRANSPARENCY_PEN,0);

			multi--;
		}
	}
}

static void dec0_pf1_update(void)
{
	int offs,mx,my,color,tile,quarter;
	int offsetx[4],offsety[4];

	switch (READ_WORD(&dec0_pf1_control_0[6]))
	{
		case 0:	/* 4x1 */
			offsetx[0] = 0;
			offsetx[1] = 256;
			offsetx[2] = 512;
			offsetx[3] = 768;
			offsety[0] = 0;
			offsety[1] = 0;
			offsety[2] = 0;
			offsety[3] = 0;
			if (dec0_pf1_current_shape != 0)
			{
				osd_free_bitmap(dec0_pf1_bitmap);
				dec0_pf1_bitmap = osd_create_bitmap(1024,256);
				dec0_pf1_current_shape = 0;
				memset(dec0_pf1_dirty,1,TEXTRAM_SIZE);
			}
			break;
		case 1:	/* 2x2 */
			offsetx[0] = 0;
			offsetx[1] = 0;
			offsetx[2] = 256;
			offsetx[3] = 256;
			offsety[0] = 0;
			offsety[1] = 256;
			offsety[2] = 0;
			offsety[3] = 256;
			if (dec0_pf1_current_shape != 1)
			{
				osd_free_bitmap(dec0_pf1_bitmap);
				dec0_pf1_bitmap = osd_create_bitmap(512,512);
				dec0_pf1_current_shape = 1;
				memset(dec0_pf1_dirty,1,TEXTRAM_SIZE);
			}
			break;
		case 2:	/* 1x4 */
			offsetx[0] = 0;
			offsetx[1] = 0;
			offsetx[2] = 0;
			offsetx[3] = 0;
			offsety[0] = 0;
			offsety[1] = 256;
			offsety[2] = 512;
			offsety[3] = 768;
			if (dec0_pf1_current_shape != 2)
			{
				osd_free_bitmap(dec0_pf1_bitmap);
				dec0_pf1_bitmap = osd_create_bitmap(256,1024);
				dec0_pf1_current_shape = 2;
				memset(dec0_pf1_dirty,1,TEXTRAM_SIZE);
			}
			break;
		default:
			if (errorlog) fprintf(errorlog,"error: pf1_update with unknown shape %04x\n",READ_WORD(&dec0_pf1_control_0[6]));
			return;
	}

	for (quarter = 0;quarter < 4;quarter++)
	{
		mx = -1;
		my = 0;

		for (offs = 0x800 * quarter; offs < 0x800 * quarter + 0x800;offs += 2)
		{
			mx++;
			if (mx == 32)
			{
				mx = 0;
				my++;
			}

			if (dec0_pf1_dirty[offs])
			{
				dec0_pf1_dirty[offs] = 0;
				tile = READ_WORD(&dec0_pf1_data[offs]);
				color = (tile & 0xf000) >> 12;

				drawgfx(dec0_pf1_bitmap,Machine->gfx[0],
						tile & 0x0fff,
						color,
						0,0,
						8*mx + offsetx[quarter],8*my + offsety[quarter],
						0,TRANSPARENCY_NONE,0);
			}
		}
	}
}

static void dec0_pf2_update(int transparent)
{
	int offs,mx,my,color,tile,quarter;
	int offsetx[4],offsety[4];
	static int last_transparent;

	if (transparent != last_transparent)
	{
		last_transparent = transparent;
		memset(dec0_pf2_dirty,1,TILERAM_SIZE);
	}

	switch (READ_WORD(&dec0_pf2_control_0[6]))
	{
		case 0:	/* 4x1 */
			offsetx[0] = 0;
			offsetx[1] = 256;
			offsetx[2] = 512;
			offsetx[3] = 768;
			offsety[0] = 0;
			offsety[1] = 0;
			offsety[2] = 0;
			offsety[3] = 0;
			if (dec0_pf2_current_shape != 0)
			{
				osd_free_bitmap(dec0_pf2_bitmap);
				dec0_pf2_bitmap = osd_create_bitmap(1024,256);
				dec0_pf2_current_shape = 0;
				memset(dec0_pf2_dirty,1,TILERAM_SIZE);
			}
			break;
		case 1:	/* 2x2 */
			offsetx[0] = 0;
			offsetx[1] = 0;
			offsetx[2] = 256;
			offsetx[3] = 256;
			offsety[0] = 0;
			offsety[1] = 256;
			offsety[2] = 0;
			offsety[3] = 256;
			if (dec0_pf2_current_shape != 1)
			{
				osd_free_bitmap(dec0_pf2_bitmap);
				dec0_pf2_bitmap = osd_create_bitmap(512,512);
				dec0_pf2_current_shape = 1;
				memset(dec0_pf2_dirty,1,TILERAM_SIZE);
			}
			break;
		case 2:	/* 1x4 */
			offsetx[0] = 0;
			offsetx[1] = 0;
			offsetx[2] = 0;
			offsetx[3] = 0;
			offsety[0] = 0;
			offsety[1] = 256;
			offsety[2] = 512;
			offsety[3] = 768;
			if (dec0_pf2_current_shape != 2)
			{
				osd_free_bitmap(dec0_pf2_bitmap);
				dec0_pf2_bitmap = osd_create_bitmap(256,1024);
				dec0_pf2_current_shape = 2;
				memset(dec0_pf2_dirty,1,TILERAM_SIZE);
			}
			break;
		default:
			if (errorlog) fprintf(errorlog,"error: pf2_update with unknown shape %04x\n",READ_WORD(&dec0_pf2_control_0[6]));
			return;
	}


	for (quarter = 0;quarter < 4;quarter++)
	{
		mx = -1;
		my = 0;

		for (offs = 0x200 * quarter; offs < 0x200 * quarter + 0x200;offs += 2)
		{
			mx++;
			if (mx == 16)
			{
				mx = 0;
				my++;
			}

			if (dec0_pf2_dirty[offs])
			{
				dec0_pf2_dirty[offs] = 0;
				tile = READ_WORD(&dec0_pf2_data[offs]);
				color = (tile & 0xf000) >> 12;

				drawgfx(dec0_pf2_bitmap,Machine->gfx[1],
						tile & 0x0fff,
						color,
						0,0,
						16*mx + offsetx[quarter],16*my + offsety[quarter],
						0,TRANSPARENCY_NONE,0);
			}
		}
	}
}

static void dec0_pf3_update(int transparent)
{
	int offs,mx,my,color,tile,quarter;
	int offsetx[4],offsety[4];
	static int last_transparent;

	if (transparent != last_transparent)
	{
		last_transparent = transparent;
		memset(dec0_pf3_dirty,1,TILERAM_SIZE);
	}

	switch (READ_WORD(&dec0_pf3_control_0[6]))
	{
		case 0:	/* 4x1 */
			offsetx[0] = 0;
			offsetx[1] = 256;
			offsetx[2] = 512;
			offsetx[3] = 768;
			offsety[0] = 0;
			offsety[1] = 0;
			offsety[2] = 0;
			offsety[3] = 0;
			if (dec0_pf3_current_shape != 0)
			{
				osd_free_bitmap(dec0_pf3_bitmap);
				dec0_pf3_bitmap = osd_create_bitmap(1024,256);
				dec0_pf3_current_shape = 0;
				memset(dec0_pf3_dirty,1,TILERAM_SIZE);
			}
			break;
		case 1:	/* 2x2 */
			offsetx[0] = 0;
			offsetx[1] = 0;
			offsetx[2] = 256;
			offsetx[3] = 256;
			offsety[0] = 0;
			offsety[1] = 256;
			offsety[2] = 0;
			offsety[3] = 256;
			if (dec0_pf3_current_shape != 1)
			{
				osd_free_bitmap(dec0_pf3_bitmap);
				dec0_pf3_bitmap = osd_create_bitmap(512,512);
				dec0_pf3_current_shape = 1;
				memset(dec0_pf3_dirty,1,TILERAM_SIZE);
			}
			break;
		case 2:	/* 1x4 */
			offsetx[0] = 0;
			offsetx[1] = 0;
			offsetx[2] = 0;
			offsetx[3] = 0;
			offsety[0] = 0;
			offsety[1] = 256;
			offsety[2] = 512;
			offsety[3] = 768;
			if (dec0_pf3_current_shape != 2)
			{
				osd_free_bitmap(dec0_pf3_bitmap);
				dec0_pf3_bitmap = osd_create_bitmap(256,1024);
				dec0_pf3_current_shape = 2;
				memset(dec0_pf3_dirty,1,TILERAM_SIZE);
			}
			break;
		default:
			if (errorlog) fprintf(errorlog,"error: pf3_update with unknown shape %04x\n",READ_WORD(&dec0_pf3_control_0[6]));
			return;
	}

	for (quarter = 0;quarter < 4;quarter++)
	{
		mx = -1;
		my = 0;

		for (offs = 0x200 * quarter; offs < 0x200 * quarter + 0x200;offs += 2)
		{
			mx++;
			if (mx == 16)
			{
				mx = 0;
				my++;
			}

			if (dec0_pf3_dirty[offs])
			{
				dec0_pf3_dirty[offs] = 0;
				tile = READ_WORD(&dec0_pf3_data[offs]);
				color = (tile & 0xf000) >> 12;

				drawgfx(dec0_pf3_bitmap,Machine->gfx[2],
						tile & 0x0fff,
						color,
						0,0,
						16*mx + offsetx[quarter],16*my + offsety[quarter],
						0,TRANSPARENCY_NONE,0);
			}
		}
	}
}

/******************************************************************************/

void dec0_pf1_draw(struct osd_bitmap *bitmap)
{
	int offs;

	/* We check for column scroll and use that if needed, otherwise use row scroll,
	   I am 99% sure they are never needed at same time ;) */
	if (READ_WORD(&dec0_pf1_colscroll[0])) /* This is NOT a good check for col scroll, I can't find real bit */
	{
		int cscrolly[64];
		int scrollx;

		scrollx = -READ_WORD(&dec0_pf1_control_1[0]);

		for (offs = 0;offs < 32;offs++)
		cscrolly[offs] = -READ_WORD(&dec0_pf1_control_1[2]) - READ_WORD(&dec0_pf1_colscroll[2*offs]);

		copyscrollbitmap(bitmap,dec0_pf1_bitmap,1,&scrollx,32,cscrolly,&Machine->drv->visible_area,TRANSPARENCY_PEN,palette_transparent_pen);
	}
	/* Row scroll enable bit (unsure if this enables/disables col scroll too) */
	else if (READ_WORD(&dec0_pf1_control_0[0])&0x4)
	{
		int rscrollx[512],scrolly;
		scrolly = -READ_WORD(&dec0_pf1_control_1[2]);

		switch (READ_WORD(&dec0_pf1_control_1[6]))
		{
			case 8: /* Appears to be no row-scroll - so maybe only bottom 3 bits are style */
				break;

			case 4: /* 16 horizontal scroll registers (Bad Dudes, Sly Spy) */
				for (offs = 0;offs < 16;offs++)
				rscrollx[offs] = -READ_WORD(&dec0_pf1_control_1[0]) - READ_WORD(&dec0_pf1_rowscroll[2*offs]);
				copyscrollbitmap(bitmap,dec0_pf1_bitmap,16,rscrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_PEN,palette_transparent_pen);
				break;

			case 3: /* 32 horizontal scroll registers (Heavy Barrel title screen) */
				for (offs = 0;offs < 32;offs++)
				rscrollx[offs] = -READ_WORD(&dec0_pf1_control_1[0]) - READ_WORD(&dec0_pf1_rowscroll[2*offs]);
				copyscrollbitmap(bitmap,dec0_pf1_bitmap,32,rscrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_PEN,palette_transparent_pen);
				break;

			case 0: /* 512 horizontal scroll registers (Robocop) */
				for (offs = 0;offs < 512;offs++)
				rscrollx[offs] = -READ_WORD(&dec0_pf1_control_1[0]) - READ_WORD(&dec0_pf1_rowscroll[2*offs]);
				copyscrollbitmap(bitmap,dec0_pf1_bitmap,512,rscrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_PEN,palette_transparent_pen);
				break;

			default:
				if (errorlog) fprintf(errorlog,"Warning: Unknown row scroll type %d selected\n",READ_WORD(&dec0_pf1_control_1[6]));
		}
	}
	else /* Scroll registers not enabled */
	{
		int scrollx,scrolly;

		scrollx = -READ_WORD(&dec0_pf1_control_1[0]);
		scrolly = -READ_WORD(&dec0_pf1_control_1[2]);
		copyscrollbitmap(bitmap,dec0_pf1_bitmap,1,&scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_PEN,palette_transparent_pen);
	}
}

/******************************************************************************/

void dec0_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh)
{
	int offs,xscroll_f,yscroll_f,xscroll_b,yscroll_b;


	/* WARNING: priority inverted wrt all the other games */
	dec0_update_palette(~dec0_pri & 0x01);

	/* Scroll positions */
	xscroll_b=READ_WORD(&dec0_pf3_control_1[0]);
	yscroll_b=READ_WORD(&dec0_pf3_control_1[2]);
	xscroll_f=READ_WORD(&dec0_pf2_control_1[0]);
	yscroll_f=READ_WORD(&dec0_pf2_control_1[2]);

	dec0_pf1_update();


/* WARNING: inverted wrt Midnight Resistance */
if ((dec0_pri & 0x01) == 0)
{
	int scrollx[32];
	int scrolly;


	dec0_pf2_update(0);
	dec0_pf3_update(1);

    /* Foreground */
	for (offs = 0;offs < 32;offs++)
		scrollx[offs] = -xscroll_f;
	if (READ_WORD(&dec0_pf2_control_1[6]) == 0x04)
	{
		for (offs = 0;offs < 16;offs++)
			scrollx[offs] -= READ_WORD(&dec0_pf2_rowscroll[2*offs]);
	}
   	scrolly=-yscroll_f;
	copyscrollbitmap(bitmap,dec0_pf2_bitmap,32,scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_NONE,0);

    /* Background */
	for (offs = 0;offs < 32;offs++)
		scrollx[offs] = -xscroll_b;
	if (READ_WORD(&dec0_pf3_control_1[6]) == 0x04)
	{
		for (offs = 0;offs < 16;offs++)
			scrollx[offs] -= READ_WORD(&dec0_pf3_rowscroll[2*offs]);
	}
   	scrolly=-yscroll_b;
    copyscrollbitmap(bitmap,dec0_pf3_bitmap,32,scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_PEN,palette_transparent_pen);

	/* I'm not supporting sprite priority because I can't figure out how it works. */
	/* I would exepct e.g. the trees to cover the sprites in level 4 (forest) */
	dec0_drawsprites(bitmap,0x00,0x00);
}
else
{
	int scrollx[32];
	int scrolly;


	dec0_pf3_update(0);
	dec0_pf2_update(1);

    /* Background */
	for (offs = 0;offs < 32;offs++)
		scrollx[offs] = -xscroll_b;
	if (READ_WORD(&dec0_pf3_control_1[6]) == 0x04)
	{
		for (offs = 0;offs < 16;offs++)
			scrollx[offs] -= READ_WORD(&dec0_pf3_rowscroll[2*offs]);
	}
   	scrolly=-yscroll_b;
    copyscrollbitmap(bitmap,dec0_pf3_bitmap,32,scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_NONE,0);

    /* Foreground */
	for (offs = 0;offs < 32;offs++)
		scrollx[offs] = -xscroll_f;
	if (READ_WORD(&dec0_pf2_control_1[6]) == 0x04)
	{
		for (offs = 0;offs < 16;offs++)
			scrollx[offs] -= READ_WORD(&dec0_pf2_rowscroll[2*offs]);
	}
   	scrolly=-yscroll_f;
	copyscrollbitmap(bitmap,dec0_pf2_bitmap,32,scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_PEN,palette_transparent_pen);

	/* I'm not supporting sprite priority because I can't figure out how it works. */
	/* I would exepct e.g. the trees to cover the sprites in level 4 (forest) */
	dec0_drawsprites(bitmap,0x00,0x00);
}

	dec0_pf1_draw(bitmap);

  #ifdef PRINT_PF_ATTRIBUTES
	printpfattributes();
  #endif
}

/******************************************************************************/

void robocop_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh)
{
	int xscroll_f,yscroll_f,xscroll_b,yscroll_b;
	int scrollx,scrolly;


	dec0_update_palette(dec0_pri & 0x01);

	/* Scroll positions */
	xscroll_b=READ_WORD (&dec0_pf3_control_1[0]);
	yscroll_b=READ_WORD (&dec0_pf3_control_1[2]);
	xscroll_f=READ_WORD (&dec0_pf2_control_1[0]);
	yscroll_f=READ_WORD (&dec0_pf2_control_1[2]);

	dec0_pf1_update();

if (dec0_pri & 0x01)
{
	int trans;


	dec0_pf2_update(0);
	dec0_pf3_update(1);

	/* WARNING: inverted wrt Midnight Resistance */
	/* Robocop uses it only for the title screen, so this might be just */
	/* completely wrong. The top 8 bits of the register might mean */
	/* something (they are 0x80 in midres, 0x00 here) */
	if (dec0_pri & 0x04)
		trans = 0x08;
	else trans = 0x00;

    /* Foreground */
    scrollx=-xscroll_f;
   	scrolly=-yscroll_f;
   	copyscrollbitmap(bitmap,dec0_pf2_bitmap,1,&scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_NONE,0);

	if (dec0_pri & 0x02)
		dec0_drawsprites(bitmap,0x08,trans);

    /* Background */
    scrollx=-xscroll_b;
   	scrolly=-yscroll_b;
    copyscrollbitmap(bitmap,dec0_pf3_bitmap,1,&scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_PEN,palette_transparent_pen);

	if (dec0_pri & 0x02)
		dec0_drawsprites(bitmap,0x08,trans ^ 0x08);
	else
		dec0_drawsprites(bitmap,0x00,0x00);
}
else
{
	int trans;


	dec0_pf3_update(0);
	dec0_pf2_update(1);

	/* WARNING: inverted wrt Midnight Resistance */
	/* Robocop uses it only for the title screen, so this might be just */
	/* completely wrong. The top 8 bits of the register might mean */
	/* something (they are 0x80 in midres, 0x00 here) */
	if (dec0_pri & 0x04)
		trans = 0x08;
	else trans = 0x00;

    /* Background */
    scrollx=-xscroll_b;
   	scrolly=-yscroll_b;
    copyscrollbitmap(bitmap,dec0_pf3_bitmap,1,&scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_NONE,0);

	if (dec0_pri & 0x02)
		dec0_drawsprites(bitmap,0x08,trans);

    /* Foreground */
    scrollx=-xscroll_f;
   	scrolly=-yscroll_f;
   	copyscrollbitmap(bitmap,dec0_pf2_bitmap,1,&scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_PEN,palette_transparent_pen);

	if (dec0_pri & 0x02)
		dec0_drawsprites(bitmap,0x08,trans ^ 0x08);
	else
		dec0_drawsprites(bitmap,0x00,0x00);
}

	dec0_pf1_draw(bitmap);


  #ifdef PRINT_PF_ATTRIBUTES
	printpfattributes();
  #endif
}

/******************************************************************************/

void hbarrel_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh)
{
	int xscroll_f,yscroll_f,xscroll_b,yscroll_b;
	int scrollx,scrolly;


	dec0_update_palette(dec0_pri & 0x01);

  /* Scroll positions */
  xscroll_b=READ_WORD (&dec0_pf3_control_1[0]);
  yscroll_b=READ_WORD (&dec0_pf3_control_1[2]);
  xscroll_f=READ_WORD (&dec0_pf2_control_1[0]);
  yscroll_f=READ_WORD (&dec0_pf2_control_1[2]);

	dec0_pf1_update();
	dec0_pf3_update(0);
	dec0_pf2_update(1);

  /* Background layer */
  scrollx=-xscroll_b;
 	scrolly=-yscroll_b;

	/* Row scroll enable bit */
	if (READ_WORD(&dec0_pf3_control_0[0])&0x4) {
  	int rscrollx[8],offs;

  	/* 2 registers, bitmap is twice size of screen so we double rowscroll usage */
		if (READ_WORD(&dec0_pf2_control_1[6]) == 0x08)
		{
			for (offs = 0;offs < 2;offs++) {
				rscrollx[offs]   =  - READ_WORD(&dec0_pf3_rowscroll[2*offs]) - READ_WORD(&dec0_pf3_control_1[0]);
        rscrollx[offs+2] =  - READ_WORD(&dec0_pf3_rowscroll[2*offs]) - READ_WORD(&dec0_pf3_control_1[0]);
      }

			copyscrollbitmap(bitmap,dec0_pf3_bitmap,4,rscrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_NONE,0);
		}

    /* 4 registers */
    if (READ_WORD(&dec0_pf2_control_1[6]) == 0x07)
		{
			for (offs = 0;offs < 4;offs++) {
				rscrollx[offs]   = - READ_WORD(&dec0_pf3_rowscroll[2*offs]) - READ_WORD(&dec0_pf3_control_1[0]);
        rscrollx[offs+4] = - READ_WORD(&dec0_pf3_rowscroll[2*offs]) - READ_WORD(&dec0_pf3_control_1[0]);
      }

			copyscrollbitmap(bitmap,dec0_pf3_bitmap,8,rscrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_NONE,0);
		}
 	}
  else
		copyscrollbitmap(bitmap,dec0_pf3_bitmap,1,&scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_NONE,0);

  /* Foreground */
  scrollx=-xscroll_f;
	scrolly=-yscroll_f;
	copyscrollbitmap(bitmap,dec0_pf2_bitmap,1,&scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_PEN,palette_transparent_pen);

  /* HB always keeps pf2 on top of pf3, no need explicitly support priority register */

	dec0_drawsprites(bitmap,0x00,0x00);
	dec0_pf1_draw(bitmap);

  #ifdef PRINT_PF_ATTRIBUTES
	printpfattributes();
  #endif
}

void hippodrm_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh)
{
	int xscroll_f,yscroll_f,xscroll_b,yscroll_b;


	dec0_update_palette(dec0_pri & 0x01);

	/* Scroll positions */
	xscroll_b=READ_WORD(&dec0_pf3_control_1[0]);
	yscroll_b=READ_WORD(&dec0_pf3_control_1[2]);
	xscroll_f=READ_WORD(&dec0_pf2_control_1[0]);
	yscroll_f=READ_WORD(&dec0_pf2_control_1[2]);

	dec0_pf1_update();
	dec0_pf2_update(0);

   {
    int scrollx;
    int scrolly;

    scrollx=-xscroll_f;
   	scrolly=-yscroll_f;
   	copyscrollbitmap(bitmap,dec0_pf2_bitmap,1,&scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_NONE,0);
   }


	dec0_drawsprites(bitmap,0x00,0x00);
	dec0_pf1_draw(bitmap);

  #ifdef PRINT_PF_ATTRIBUTES
	printpfattributes();
  #endif
}

/******************************************************************************/

void midres_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh)
{
  int xscroll_f,yscroll_f,xscroll_b,yscroll_b;


	dec0_update_palette(dec0_pri & 0x01);

  /* Scroll positions */
  xscroll_b=READ_WORD (&dec0_pf3_control_1[0]);
  yscroll_b=READ_WORD (&dec0_pf3_control_1[2]);
  xscroll_f=READ_WORD (&dec0_pf2_control_1[0]);
  yscroll_f=READ_WORD (&dec0_pf2_control_1[2]);

	dec0_pf1_update();


   {
    int scrollx;
    int scrolly;

if (dec0_pri & 0x01)
{
	int trans;


	dec0_pf2_update(0);
	dec0_pf3_update(1);

	if (dec0_pri & 0x04)
		trans = 0x00;
	else trans = 0x08;

    /* Foreground */
    scrollx=-xscroll_f;
   	scrolly=-yscroll_f;
   	copyscrollbitmap(bitmap,dec0_pf2_bitmap,1,&scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_NONE,0);

	if (dec0_pri & 0x02)
		dec0_drawsprites(bitmap,0x08,trans);

    /* Background */
    scrollx=-xscroll_b;
   	scrolly=-yscroll_b;
    copyscrollbitmap(bitmap,dec0_pf3_bitmap,1,&scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_PEN,palette_transparent_pen);

	if (dec0_pri & 0x02)
		dec0_drawsprites(bitmap,0x08,trans ^ 0x08);
	else
		dec0_drawsprites(bitmap,0x00,0x00);
}
else
{
	int trans;


	dec0_pf3_update(0);
	dec0_pf2_update(1);

	if (dec0_pri & 0x04)
		trans = 0x00;
	else trans = 0x08;

    /* Background */
    scrollx=-xscroll_b;
   	scrolly=-yscroll_b;
    copyscrollbitmap(bitmap,dec0_pf3_bitmap,1,&scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_NONE,0);

	if (dec0_pri & 0x02)
		dec0_drawsprites(bitmap,0x08,trans);

    /* Foreground */
    scrollx=-xscroll_f;
   	scrolly=-yscroll_f;
   	copyscrollbitmap(bitmap,dec0_pf2_bitmap,1,&scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_PEN,palette_transparent_pen);

	if (dec0_pri & 0x02)
		dec0_drawsprites(bitmap,0x08,trans ^ 0x08);
	else
		dec0_drawsprites(bitmap,0x00,0x00);
}
   }


	dec0_pf1_draw(bitmap);

  #ifdef PRINT_PF_ATTRIBUTES
	printpfattributes();
  #endif
}

/******************************************************************************/

void slyspy_vh_screenrefresh(struct osd_bitmap *bitmap,int full_refresh)
{
	int offs,xscroll_f,yscroll_f,xscroll_b,yscroll_b;


	dec0_update_palette(dec0_pri & 0x01);

	/* Scroll positions */
	xscroll_b=READ_WORD (&dec0_pf3_control_1[0]);
	yscroll_b=READ_WORD (&dec0_pf3_control_1[2]);
	xscroll_f=READ_WORD (&dec0_pf2_control_1[0]);
	yscroll_f=READ_WORD (&dec0_pf2_control_1[2]);

	dec0_pf1_update();
	dec0_pf3_update(0);
	dec0_pf2_update(1);

	{
		int scrollx;
		int scrolly;

		/* Background area */
		scrollx=-xscroll_b;
		scrolly=-yscroll_b;
		copyscrollbitmap(bitmap,dec0_pf3_bitmap,1,&scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_NONE,0);

		/* Foreground area */
		scrolly=-yscroll_f;

    /* Row scroll enable bit */
  	if (READ_WORD(&dec0_pf2_control_0[0])&0x4)
  	{
			int rscrollx[16];

      /* Sly Spy always uses style 4 - 16 registers */
			for (offs = 0;offs < 16;offs++)
				rscrollx[offs] = -xscroll_f - READ_WORD(&dec0_pf2_rowscroll[2*offs]);
			copyscrollbitmap(bitmap,dec0_pf2_bitmap,16,rscrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_PEN,palette_transparent_pen);
		}
		else
		{
			scrollx = -xscroll_f;
			copyscrollbitmap(bitmap,dec0_pf2_bitmap,1,&scrollx,1,&scrolly,&Machine->drv->visible_area,TRANSPARENCY_PEN,palette_transparent_pen);
		}
	}

	dec0_drawsprites(bitmap,0x00,0x00);
	dec0_pf1_draw(bitmap);

  #ifdef PRINT_PF_ATTRIBUTES
	printpfattributes();
  #endif
}



/******************************************************************************/

void dec0_pf1_control_0_w(int offset,int data)
{
	COMBINE_WORD_MEM(&dec0_pf1_control_0[offset],data);
}

void dec0_pf1_control_1_w(int offset,int data)
{
	COMBINE_WORD_MEM(&dec0_pf1_control_1[offset],data);
}

void dec0_pf1_rowscroll_w(int offset,int data)
{
	COMBINE_WORD_MEM(&dec0_pf1_rowscroll[offset],data);
}

void dec0_pf1_colscroll_w(int offset,int data)
{
	COMBINE_WORD_MEM(&dec0_pf1_colscroll[offset],data);
}

void dec0_pf1_data_w(int offset,int data)
{
	int oldword = READ_WORD(&dec0_pf1_data[offset]);
	int newword = COMBINE_WORD(oldword,data);

	if (oldword != newword)
	{
		WRITE_WORD(&dec0_pf1_data[offset],newword);
		dec0_pf1_dirty[offset] = 1;
	}
}

int dec0_pf1_data_r(int offset)
{
	return READ_WORD(&dec0_pf1_data[offset]);
}

void dec0_pf2_control_0_w(int offset,int data)
{
	COMBINE_WORD_MEM(&dec0_pf2_control_0[offset],data);
}

void dec0_pf2_control_1_w(int offset,int data)
{
	COMBINE_WORD_MEM(&dec0_pf2_control_1[offset],data);
}

void dec0_pf2_rowscroll_w(int offset,int data)
{
	COMBINE_WORD_MEM(&dec0_pf2_rowscroll[offset],data);
}

void dec0_pf2_colscroll_w(int offset,int data)
{
	COMBINE_WORD_MEM(&dec0_pf2_colscroll[offset],data);
}

void dec0_pf2_data_w(int offset,int data)
{
	int oldword = READ_WORD(&dec0_pf2_data[offset]);
	int newword = COMBINE_WORD(oldword,data);

	if (oldword != newword)
	{
		WRITE_WORD(&dec0_pf2_data[offset],newword);
		dec0_pf2_dirty[offset] = 1;
	}
}

int dec0_pf2_data_r(int offset)
{
	return READ_WORD(&dec0_pf2_data[offset]);
}

void dec0_pf3_control_0_w(int offset,int data)
{
	COMBINE_WORD_MEM(&dec0_pf3_control_0[offset],data);
}

void dec0_pf3_control_1_w(int offset,int data)
{
	COMBINE_WORD_MEM(&dec0_pf3_control_1[offset],data);
}

void dec0_pf3_rowscroll_w(int offset,int data)
{
	COMBINE_WORD_MEM(&dec0_pf3_rowscroll[offset],data);
}

void dec0_pf3_colscroll_w(int offset,int data)
{
	COMBINE_WORD_MEM(&dec0_pf3_colscroll[offset],data);
}

int dec0_pf3_colscroll_r(int offset)
{
	return READ_WORD(&dec0_pf3_colscroll[offset]);
}

void dec0_pf3_data_w(int offset,int data)
{
	int oldword = READ_WORD(&dec0_pf3_data[offset]);
	int newword = COMBINE_WORD(oldword,data);

	if (oldword != newword)
	{
		WRITE_WORD(&dec0_pf3_data[offset],newword);
		dec0_pf3_dirty[offset] = 1;
	}
}

int dec0_pf3_data_r(int offset)
{
	return READ_WORD(&dec0_pf3_data[offset]);
}

void dec0_priority_w(int offset,int data)
{
	if (offset == 0)
  	dec0_pri = data;
	else
		if (errorlog) fprintf(errorlog,"PC %06x write %02x to priority offset %d\n",cpu_getpc(),data,offset);
}

/* Nice 'wrap-around' patch which fixes sprite memory problems :) */
void hippo_sprite_fix(int offset, int data)
{
	WRITE_WORD (&dec0_sprite[offset], data);
}

/******************************************************************************/

int dec0_vh_start (void)
{
	/* Allocate bitmaps */
	if ((dec0_pf1_bitmap = osd_create_bitmap(512,512)) == 0) {
		dec0_vh_stop ();
		return 1;
	}
	dec0_pf1_current_shape = 1;

	if ((dec0_pf2_bitmap = osd_create_bitmap(512,512)) == 0) {
		dec0_vh_stop ();
		return 1;
	}
	dec0_pf2_current_shape = 1;

	if ((dec0_pf3_bitmap = osd_create_bitmap(512,512)) == 0) {
		dec0_vh_stop ();
		return 1;
	}
	dec0_pf3_current_shape = 1;

	dec0_pf1_dirty = malloc(TEXTRAM_SIZE);
	dec0_pf3_dirty = malloc(TILERAM_SIZE);
	dec0_pf2_dirty = malloc(TILERAM_SIZE);

	memset(dec0_pf1_dirty,1,TEXTRAM_SIZE);
	memset(dec0_pf2_dirty,1,TILERAM_SIZE);
	memset(dec0_pf3_dirty,1,TILERAM_SIZE);

	return 0;
}

/******************************************************************************/

void dec0_vh_stop (void)
{
	#ifdef DE_DEBUG
	int i;

  FILE *fp;
  fp=fopen("robo_hi.ram","wb");
  fwrite(dec0_sprite,1,0x1000,fp);
  fclose(fp);

  fp=fopen("video.ram","wb");
  fwrite(dec0_pf1_data,1,0x2000,fp);
  fclose(fp);

  fp=fopen("fore.ram","wb");
  fwrite(dec0_pf2_data,1,0x800,fp);
  fclose(fp);

  fp=fopen("back.ram","wb");
  fwrite(dec0_pf3_data,1,0x800,fp);
  fclose(fp);

  fp=fopen("pal_rg.ram","wb");
  fwrite(paletteram,1,0x800,fp);
  fclose(fp);

  fp=fopen("pal_b.ram","wb");
  fwrite(paletteram2,1,0x800,fp);
  fclose(fp);

  fp=fopen("system.rom","wb");
  fwrite(RAM,1,0x60000,fp);
  fclose(fp);

  fp=fopen("system.ram","wb");
  fwrite(dec0_mem,1,0x4000,fp);
  fclose(fp);

  fp=fopen("color.txt","wb");

  fclose(fp);

  #endif

	osd_free_bitmap(dec0_pf2_bitmap);
	osd_free_bitmap(dec0_pf3_bitmap);
	osd_free_bitmap(dec0_pf1_bitmap);
	free(dec0_pf3_dirty);
	free(dec0_pf2_dirty);
	free(dec0_pf1_dirty);

  generic_vh_stop();
}

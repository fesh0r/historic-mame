/***************************************************************************

	vidhrdw.c

	Functions to emulate the video hardware of the machine.

	There are only a few differences between the video hardware of Mysterious
	Stones and Mat Mania. The tile bank select bit is different and the sprite
	selection seems to be different as well. Additionally, the palette is stored
	differently. I'm also not sure that the 2nd tile page is really used in
	Mysterious Stones.

***************************************************************************/

#include "driver.h"
#include "vidhrdw/generic.h"



unsigned char *mystston_videoram2,*mystston_colorram2;
int mystston_videoram2_size;
unsigned char *mystston_videoram3,*mystston_colorram3;
int mystston_videoram3_size;
unsigned char *mystston_scroll;
unsigned char *mystston_paletteram;
static struct osd_bitmap *tmpbitmap2;
static unsigned char *dirtybuffer2;

unsigned char *matmania_pageselect;


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Actually Mysterious Stones uses RAM, not PROMs to store the palette.
  I don't know for sure how the palette RAM is connected to the RGB output,
  but it's probably the usual:

  bit 7 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
  bit 0 -- 1  kohm resistor  -- RED

***************************************************************************/
void mystston_vh_convert_color_prom(unsigned char *palette, unsigned char *colortable,const unsigned char *color_prom)
{
	int i;


	/* the palette will be initialized by the game. We just set it to some */
	/* pre-cooked values so the startup copyright notice can be displayed. */
	for (i = 0;i < Machine->drv->total_colors;i++)
	{
		*(palette++) = ((i & 1) >> 0) * 0xff;
		*(palette++) = ((i & 2) >> 1) * 0xff;
		*(palette++) = ((i & 4) >> 2) * 0xff;
	}

	/* initialize the color table */
	for (i = 0;i < Machine->drv->color_table_len;i++)
		colortable[i] = i;
}



/***************************************************************************

  Convert the color PROMs into a more useable format.

  Mat Mania is unusual in that it has both PROMs and RAM to control the
  palette. PROMs are used for characters and background tiles, RAM for
  sprites.
  I don't know for sure how the PROMs are connected to the RGB output,
  but it's probably the usual:

  bit 7 -- 220 ohm resistor  -- GREEN
        -- 470 ohm resistor  -- GREEN
        -- 1  kohm resistor  -- GREEN
        -- 2.2kohm resistor  -- GREEN
        -- 220 ohm resistor  -- RED
        -- 470 ohm resistor  -- RED
        -- 1  kohm resistor  -- RED
  bit 0 -- 2.2kohm resistor  -- RED

  bit 3 -- 220 ohm resistor  -- BLUE
        -- 470 ohm resistor  -- BLUE
        -- 1  kohm resistor  -- BLUE
  bit 0 -- 2.2kohm resistor  -- BLUE

***************************************************************************/
void matmania_vh_convert_color_prom(unsigned char *palette, unsigned char *colortable,const unsigned char *color_prom)
{
	int i;
	#define TOTAL_COLORS(gfxn) (Machine->gfx[gfxn]->total_colors * Machine->gfx[gfxn]->color_granularity)
	#define COLOR(gfxn,offs) (colortable[Machine->drv->gfxdecodeinfo[gfxn].color_codes_start + offs])


	for (i = 0;i < 64;i++)
	{
		int bit0,bit1,bit2,bit3;


		bit0 = (color_prom[0] >> 0) & 0x01;
		bit1 = (color_prom[0] >> 1) & 0x01;
		bit2 = (color_prom[0] >> 2) & 0x01;
		bit3 = (color_prom[0] >> 3) & 0x01;
		*(palette++) = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[0] >> 4) & 0x01;
		bit1 = (color_prom[0] >> 5) & 0x01;
		bit2 = (color_prom[0] >> 6) & 0x01;
		bit3 = (color_prom[0] >> 7) & 0x01;
		*(palette++) = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (color_prom[64] >> 0) & 0x01;
		bit1 = (color_prom[64] >> 1) & 0x01;
		bit2 = (color_prom[64] >> 2) & 0x01;
		bit3 = (color_prom[64] >> 3) & 0x01;
		*(palette++) = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		color_prom++;
	}

	/* the palette will be initialized by the game. We just set it to some */
	/* pre-cooked values so the startup copyright notice can be displayed. */
	for (i = 64;i < Machine->drv->total_colors;i++)
	{
		*(palette++) = ((i & 1) >> 0) * 0xff;
		*(palette++) = ((i & 2) >> 1) * 0xff;
		*(palette++) = ((i & 4) >> 2) * 0xff;
	}


	/* characters use colors 0-31 */
	for (i = 0;i < TOTAL_COLORS(0);i++)
		COLOR(0,i) = i;

	/* tiles use colors 32-63 */
	for (i = 0;i < TOTAL_COLORS(1);i++)
		COLOR(1,i) = i + 32;

	/* sprites use colors 64-79 */
	for (i = 0;i < TOTAL_COLORS(2);i++)
		COLOR(2,i) = i + 64;
}



/***************************************************************************

  Start the video hardware emulation.

***************************************************************************/
int mystston_vh_start(void)
{
	if ((dirtybuffer = malloc(videoram_size)) == 0)
		return 1;
	memset(dirtybuffer,1,videoram_size);

	if ((dirtybuffer2 = malloc(mystston_videoram3_size)) == 0)
	{
		free(dirtybuffer);
		return 1;
	}
	memset(dirtybuffer2,1,mystston_videoram3_size);

	/* Mysterious Stones has a virtual screen twice as large as the visible screen */
	if ((tmpbitmap = osd_create_bitmap(2 * Machine->drv->screen_width,Machine->drv->screen_height)) == 0)
	{
		free(dirtybuffer);
		free(dirtybuffer2);
		return 1;
	}

	/* Mysterious Stones has a virtual screen twice as large as the visible screen */
	if ((tmpbitmap2 = osd_create_bitmap(2 * Machine->drv->screen_width,Machine->drv->screen_height)) == 0)
	{
		free(tmpbitmap);
		free(dirtybuffer);
		free(dirtybuffer2);
		return 1;
	}

	return 0;
}



/***************************************************************************

  Stop the video hardware emulation.

***************************************************************************/
void mystston_vh_stop(void)
{
	free(dirtybuffer);
	free(dirtybuffer2);
	osd_free_bitmap(tmpbitmap);
	osd_free_bitmap(tmpbitmap2);
}


void mystston_videoram3_w(int offset,int data)
{
	if (mystston_videoram3[offset] != data)
	{
		dirtybuffer2[offset] = 1;

		mystston_videoram3[offset] = data;
	}
}



void mystston_colorram3_w(int offset,int data)
{
	if (mystston_colorram3[offset] != data)
	{
		dirtybuffer2[offset] = 1;

		mystston_colorram3[offset] = data;
	}
}

void mystston_paletteram_w(int offset,int data)
{
	int bit0,bit1,bit2;
	int r,g,b;

	mystston_paletteram[offset] = data;

	bit0 = (data >> 0) & 0x01;
	bit1 = (data >> 1) & 0x01;
	bit2 = (data >> 2) & 0x01;
	r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	bit0 = (data >> 3) & 0x01;
	bit1 = (data >> 4) & 0x01;
	bit2 = (data >> 5) & 0x01;
	g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	bit0 = 0;
	bit1 = (data >> 6) & 0x01;
	bit2 = (data >> 7) & 0x01;
	b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

	osd_modify_pen(Machine->pens[offset],r,g,b);
}

void matmania_paletteram_w(int offset,int data)
{
	int bit0,bit1,bit2,bit3,val;
	int r,g,b;
	int offs2;


	mystston_paletteram[offset] = data;
	offs2 = offset & 0x0f;

	val = mystston_paletteram[offs2];
	bit0 = (val >> 0) & 0x01;
	bit1 = (val >> 1) & 0x01;
	bit2 = (val >> 2) & 0x01;
	bit3 = (val >> 3) & 0x01;
	r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	val = mystston_paletteram[offs2 | 0x10];
	bit0 = (val >> 0) & 0x01;
	bit1 = (val >> 1) & 0x01;
	bit2 = (val >> 2) & 0x01;
	bit3 = (val >> 3) & 0x01;
	g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	val = mystston_paletteram[offs2 | 0x20];
	bit0 = (val >> 0) & 0x01;
	bit1 = (val >> 1) & 0x01;
	bit2 = (val >> 2) & 0x01;
	bit3 = (val >> 3) & 0x01;
	b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

	osd_modify_pen(Machine->pens[offs2 + 64],r,g,b);
}



/***************************************************************************

  Draw the game screen in the given osd_bitmap.
  Do NOT call osd_update_display() from this function, it will be called by
  the main emulation engine.

***************************************************************************/
void mystston_vh_screenrefresh(struct osd_bitmap *bitmap)
{
	int offs;

	/* for every character in the Video RAM, check if it has been modified */
	/* since last time and update it accordingly. */
	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		if (dirtybuffer[offs])
		{
			int sx,sy;


			dirtybuffer[offs] = 0;

			sx = 16 * (offs % 32);
			sy = 16 * (offs / 32);

			drawgfx(tmpbitmap,Machine->gfx[2],
					videoram[offs] + 256 * (colorram[offs] & 0x01),
					0,
					sx >= 256,0,	/* flip horizontally tiles on the right half of the bitmap */
					sx,sy,
					0,TRANSPARENCY_NONE,0);
		}
	}


	/* copy the temporary bitmap to the screen */
	{
		int scrollx;


		scrollx = -*mystston_scroll;

		copyscrollbitmap(bitmap,tmpbitmap,1,&scrollx,0,0,&Machine->drv->visible_area,TRANSPARENCY_NONE,0);
	}


	/* Draw the sprites */
	for (offs = 0;offs < spriteram_size;offs += 4)
	{
		if (spriteram[offs] & 0x01)
		{
			drawgfx(bitmap,Machine->gfx[(spriteram[offs] & 0x10) ? 4 : 3],
					spriteram[offs+1],
					0,
					spriteram[offs] & 0x02,spriteram[offs] & 0x04,
					(240 - spriteram[offs+2]) & 0xff,spriteram[offs+3],
					&Machine->drv->visible_area,TRANSPARENCY_PEN,0);
		}
	}


	/* draw the frontmost playfield. They are characters, but draw them as sprites */
	for (offs = mystston_videoram2_size - 1;offs >= 0;offs--)
	{
		if ((mystston_colorram2[offs] & 0x07) != 0x07 ||
				mystston_videoram2[offs] != 0x40)	/* don't draw spaces */
		{
			int sx,sy;


			sx = 8 * (offs % 32);
			sy = 8 * (offs / 32);

			drawgfx(bitmap,Machine->gfx[(mystston_colorram2[offs] & 0x04) ? 1 : 0],
					mystston_videoram2[offs] + 256 * (mystston_colorram2[offs] & 0x03),
					0,
					0,0,
					sx,sy,
					&Machine->drv->visible_area,TRANSPARENCY_PEN,0);
		}
	}
}

void matmania_vh_screenrefresh(struct osd_bitmap *bitmap)
{
	int offs;


	/* Update the tiles in the left tile ram bank */
	for (offs = videoram_size - 1;offs >= 0;offs--)
	{
		if (dirtybuffer[offs])
		{
			int sx,sy;


			dirtybuffer[offs] = 0;

			sx = offs % 32;
			sy = offs / 32;

			drawgfx(tmpbitmap,Machine->gfx[1],
					videoram[offs] + ((colorram[offs] & 0x08) << 5),
					(colorram[offs] & 0x30) >> 4,
					sx >= 16,0,	/* flip horizontally tiles on the right half of the bitmap */
					16*sx,16*sy,
					0,TRANSPARENCY_NONE,0);
		}
	}

	/* Update the tiles in the right tile ram bank */
	for (offs = mystston_videoram3_size - 1;offs >= 0;offs--)
	{
		if (dirtybuffer2[offs])
		{
			int sx,sy;


			dirtybuffer2[offs] = 0;

			sx = offs % 32;
			sy = offs / 32;

			drawgfx(tmpbitmap2,Machine->gfx[1],
					mystston_videoram3[offs] + ((mystston_colorram3[offs] & 0x08) << 5),
					(mystston_colorram3[offs] & 0x30) >> 4,
					sx >= 16,0,	/* flip horizontally tiles on the right half of the bitmap */
					16*sx,16*sy,
					0,TRANSPARENCY_NONE,0);
		}
	}


	/* copy the temporary bitmap to the screen */
	{
		int scrollx;


		scrollx = -*mystston_scroll;
		if (*matmania_pageselect)
			copyscrollbitmap(bitmap,tmpbitmap2,1,&scrollx,0,0,&Machine->drv->visible_area,TRANSPARENCY_NONE,0);
		else
			copyscrollbitmap(bitmap,tmpbitmap,1,&scrollx,0,0,&Machine->drv->visible_area,TRANSPARENCY_NONE,0);
	}


	/* Draw the sprites */
	for (offs = 0;offs < spriteram_size;offs += 4)
	{
		if (spriteram[offs] & 0x01)
		{
			drawgfx(bitmap,Machine->gfx[2],
					spriteram[offs+1] + ((spriteram[offs] & 0xf0) << 4),
					(spriteram[offs] & 0x08) >> 3,
					spriteram[offs] & 0x02,spriteram[offs] & 0x04,
					(240 - spriteram[offs+2]) & 0xff,spriteram[offs+3]+1,
					&Machine->drv->visible_area,TRANSPARENCY_PEN,0);
		}
	}


	/* draw the frontmost playfield. They are characters, but draw them as sprites */
	for (offs = mystston_videoram2_size - 1;offs >= 0;offs--)
	{
		int code;


		code = mystston_videoram2[offs] + 256 * (mystston_colorram2[offs] & 0x07);
		if (code != 0)	/* don't draw spaces */
		{
			int sx,sy;


			sx = offs % 32;
			sy = offs / 32;

			drawgfx(bitmap,Machine->gfx[0],
					code,
					(mystston_colorram2[offs] & 0x30) >> 4,
					0,0,
					8*sx,8*sy,
					&Machine->drv->visible_area,TRANSPARENCY_PEN,0);
		}
	}
}

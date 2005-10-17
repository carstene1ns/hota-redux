/*
 * Heart of The Alien: Sprite handling
 * Copyright (c) 2004 Gil Megidish
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "debug.h"
#include "sprites.h"
#include "vm.h"
#include "common.h"
#include "render.h"
#include "screen.h"

const char *sprite_data_byte_str[16] =
{
	"index", "u1", "next", "frame",
	0, 0, 0, 0, "u2", "u3",
	0, 0, 0, 0, "u6", "u7"
};

int first_sprite;
int sprite_count;
int last_sprite;

sprite_t sprites[64];

/** Prints out sprite debug information
    @param p   sprite entry
*/
void print_sprite(int p)
{
	LOG(("sprite %d\n", p));
	LOG(("\tindex: %d\n", sprites[p].index));
	LOG(("\tu1: %d\n", sprites[p].u1));
	LOG(("\tnext: %d\n", sprites[p].next));
	LOG(("\tframe: %d\n", sprites[p].frame));
	LOG(("\tx: %d\n", sprites[p].x));
	LOG(("\ty: %d\n", sprites[p].y));
	LOG(("\tu2: %d\n", sprites[p].u2));
	LOG(("\tu3: %d\n", sprites[p].u3));
	LOG(("\tw4: %d\n", sprites[p].w4));
	LOG(("\tw5: %d\n", sprites[p].w5));
	LOG(("\tu6: %d\n", sprites[p].u6));
	LOG(("\tu7: %d\n", sprites[p].u7));
}

short get_sprite_data_word(int entry, int i)
{
	assert(i >= 0 && i <= 15);

	switch(i)
	{
		case 4:
		return sprites[entry].x;

		case 6:
		return sprites[entry].y;

		case 10:
		return sprites[entry].w4;

		case 12:
		return sprites[entry].w5;

		default:
		LOG(("get_sprite_data_word on %d\n", i));
		assert(0);
		return 0;
	}
}

/** Returns sprite data by offset
    @param entry   sprite index
    @param i       structure offset
    @returns data value
*/
void set_sprite_data_word(int entry, int i, short value)
{
	assert(i >= 0 && i <= 15);

	LOG(("set_sprite_data_word(entry=%d, i=%d, value=%d)\n", entry, i, value));

	switch(i)
	{
		case 4:
		sprites[entry].x = value;
		break;

		case 6:
		sprites[entry].y = value;
		break;

		case 10:
		sprites[entry].w4 = value;
		break;

		case 12:
		sprites[entry].w5 = value;
		break;

		default:
		LOG(("set_sprite_data_word on %d value %d\n", i, value));
		assert(0);
	}
}

/** Returns sprite data by offset
    @param entry   sprite index
    @param i       structure offset
    @returns data value
*/
unsigned char get_sprite_data_byte(int entry, int i)
{
	assert(i >= 0 && i <= 15);

	switch(i)
	{
		case 0:
		return sprites[entry].index;

		case 1:
		return sprites[entry].u1;

		case 8:
		return sprites[entry].u2;

		case 9:
		return sprites[entry].u3;

		case 14:
		return sprites[entry].u6;

		case 15:
		return sprites[entry].u7;

		default:
		LOG(("get_sprite_data_byte on %d\n", i));
		assert(0);
		return 0;
	}
}

void set_sprite_data_byte(int entry, int i, unsigned char value)
{
	assert(i >= 0 && i <= 15);

	LOG(("set_sprite_data_byte(entry=%d, i=%d, value=%d)\n", entry, i, value));

	switch(i)
	{
		case 0:
		sprites[entry].index = value;
		break;

		case 1:
		sprites[entry].u1 = value;
		break;

		case 8:
		sprites[entry].u2 = value;
		break;

		case 9:
		sprites[entry].u3 = value;
		break;

		case 14:
		sprites[entry].u6 = value;
		break;

		case 15:
		sprites[entry].u7 = value;
		break;

		default:
		LOG(("set_sprite_data_byte on %d to %d\n", i, value));
		assert(0);
	}
}

void reset_sprite_list()
{
	int d0;

	memset(&sprites, 0, sizeof(sprites));
	                    
	/* cc5a */
	first_sprite = 1;
	sprite_count = 0;
	last_sprite = 1;

	/* default zorder */
	for (d0=1; d0<64; d0++)
	{
		sprites[d0].next = d0 + 1;
	}
	    
	/* note: is it 64 or 63 */
	sprites[63].next = 0;
}

void move_sprite_by(int sprite, int dx, int dy)
{
	sprites[sprite].x += dx;
	sprites[sprite].w4 += dx;
	sprites[sprite].y += dy;
	sprites[sprite].w5 += dy;
}

void flip_sprite(int sprite)
{
	int old;

	old = sprites[sprite].frame & 0x80;
	sprites[sprite].frame ^= 0x80;
	if (old == 0)
	{
		sprites[sprite].x += sprites[sprite].u2;
	}
	else
	{
		sprites[sprite].x -= sprites[sprite].u2;
	}
}

/** Turns on the mirror flag on this sprite
    @param sprite index
*/
void mirror_sprite(int sprite)
{
	if ((sprites[sprite].frame & 0x80) == 0)
	{
		flip_sprite(sprite);
	}
}

/** Turns off the mirror flag on this sprite
    @param sprite index
*/
void unmirror_sprite(int sprite)
{
	if ((sprites[sprite].frame & 0x80))
	{
		flip_sprite(sprite);
	}
}

/** Removes one or all sprites from sprite list
    @param var variable that specifies which sprite to remove (0 for all)
*/
void remove_sprite(int var)
{
	int entry;
	int to_remove;

	/* remove sprite */
	if (var == 0)
	{
		/* reset all sprites */
		reset_sprite_list();
		return;
	}

	if (sprite_count == 0)
	{
		/* no sprites anyway */
		return;
	}

	to_remove = get_variable(var);

	if (to_remove == first_sprite)
	{
		/* remove first sprite */
		first_sprite = sprites[to_remove].next;
		sprites[to_remove].next = last_sprite;
		last_sprite = to_remove;
		sprite_count--;
		return;
	}

	entry = first_sprite;

	/* look for the sprite to be removed */
	while (sprites[entry].next != to_remove)
	{
		entry = sprites[entry].next;
		if (entry == 0)
		{
			return;
		}
	}

	sprites[entry].next = sprites[to_remove].next;
	sprites[to_remove].next = last_sprite;
	sprite_count--;
}

/** Quickloads sprites state from file descriptor
    @param fp 
    @returns zero on success
*/
int quickload_sprites(FILE *fp)
{
	int i;

	for (i=0; i<MAX_SPRITES; i++)
	{
		sprite_t *spr = &sprites[i];

		spr->index = fgetc(fp);
		spr->u1 = fgetc(fp);
		spr->next = fgetc(fp);
		spr->frame = fgetc(fp);
		spr->x = fgetw(fp);
		spr->y = fgetw(fp);
		spr->u2 = fgetc(fp);
		spr->u3 = fgetc(fp);
		spr->w4 = fgetw(fp);
		spr->w5 = fgetw(fp);
		spr->u6 = fgetc(fp);
		spr->u7 = fgetc(fp);
	}

	first_sprite = fgetc(fp);
 	sprite_count = fgetc(fp);
	last_sprite = fgetc(fp);
	return 0;
}

/** Quicksaves sprite data onto a file descriptor
    @param fp  file descriptor to write data to
    @returns zero on success
*/
int quicksave_sprites(FILE *fp)
{
	int i;

	/* write all sprites */
	for (i=0; i<MAX_SPRITES; i++)
	{
		sprite_t *spr = &sprites[i];

		fputc(spr->index, fp);
		fputc(spr->u1, fp);
		fputc(spr->next, fp);
		fputc(spr->frame, fp);
		fputw(spr->x, fp);
		fputw(spr->y, fp);
		fputc(spr->u2, fp);
		fputc(spr->u3, fp);
		fputw(spr->w4, fp);
		fputw(spr->w5, fp);
		fputc(spr->u6, fp);
		fputc(spr->u7, fp);
	}

	fputc(first_sprite, fp);
 	fputc(sprite_count, fp);
	fputc(last_sprite, fp);
	return 0;
}

/** Renders a single sprite
    @param list_entry
*/
void render_sprite(int list_entry)
{
	unsigned long d0, d1, d2, d3, d4, d6;
	int d7;
	int offset;
	int color;
	int a2, a3;
	int count;
	int sx, sy;
	int x, y, dx, dy;

	/* d89e */
	d2 = sprites[list_entry].index;

	a3 = get_long(0xf904) + (d2 << 2);
	a3 = get_long(a3);

	d0 = d1 = d2 = 0;
	x = extl(sprites[list_entry].x);
	y = extl(sprites[list_entry].y);

	d3 = y*304 + x;

	d2 = sprites[list_entry].frame & 0x7f;
	d1 = get_word(a3 + d2*2 + 6);
	a2 = a3 + d1 + 4;
	d6 = get_word(a2);
	a2 += 2;
	
	for (color = 0; color < 16; color++)
	{
		if ((d6 & (1 << color)) == 0)
		{
			/* no such color in sprite */
			continue;
		}

		if (sprites[list_entry].frame & 0x80)
		{
			goto sprite_mirrored;
		}
	
		/* d2 is color */
		loc_d91e:
		offset = get_word(a2); /* relative offset */
		a2 = a2 + 2;
		dx = offset % 304;
		dy = offset / 304;
		sx = x + dx;
		sy = y + dy;
		count = get_byte(a2++) + 1;
		fill_line(count, sx, sy, color);
	
		loc_d93c:
		d7 = get_byte(a2++);
		if (d7 == 0x99)
		{
			/* repeat using the same color! */
			goto loc_d91e;
		}
	
		if (d7 == 0xaa)
		{
			continue;
		}
	 
		/* d94a */
		/* d7: two signed nibbles */
		d4 = d7;
	
		d7 = extn((unsigned char)(d7 & 0x0f)); /* delta-count */
		dx = extn((unsigned char)(d4 >> 4));
		count = count + d7 - dx;
	
		sy = sy + 1;
		sx = sx + dx;
		fill_line(count, sx, sy, color);
		goto loc_d93c;

		sprite_mirrored:
		/* d9a6 */
		/* d2 is color */
		loc_d9b4:
		offset = get_word(a2);
		a2 += 2;
	
		dx = offset % 304;
		dy = offset / 304;
		sx = x - dx;
		sy = y + dy;
	
		count = get_byte(a2++) + 1;
		fill_line_reversed(count, sx, sy, color);
	
		loc_d9e2:
		d7 = get_byte(a2++);
		if (d7 == 0x99)
		{
			goto loc_d9b4;
		}
	
		if (d7 == 0xaa)
		{
			continue;
		}
	
		d4 = d7;
		d7 = extn((unsigned char)(d7 & 0x0f));
		dx = extn((unsigned char)(d4 >> 4));
		count = count + d7 - dx;
	
		sy = sy + 1;
		sx = sx - dx;
	
		fill_line_reversed(count, sx, sy, color);
		goto loc_d9e2;
	}
}

/** Draws all visible sprites

    Sprites are arranged in a linked list of z-order, where each
    has a visibility flag, and a mirror flag
*/
void draw_sprites()
{
	LOG(("draw_sprites"));

	if (sprite_count != 0)
	{
		unsigned char d1;

		d1 = first_sprite;
		while (1)
		{
			if ((sprites[d1].u1 & FLAG_HIDDEN) == 0)
			{
				/* visible */
				render_sprite(d1);
			}

			d1 = sprites[d1].next;
			if (d1 == 0)
			{
				break;
			}

			LOG(("linkedup to %d\n", d1));
		}
	}
}

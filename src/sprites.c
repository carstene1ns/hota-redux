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

void mirror_sprite(int sprite)
 {
	if ((sprites[sprite].frame & 0x80) == 0)
	{
		flip_sprite(sprite);
	}
}

void unmirror_sprite(int sprite)
{
	if ((sprites[sprite].frame & 0x80))
	{
		flip_sprite(sprite);
	}
}

void remove_sprite(int var)
{
	int d1, entry;
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
	loc_bf0c:
	if (sprites[entry].next == to_remove)
	{
		goto loc_bf24;
	}

	d1 = sprites[entry].next;
	if (d1 == 0)
	{
		return;
	}

	entry = d1;
	goto loc_bf0c;

	loc_bf24:
	d1 = last_sprite;
	last_sprite = to_remove;
	sprites[entry].next = sprites[to_remove].next;
	sprites[to_remove].next = d1;
	sprite_count--;
}


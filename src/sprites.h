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
#ifndef __SPRITES_INCLUDED__
#define __SPRITES_INCLUDED__

#define FLAG_HIDDEN 0x80

#define MAX_SPRITES 64

typedef struct sprite_s
{
	unsigned char index;
	unsigned char u1;
	unsigned char next;
	unsigned char frame;
	short x;
	short y;
	unsigned char u2;
	unsigned char u3;
	short w4;
	short w5;
	unsigned char u6;
	unsigned char u7;
} sprite_t;

/////////
extern sprite_t sprites[];
extern const char *sprite_data_byte_str[16];
/////////

void print_sprite(int p);
short get_sprite_data_word(int entry, int i);
void set_sprite_data_word(int entry, int i, short value);
unsigned char get_sprite_data_byte(int entry, int i);
void set_sprite_data_byte(int entry, int i, unsigned char value);
void reset_sprite_list();
void move_sprite_by(int sprite, int dx, int dy);
void flip_sprite(int sprite);
void mirror_sprite(int sprite);
void unmirror_sprite(int sprite);
void remove_sprite(int var);
void render_sprite(int list_entry);

int quickload_sprites(FILE *fp);
int quicksave_sprites(FILE *fp);

#endif

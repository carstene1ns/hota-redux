/*
 * Heart of The Alien: Opcode decoder
 * Copyright (c) 2004 Gil Megidish
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include <assert.h>
#include <SDL.h>

#include "vm.h"
#include "music.h"
#include "debug.h"
#include "sound.h"
#include "common.h"
#include "decode.h"
#include "render.h"
#include "screen.h"
#include "sprites.h"
#include "animation.h"

//////
extern unsigned short variables[];
extern unsigned short auxvars[];
extern int current_room;
extern int next_script;
extern char first_sprite, last_sprite, sprite_count;

extern short task_pc[64];
extern short new_task_pc[64];
extern short enabled_tasks[64];
extern short new_enabled_tasks[64];

//////

static char *cmpopar[6] = {"==", "!=", ">", ">=", "<", "<="};

int pc;
int script_ptr;

static int stack[128];
static int stack_ptr = 0;

static void push(int v)
{
	assert(stack_ptr < 128);
	stack[stack_ptr++] = v;
}

static int pop()
{
	assert(stack_ptr > 0);
	stack_ptr--;
	return stack[stack_ptr];
}

unsigned char next_pc()
{
	unsigned char rv;

	rv = get_byte(script_ptr + pc);
	pc++;
	return rv;
}

unsigned short next_pc_word()
{
	int left, right;

	left = next_pc();
	right = next_pc();
	return (left << 8) + right;
}

void fill_line_reversed(int count, int x, int y, int color)
{
	unsigned char *bufp;

	if (x < 0 || y < 0 || y > 191)
	{
		LOG(("out of screen\n"));
		return;
	}

	if (x - count < 0)
	{
		count = x + 1;
	}

	if (x > 303)
	{
		count = count - (x - 303);
		x = 303;
	}

	bufp = (unsigned char *)get_selected_screen_ptr() + (y * 304) + x;
	while (count > 0)
	{
		*bufp-- = color;
		count--;
	}
}

void fill_line(int count, int x, int y, int color)
{
	unsigned char *bufp;

	//LOG(("fill line count=%d, x=%d, y=%d, color=%d\n", count, x, y, color));

	/* output offset */
	if (x > 303 || y > 191 || y < 0)
	{
		LOG(("WARN: out of screen! (x=%d, y=%d)\n", x, y));
		return;
	}

	while (x < 0)
	{
		x++;
		count--;
	}

	if (x + count > 303)
	{
		count = 303 - x + 1;
	}

	bufp = (unsigned char *)get_selected_screen_ptr() + (y * 304) + x;
	while (count > 0)
	{
		*bufp++ = color;
		count--;
	}
}

void render_sprite(int list_entry)
{
	unsigned long dword_0_DC7A;
	unsigned long d0, d1, d2, d3, d4, d6;
	int d7;
	int offset;
	int color;
	int a2, a3;
	int count;
	int sx, sy;
	int x, y, dx, dy;

	dword_0_DC7A = -1;

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
	
	color = 0;

	/* d8ea */
	d6 = d6 | 0x10000; /* bit 17th turned on, to break loop */
	loc_d8ee:
	if (d6 & (1 << color))
	{
		goto loc_d8f8;
	}

	color++;
	goto loc_d8ee;

	loc_d8f8:
	if (color >= 16)
	{
		goto leave;
	}

	if (sprites[list_entry].frame & 0x80)
	{
		goto sprite_mirrored;
	}

	loc_d916:
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
		goto loc_d986;
	}
 
	/* d94a */
	/* d7: two signed nibbles */
	d4 = d7;

	d7 = extn(d7 & 0x0f); /* delta-count */
	dx = extn(d4 >> 4);
	count = count + d7 - dx;

	sy = sy + 1;
	sx = sx + dx;
	fill_line(count, sx, sy, color);
	goto loc_d93c;

	loc_d986:
	color++;

	/* look for next bit */
	while (1)
	{
		if (d6 & (1 << color))
		{
			break;
		}

		color++;
	}

	if (color < 16)
	{
		goto loc_d916;
	}

	leave:
	return;

	sprite_mirrored:
	loc_d9ac:
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
		goto loc_da2c;
	}

	d4 = d7;
	d7 = extn(d7 & 0x0f);
	dx = extn(d4 >> 4);
	count = count + d7 - dx;

	sy = sy + 1;
	sx = sx - dx;

	fill_line_reversed(count, sx, sy, color);
	goto loc_d9e2;

	loc_da2c:
	color++;

	loc_da34:
	if (d6 & (1 << color))
	{
		goto loc_da3e;
	}

	color++;
	goto loc_da34;

	loc_da3e:
	if (color < 16)
	{
		goto loc_d9ac;
	}
}

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

void load_sprite(int index, int list_entry)
{
	unsigned long a2, a4;
	unsigned short d0, d1, d2, d3;

	/* copy sprite from resources into list */

	/* cbdc get resource */
	a4 = get_long(0xf904) + (index << 2);
	a4 = get_long(a4);

	/* cbec a6-1 => 3 */
	d3 = sprites[list_entry].frame;
	d0 = d3 & 0x7f;
	d1 = get_word(a4 + d0*2 + 6);
	a2 = a4 + d1 + 2;
	d0 = get_byte(a2++);
	sprites[list_entry].u2 = (unsigned char)d0;
	d0 = d0 >> 1;
	d1 = next_pc_word();

	d2 = d1;
	if (d3 & 0x80)
	{
		/* mirrored */
		d1 = d1 + d0;
	}
	else
	{
		d1 = d1 - d0;
	}

	sprites[list_entry].x = d1;
	d0 = get_byte(a4 + 3);    
	sprites[list_entry].u6 = (unsigned char)d0;
	d0 = d0 >> 1;
	d2 = d2 - d0;

	/* cc30 */
	d1 = next_pc_word();
	d3 = d1;
	d0 = get_byte(a2++);
	d1 = d1 - d0;
	sprites[list_entry].y = d1;
	LOG(("\n"));
	LOG(("sprites[%d].x == %d\n", list_entry, sprites[list_entry].x));
	LOG(("sprites[%d].y == %d\n", list_entry, sprites[list_entry].y));

	d0 = d0 >> 1;
	d3 = d3 - d0;

	/* cc42 */
	d1 = get_byte(a4 + 4);
	sprites[list_entry].u7 = d1;
	d1 = d1 >> 1;
	d3 = d3 - d1;
	sprites[list_entry].w4 = d2;
	sprites[list_entry].w5 = d3;

	/* used before calling */
	next_pc();
}

void op_85()
{
	int d0, entry;

	reset_sprite_list();
	d0 = get_variable(next_pc());

	sprite_count = 1;
	first_sprite = d0;
	entry = d0;
	if (entry != 1)
	{
		goto loc_bf88;
	}

	last_sprite = sprites[entry].next;
	sprites[entry].next = 0;
	return;

	loc_bf88:
	LOG(("NOT IMPLMENTED!\n"));
	assert(0);
}

void collision_against_set()
{
	int entry_a1, entry_trg, entry_src;
	short d2, d3;
	int d4, d5, d6, d7;
	int flipped;

	/* op_28 collision detection! */
	d2 = d3 = 0;
	entry_src = get_variable(next_pc());
	LOG(("collision of sprite %d\n", entry_src));
	if (sprite_count == 0)
	{
		/* no sprites at all! */
		set_variable(2, 0);
		set_variable(3, 0);
		return;
	}

	print_sprite(entry_src);

	LOG(("\n\n"));

	entry_trg = first_sprite;

	/* a3 is source */

	flipped = (sprites[entry_src].frame & 0x80);
	d6 = sprites[entry_src].w4 + sprites[entry_src].u6;
	d7 = sprites[entry_src].w5 + sprites[entry_src].u7;

	loop:
	if (entry_src == entry_trg)
	{
		/* same sprite, cant collide */
		goto next_sprite;
	}

	LOG(("checking collision against sprite %d\n", entry_trg));
	LOG(("src (%d, %d) - (%d, %d)\n", sprites[entry_src].w4, sprites[entry_src].w5,  sprites[entry_src].w4 + sprites[entry_src].u6, sprites[entry_src].w5 + sprites[entry_src].u7)); 
	LOG(("dst (%d, %d) - (%d, %d)\n", sprites[entry_trg].w4, sprites[entry_trg].w5,  sprites[entry_trg].w4 + sprites[entry_trg].u6, sprites[entry_trg].w5 + sprites[entry_trg].u7)); 

	if (sprites[entry_trg].u3 == 0)
	{
		goto next_sprite;
	}

	if (sprites[entry_trg].u3 == sprites[entry_src].u3)
	{
		goto next_sprite;
	}

	if (sprites[entry_trg].u1 & FLAG_HIDDEN)
	{
		/* hidden */
		goto next_sprite;
	}

	if (sprites[entry_trg].u3 < 3 || sprites[entry_trg].u3 > 6)
	{
		goto loc_c5b8;
	}

	if (sprites[entry_src].u3 < 3 || sprites[entry_src].u3 > 6)
	{
		goto loc_c5b8;
	}

	goto next_sprite;

	loc_c5b8:
	if (d6 < sprites[entry_trg].w4 || d7 < sprites[entry_trg].w5)
	{
		goto next_sprite;
	}

	LOG(("A (%d, %d) / (%d, %d)\n", d6, d7, sprites[entry_trg].w4, sprites[entry_trg].w5));

	d4 = sprites[entry_trg].w4 + sprites[entry_trg].u6; 
	d5 = sprites[entry_trg].w5 + sprites[entry_trg].u7; 

	/* c5d8 */
	if (d4 < sprites[entry_src].w4 || d5 < sprites[entry_src].w5)
	{
		goto next_sprite;
	}

	LOG(("B (d4=%d, d5=%d) / (w4=%d, w5=%d) required d4>=w4 && d5>=w5\n", d4, d5, sprites[entry_src].w4, sprites[entry_src].w5));

	if (d3 == 0)
	{
		/* first collision! */
		LOG(("collides because d3==0\n"));
		goto collide;
	}

	/* if it gets here, then there are two or more objects that
	 * collide with the sprite, we check if the current sprite
	 * is closer to the source entry.
	 */
	entry_a1 = d3;
	
	if (flipped == 0)
	{
		/* c5fc */
		if (sprites[entry_a1].w4 <= sprites[entry_trg].w4)
		{
			goto next_sprite;
		}

		/* collision! */
		LOG(("collides because a1.w4 > trg.w4 (%d >= %d) flipped=0\n", sprites[entry_a1].w4, sprites[entry_trg].w4));
		goto collide;
	}
	else
	{
		/* c608 */
		if (sprites[entry_a1].w4 >= sprites[entry_trg].w4)
		{
			goto next_sprite;
		}

		/* collision! */
		LOG(("collides because a1.w4 < trg.w4 (%d >= %d) flipped=1\n", sprites[entry_a1].w4, sprites[entry_trg].w4));
		goto collide;
	}

	collide:
	d2 = sprites[entry_trg].u3;
	d3 = entry_trg;
	LOG(("colides with d3=%d d2=%d\n", d3, d2));

	next_sprite:
	entry_trg = sprites[entry_trg].next;
	if (entry_trg != 0)
	{
		/* more sprites to check collision against */
		goto loop;
	}

	LOG(("leaving op28 with d2=%d, d3=%d\n", d2, d3));
	set_variable(2, d2);
	set_variable(3, d3);
}

void op_27()
{
	int entry;
	short d0, d1, d2, d3;
	int a2, a4;

	/* get sprite info */
	d0 = get_variable(next_pc());
	d1 = next_pc();

	entry = d0;
		
	if (d1 & 0x80)
	{
		d1 = d1 & 0x7f;

		if (d1 == 0x13)
		{
			/* d1 was 0x93 */
			d0 = sprites[entry].y;
			d1 = sprites[entry].index;
	
			a4 = get_long(0xf904) + (d1 << 2);
			a4 = get_long(a4);
	
			d3 = sprites[entry].frame;
			d1 = d3 & 0x7f;
	
			d1 = get_word(a4 + d1*2 + 6);
			a2 = a4 + d1 + 3;
			d3 = d0;

			/* assumption: return hotspot y */
		
			d1 = get_byte(a2++);
			d0 = d0 + d1;
			set_variable(2, d0);

			LOG(("var[2] was set to 0x%x\n", d0));
		}
		else
		{	
			LOG(("loc_d1_ne_0x13: set_var(2, %d) d1=%d entry=%d\n", d1, entry, get_sprite_data_word(entry, d1)));

			set_variable(2, get_sprite_data_word(entry, d1));
		}
		return;
	}

	if (d1 == 3)
	{
		d1 = sprites[entry].frame;
		set_variable(2, d1 & 0x7f);
		set_variable(3, d1 & 0x80);
		LOG(("var[2] = frame(%d), var[3] = 7bit(0x%x)\n", d1 & 0x7f, d1 & 0x80));
		return;
	}

	if (d1 < 16)
	{
		int dummy = d1;
		d1 = get_sprite_data_byte(entry, d1);
		LOG(("get %s of sprite %d (0x%x)\n", sprite_data_byte_str[dummy], entry, d1));
		set_variable(2, d1);
		return;
	}

	if (d1 == 16)
	{
		d1 = sprites[entry].frame & 0x7f;
		LOG(("get frame of sprite %d (0x%x)\n", entry, d1));
		set_variable(2, d1);
		return;
	}

	if (d1 != 18)
	{
		d1 = sprites[entry].frame & 0x80;
		LOG(("get frame-7th bit of sprite %d (0x%x)\n", entry, d1));
		set_variable(2, d1);
		return;
	}

	/* d1 == 18 */
	d1 = sprites[entry].x;
	d2 = d1;
	d3 = sprites[entry].u2;
	if ((sprites[entry].frame & 0x80) == 0)
	{
		d2 = d2 + d3;
	}
	else
	{
		d1 = d1 - d3;
	}

	set_variable(2, d1);
	set_variable(3, d2);
	LOG(("(d1 == 18), var[2] = 0x%x, var[3] = 0x%x\n", d1, d2));

	/* passed, all okay */
}

void op_24()
{
	int a3;
	int d0, d1, d2, d3;
	int saved_d0, saved_d0_2, saved_d6;
	int spr;

	int x1, x2;
	int y1, y2;

	/* op24, collision of sprite against scene? */
	spr = get_variable(next_pc());
	LOG(("op24 on sprite=%d\n", spr));

	/* op_24_1(); */
	x1 = sprites[spr].x;
	x2 = sprites[spr].u2;
	if (sprites[spr].frame & 0x80)
	{
		/* mirrored */
		int tmp = x1;
		x1 = x1 - x2;
		x2 = tmp;
	}
	else    	
	{
		x2 = x2 + x1;
	}

	/* d4, sprite's x1 */
	/* d6, sprite's x2 */

	y1 = sprites[spr].y;
	y2 = y1 + sprites[spr].u7;

	/* d5, sprite's y1 */
	/* d7, sprite's y2 */
	LOG(("sprite bounding box is (%d,%d)-(%d,%d)\n", x1, y1, x2, y2));

	/* bc60 */
	a3 = script_ptr;
	d2 = get_variable(5);
	LOG(("variable 5 is 0x%x\n", d2));
	d2 = extl(d2);
	a3 = a3 + d2;
	d0 = get_byte(a3);

	LOG(("variable 4 is 0x%x\n", get_variable(4)));
	LOG(("comparing d1 > d0 (0x%x > 0x%x)\n", get_variable(4), d0));
	
	saved_d0 = d0;
	d1 = get_variable(4);
	if (d1 > d0)
	{
		LOG(("d1 > d0, no collision\n"));
		goto no_collision;
	}

	d0 = d0 - d1;

	a3 = a3 + 6*d1;

	loc_bc86:
	a3 = a3 + 2;
	d1 = extl(get_word(a3));
	a3 += 2;
	if ((get_byte(a3 - 3) & 0x80) == 0)
	{
		LOG(("get_byte(a3 - 3) & 0x80 == 0\n"));
		goto loc_bca8;
	}

	/* when does it ever get here? */
	d2 = get_byte(a3++);
	d3 = get_byte(a3++);
	y1 = get_byte(a3);
	LOG(("loaded batch 1: d2,d3,d5 = %d, %d, %d\n", d2, d3, y1));

	/* checkpoint: all okay */
	
	/* op_24_2: cb9e */
	{
		int tmp;

		tmp = (d3 * y1) + d1;
		if (x2 < d1 || x1 > tmp) 
		{
			goto loc_cbd6;
		}

		/* d0 middle point of sprite's X */
		tmp = (x1 + x2) / 2;
		x2 = d2;
		x1 = d1 + d3;

		loc_cbba:
		if (x1 > tmp)
		{
			goto loc_cbc8;
		}
	
		x2++;
		x1 = x1 + d3;
		y1--;
		if (y1 != 0)
		{
			goto loc_cbba;
		}
	
		loc_cbc8:
		x1 = x2;
		d3 = get_byte(a3-5) & 0x7f;
		goto leave_fn_2;

		loc_cbd6:
		d3 = 0;

		leave_fn_2:
		d3 = d3; // keep gcc happy
	}
	/* all okay */

	/* bca2 */
	if (d3 == 0)
	{
		/* continue */
		goto loc_bce4;
	}

	goto leave;

	loc_bca8:
	d2 = extl(get_word(a3));
	a3 += 2;
	d3 = get_byte(a3);
	LOG(("bca8: loaded d2=0x%x, d3=0x%x\n", d2, d3));
	if (d3 != 0)
	{
		LOG(("d3 != 0, jmp bcba\n"));
		goto loc_bcba;
	}

	/* op_24_3: cae0 */
	{
		d3 = 0;
		LOG(("comparing d1<x1 or d1>x2 (d1=%d, d4=%d, x2=%d)\n", d1, x1, x2));
		if (d1 < x1 || d1 > x2)
		{
			goto loc_cb0c;
		}

		LOG(("comparing d2<y1 or d2>y2 (d2=%d, y1=%d, y2=%d)\n", d2, y1, y2));
		if (d2 < y1 || d2 > y2)
		{
			goto loc_cb0c;
		}

		d3 = get_byte(a3 - 5);
		x1 = d1;
		if (d3 == 2)
		{
			x1 = x1 + 2;
		}
		else if (d3 == 3)
		{
			x1 = x1 - 2;
		}

		loc_cb0c:
		d3 = d3; // keep gcc happy
	}	
	/* all okay */

	/* bcb4 */
	if (d3 == 0)
	{
		/* continue */
		goto loc_bce4;
	}

	goto leave;

	loc_bcba:
	if (d3 & 0x80)
	{
		/* bclr */
		d3 = d3 & 0x7f;
		goto loc_bcca;
	}

	/* op_24_4 cb0e */
	{
		LOG(("op_24_4 d2=%d, d3=%d, y1=%d, y2=%d\n", d2, d3, y1, y2));
		if (d2 > y2)
		{
			goto loc_cb3e;
		}

		d2 = d2 + d3;
		if (d2 < y1)
		{
			goto loc_cb3e;
		}

		LOG(("comparing d1<x1, d1>x2  -- %d<%d or %d>%d\n", d1, x1, d1, x2));
		if (d1 < x1 || d1 > x2)
		{
			goto loc_cb3e;
		}

		d3 = get_byte(a3 - 5);
		x1 = d1;
		if (d3 == 2)
		{
			x1 = x1 + 2;
		}
		else if (d3 == 3)
		{
			x1 = x1 - 2;
		}
		goto leave_fn_4;

		loc_cb3e:
		d3 = 0;

		leave_fn_4:
		d3 = d3; // keep gcc happy
	}

	/* all okay */
	/* bcc4 */
	if (d3 == 0)
	{
		/* continue */
		goto loc_bce4;
	}

	goto leave;

	loc_bcca:
	if ((get_byte(a3-5) & 0x40) == 0)
	{
		goto loc_bcdc;
	}

	/* op_24_5 cb64 */
	{
		saved_d0_2 = d0;
		saved_d6 = x2;

		if (d2 < y1 || d2 > y2)
		{
			goto loc_cb96;
		}

		x2 = (x2 + x1) / 2;
		if (d1 > x2)
		{
			goto loc_cb96;
		}

		d0 = d1;
		d1 = d1 + d3;
		if (d1 < x2)
		{
			goto loc_cb96;
		}

		d0 = (d0 + d1) / 2;
		x1 = d0;
		d3 = get_byte(a3-5) & 0x3f;
		goto leave_fn_5;

		loc_cb96:
		d3 = 0;
		goto leave_fn_5;

		leave_fn_5:
		d0 = saved_d0_2;
		x2 = saved_d6;
	}
	
	// all okay
	if (d3 == 0)
	{
		/* continue */
		goto loc_bce4;
	}

	goto leave;

	loc_bcdc:
	/* op_24_6 : cb42 */
	{
		if (d1 > x2)
		{
			goto loc_cb60;
		}

		d1 = d1 + d3;
		if (d1 < x1)
		{
			goto loc_cb60;
		}

		if (d2 < y1 || d2 > y2)
		{
			goto loc_cb60;
		}

		x1 = get_byte(a3-5);
		d3 = x1;
		x1 = d2;
		goto leave_fn_6;

		loc_cb60:
		d3 = 0;

		leave_fn_6:
		d3 = d3; // keep gcc happy
	}

	if (d3 != 0)
	{
		goto leave;
	}

	loc_bce4:
	/* continue */
	LOG(("loop collisions d0=%d\n", d0));
	if (d0 >= 0)
	{
		d0--;
		goto loc_bc86;
	}

	no_collision:
	LOG(("no_collision\n"));
	d3 = 0;
	x1 = 0;

	leave:
	set_variable(2, d3);
	set_variable(3, x1);

	LOG(("post op24, var2=%d, var3=%d\n", d3, x1));

	x1 = saved_d0 - d0 + 1;
	set_variable(4, x1);
}

void add_sprite()
{
	int entry_a6, entry_a4;
	unsigned char p;
	short d0, d1, d2, d3;

	/* op_25 add sprite add zorder d0 */

	/* 25 u1 index frame some_bit xx xx xx xx yy */

	/* bd0a */
	entry_a4 = first_sprite;
	d0 = next_pc();

	if (sprite_count == 0)
	{
		/* bde8 */
		d2 = last_sprite;
		set_variable(2, d2);
		first_sprite = d2;
		entry_a6 = d2;
		last_sprite = sprites[entry_a6].next;

		d1 = next_pc();
		sprites[entry_a6].index = (unsigned char)d1;
		sprites[entry_a6].u1 = (unsigned char)d0;
		sprites[entry_a6].next = 0;

		p = next_pc();
		if (next_pc() != 0)
		{
			p = p | 0x80;
		}

		sprites[entry_a6].frame = p;

		sprites[entry_a6].u3 = get_byte(script_ptr + pc + 4);

		/* passed: code is okay! */
		load_sprite(d1, entry_a6);
		sprite_count++;

		//LOG(("d2 == %d\n", d2);
		//print_hex(&sprites[d2][0], 16);
	}
	else
	{
		/* bd2c */
		if (d0 <= sprites[entry_a4].u1)
		{
			/* bd34 */
			loc_bd34:
			d1 = sprites[entry_a4].next;
			if (d1 == 0)
			{
				/* be32 */
				d2 = last_sprite;
				set_variable(2, d2);
				sprites[entry_a4].next = (unsigned char)d2;
				entry_a6 = d2;
				last_sprite = sprites[entry_a6].next;

				d1 = next_pc();
				sprites[entry_a6].index = (unsigned char)d1;
				sprites[entry_a6].u1 = (unsigned char)d0;
				sprites[entry_a6].next = 0;
				p = next_pc();
				if (next_pc() != 0)
				{
					p = p | 0x80;
				}

				sprites[entry_a6].frame = p;
				sprites[entry_a6].u3 = get_byte(script_ptr + pc + 4);
				load_sprite(d1, entry_a6);
				sprite_count++;
				return;
			}

			/* else */
			entry_a6 = entry_a4;
			entry_a4 = d1;
			if (d0 < sprites[entry_a4].u1)
			{
				goto loc_bd34;
			}

			/* bd4c */
			d2 = last_sprite;
			set_variable(2, d2);
			d3 = sprites[entry_a6].next;
			sprites[entry_a6].next = (unsigned char)d2;
			entry_a6 = d2;

			last_sprite = sprites[entry_a6].next;
			d1 = next_pc();
			sprites[entry_a6].index = (unsigned char)d1;
			sprites[entry_a6].u1 = (unsigned char)d0;
			sprites[entry_a6].next = (unsigned char)d3;
			
			p = next_pc();
			if (next_pc() != 0)
			{
				p = p | 0x80;
			}

			sprites[entry_a6].frame = p;
			sprites[entry_a6].u3 = get_byte(script_ptr + pc + 4);				
			load_sprite(d1, entry_a6);
			sprite_count++;
			return;
		}
		else
		{
			/* bd98 */
			d2 = last_sprite;
			set_variable(2, d2);
			d3 = first_sprite;
			first_sprite = d2;
			entry_a6 = d2;
			last_sprite = sprites[entry_a6].next;
			d1 = next_pc();
			sprites[entry_a6].index = (unsigned char)d1;
			sprites[entry_a6].u1 = (unsigned char)d0;
			sprites[entry_a6].next = (unsigned char)d3;

			p = next_pc();
			if (next_pc() != 0)
			{
				p = p | 0x80;
			}

			sprites[entry_a6].frame = p;
			sprites[entry_a6].u3 = get_byte(script_ptr + pc + 4);				
			load_sprite(d1, entry_a6);
			sprite_count++;
			return;
		}
	}
}

void is_last_frame_in_animation(int entry)
{
	int a4, d0;

	a4 = get_long(0xf904) + (sprites[entry].index << 2);
	a4 = get_long(a4);

	/* returns 1 if sprite has reached the last frame of its sequence */

	d0 = sprites[entry].frame & 0x7f;
	if (d0 < get_byte(a4))
	{
		set_variable(2, 0);
	}
	else
	{
		set_variable(2, 1);
	}
}

void copy_tls_to_global()
{
	int src_index, dst_index, count;

	dst_index = next_pc();
	src_index = next_pc();
	count = next_pc();

	/* copy variables from variables to active-var-vector */
	while (count >= 0)
	{
		LOG(("copy tls-var %d into globalvar %d\n",  src_index, dst_index));
		variables[dst_index++] = get_variable(src_index++);
		count--;
	}
}

void copy_global_to_tls()
{
	int src_index, dst_index, count;

	dst_index = next_pc();
	src_index = next_pc();
	count = next_pc();

	LOG(("copy %d from global to tls starting at src=%d, dst=%d\n", count + 1, src_index, dst_index));

	/* copy variables from variables to active-var-vector */
	while (count >= 0)
	{
		LOG(("copy globalvar %d into tls-var %d\n", src_index, dst_index));
		set_variable(dst_index++, variables[src_index++]);
		count--;
	}
}

void op_89()
{
	int entry, d1, p;

	/* format 89 aa bb cc dd */
	/* aa: sprite index in list */
	/* bb: character index */
	/* cc: start frame in sequence */
	/* dd: flipped if ne 0 */

	entry = get_variable(next_pc());
	d1 = next_pc();
	sprites[entry].index = d1;

	p = next_pc();
	if (next_pc() != 0)
	{
		/* flipped */
		p = p | 0x80;
	}

	sprites[entry].frame = p;
	load_sprite(d1, entry);

	/* WTF! */
	pc--;
}

/* op_2a: next frame */
void op_2a()
{
	unsigned long a2, a4;
	int d0, d1, d2, d3;
	int dummy;
	int which;

	which = next_pc();
	which = get_variable(which);

	d0 = sprites[which].index;
	LOG(("sprite d0 = %d\n", d0));

	a4 = get_long(0xf904) + (d0 << 2);
	a4 = get_long(a4);

	d0 = d2 = d3 = 0;

	d0 = sprites[which].frame & 0x7f;
	d1 = d0;

	/* next frame */
	d0++;

	/* get_byte(a4) == total frames. d0 = current frame */
	LOG((" 	comparing d0 with get_byte(a4), %d with %d\n", d0, get_byte(a4)));
	if (d0 > get_byte(a4))
	{
		/* c3f8 */
		d0 = get_byte(a4 + 5); /* first frame (on loop) */
		d2 = get_byte(a4 + 1); /* hotspot x */
		d3 = get_byte(a4 + 2); /* hotspot y */

		LOG(("assigning d0, d2, d3 with %d %d %d\n", d0, d2, d3));
		sprites[which].frame = (sprites[which].frame & 0x80) | d0;
	
		d1 = get_word(a4 + d0*2 + 6);
		a2 = a4 + d1;
	}
	else
	{
		/* c41c */
		sprites[which].frame = (sprites[which].frame & 0x80) | d0;
	
		d1 = get_word(a4 + d0*2 + 6);
		a2 = a4 + d1;
		d2 = get_byte(a2 + 0); /* hotspot x */
		d3 = get_byte(a2 + 1); /* hotspot y */

		LOG(("d3 loaded is %d(%x)\n", d3, d3));
	}

	LOG(("before d3=%d after =%d\n", d3, extw(d3)));
	d3 = extw(d3); 
	d2 = extw(d2); 

	d0 = get_byte(a2 + 2);
	sprites[which].u2 = d0;

	d0 = extw(d0);
	if ((sprites[which].frame & 0x80) == 0)
	{
		sprites[which].x = sprites[which].x + d2;
	}
	else
	{
		sprites[which].x = sprites[which].x - d2;
		d0 = -d0;
	}

	LOG(("new sprites[which].x == %d\n", sprites[which].x));

	/* loc_c45c: */
	LOG(("before adding y=%d, d3=%d\n", sprites[which].y, d3));
	sprites[which].y += d3;
	LOG(("new sprites[which].y == %d (which=%d)\n", sprites[which].y, which));
	
	/* c460 */
	d1 = get_byte(a4 + 3);
	d0 = d0 - d1;
	d0 = d0 / 2;

	dummy = sprites[which].x;
	d0 = d0 + dummy;
	sprites[which].w4 = d0;
	
	/* c472 */
	d0 = get_byte(a2 + 3);
	d0 = d0 - get_byte(a4 + 4);
	d0 = d0 / 2;

	dummy = sprites[which].y;
	d0 = d0 + dummy;
	sprites[which].w5 = d0;

	//LOG(("after 2a:\n");
	//print_hex(&sprites[which][0], 16);
}

void op_2b()
{
	int d0, d1, d2, d3, entry;
	int a2, a4;

	/* update sprite info */

	d0 = get_variable(next_pc());
	d1 = next_pc();
	entry = d0;

	if (d1 & 0x80)
	{
		/* bclr 7 */
		d1 = d1 & 0x7f;
		goto loc_c314;
	}

	if (d1 == 3)
	{
		goto loc_c286;
	}

	if (d1 < 16)
	{
		goto loc_c30c;
	}

	if (d1 == 16)
	{
		goto loc_c29a;
	}
	
	d3 = next_pc();
	d0 = sprites[entry].frame & 0x7f;

	loc_c252:
	if ((d3 & 0x80) == 0)
	{
		goto loc_c26e;
	}

	/* c258 */
	if (sprites[entry].frame & 0x80)
	{
		goto loc_c2aa;
	}

	sprites[entry].frame |= 0x80;

	sprites[entry].x += sprites[entry].u2;
	goto loc_c2aa;

	loc_c26e:
	if ((sprites[entry].frame & 0x80) == 0)
	{
		goto loc_c2aa;
	}

	sprites[entry].frame &= 0x7f;
	sprites[entry].x -= sprites[entry].u2;
	goto loc_c2aa;

	loc_c286:
	d3 = next_pc();
	d0 = d3 & 0x7f;
	sprites[entry].frame &= 0x80;
	sprites[entry].frame |= d0;
	goto loc_c252;

	loc_c29a:
	d0 = next_pc();
	d3 = sprites[entry].frame;
	d3 = (d3 & 0x80) | d0;
	sprites[entry].frame = d3;
	goto loc_c252;
	
	loc_c2aa:
	d1 = sprites[entry].index;

	a4 = get_long(0xf904) + (d1 << 2);
	a4 = get_long(a4);

	d1 = get_word(a4 + d0*2 + 6);
	a2 = a4 + d1 + 2;

	d0 = get_byte(a2++);
	sprites[entry].u2 = d0;
	d0 >>= 1;
	d2 = sprites[entry].x;
	if (d3 & 0x80)
	{
		d2 = d2 - d0;
	}
	else
	{
		d2 = d2 + d0;
	}

	d0 = get_byte(a4+3);
	d0 >>= 1;
	d2 = d2 - d0;
	d3 = sprites[entry].y;
	d0 = get_byte(a2++);
	d0 >>= 1;
	d3 = d3 + d0;
	d1 = get_byte(a4+4);
	d1 >>= 1;
	d3 = d3 - d1;
	sprites[entry].w4 = d2;
	sprites[entry].w5 = d3;
	return;

	loc_c30c:
	set_sprite_data_byte(entry, d1, next_pc());
	return;

	loc_c314:
	d0 = next_pc_word();
	set_sprite_data_word(entry, d1, d0);

	if (d1 < 4 || d1 > 6)
	{
		goto loc_c33c;
	}

	d0 = sprites[entry].frame;
	d3 = d0 & 0x80;
	d0 = d0 & 0x7f;
	goto loc_c2aa;

	loc_c33c:
	/* keep gcc happy */
	d0 = d0;
}

void op_70()
{
	int d0, d1, d2, d3;
	int a2, a4, dummy, entry;
	
	d0 = get_variable(next_pc());
	d1 = next_pc();
	entry = d0;

	LOG(("op70 d0=%d, d1=%d\n", d0, d1));

	if (d1 & 0x80)
	{
		d1 = d1 & 0x7f;
		goto loc_c190;
	}

	d0 = get_variable(next_pc());
	if (d1 == 3)
	{
		goto loc_c106;
	}
	
	if (d1 < 16)
	{
		goto loc_c188;
	}

	if (d1 == 16)
	{
		goto loc_c118;
	}

	d3 = sprites[entry].frame & 0x7f;

	loc_c0cc:
	if ((d0 & 0x80) == 0)
	{
		goto loc_c0ec;
	}

	d0 = d3;
	if ((sprites[entry].frame & 0x80) == 0)
	{
		/* FIXME: not sure about this: */
		/* seg000:0000C0DA                 bne.w   op_70_mainloop */
		sprites[entry].frame |= 0x80;
		goto loc_mainloop;
	}

	d1 = sprites[entry].u2;
	dummy = sprites[entry].x;
	dummy = dummy + d1;
	sprites[entry].x = dummy;
	goto loc_mainloop;

	loc_c0ec:
	d0 = d3;
	if ((sprites[entry].frame & 0x80) == 0)
	{
		/* not sure about this either: loc_c0ecc */
		goto loc_mainloop;
	}

	sprites[entry].frame &= 0x7f;
	d1 = sprites[entry].u2;
	dummy = sprites[entry].x;
	dummy = dummy - d1;
	sprites[entry].x = dummy;
	LOG(("loc_c0ec new sprite.x = %d\n", dummy));
	goto loc_mainloop;

	loc_c106:
	d3 = d0 & 0x7f;
	sprites[entry].frame = (sprites[entry].frame & 0x80) | d3;
	goto loc_c0cc;

	loc_c118:
	d3 = sprites[entry].frame & 0x80;
	d3 = d3 | d0;
	sprites[entry].frame = d3; 

	loc_mainloop:
	d1 = sprites[entry].index;

	/* a4 points to resource */
	a4 = get_long(0xf904) + (d1 << 2);
	a4 = get_long(a4);

	d1 = get_word(a4 + d0*2 + 6);
	a2 = a4 + d1 + 2;
	d0 = get_byte(a2++);
	sprites[entry].u2 = d0;
	d0 >>= 1;
	d2 = sprites[entry].x;
	if (d3 & 0x80)
	{
		goto loc_c15e;
	}

	d2 = d2 + d0;
	goto loc_c160;

	loc_c15e:
	d2 = d2 - d0;

	loc_c160:
	d0 = get_byte(a4 + 3);
	d0 = d0 >> 1;
	d2 = d2 - d0;
	d3 = sprites[entry].y;
	d0 = get_byte(a2++);
	d0 = d0 >> 1;
	d3 = d3 + d0;
	d1 = get_byte(a4 + 4);
	d1 = d1 >> 1;
	d3 = d3 - d1;
	
	sprites[entry].w4 = d2;
	sprites[entry].w5 = d3;
	return;

	loc_c188:
	set_sprite_data_byte(entry, d1, d0);
	return;

	loc_c190:
	d0 = get_variable(next_pc());
	if (d1 != 0x13)
	{
		goto loc_c1f0;
	}

	d1 = sprites[entry].index;
	a4 = get_long(0xf904) + (d1 << 2);
	a4 = get_long(a4);
	d3 = sprites[entry].frame;
	d1 = d3 & 0x7f;
	
	d1 = get_word(a4 + d1*2 + 6);
	a2 = a4 + d1 + 3;

	d3 = d0;
	d1 = get_byte(a2++);
	d0 = d0 - d1;
	sprites[entry].y = d0;
	LOG(("loc_c190 new sprite.y = %d\n", d0));
	d1 = d1 >> 1;
	d3 = d3 - d1;
	d0 = get_byte(a4+4);
	sprites[entry].u7 = d0;
	d0 = d0 >> 1;
	d3 = d3 - d0;
	sprites[entry].w5 = d3;
	return;

	loc_c1f0:
	set_sprite_data_word(entry, d1, d0);
	if (d1 < 4 || d1 > 6)
	{
		return;
	}

	d0 = sprites[entry].frame;
	d3 = d0 & 0x80;
	d0 = d0 & 0x7f;
	goto loc_mainloop;
}

void new_task()
{
	int d0, d1, d2, a2;

	d0 = next_pc();
	d1 = next_pc_word();

	/* enable new task 'd0' at offset 'd1' */

	LOG(("*newtask* op_08 d0=%x d1=%x\n", d0, d1));

	new_task_pc[d0] = d1;
	task_pc[d0] = -1;
	
	d2 = next_pc();
	if (d2 != 0)
	{
		d2--;
		a2 = (d0 * 32) + 6;

		loc_75b4:
		/* d0 = variables to copy. two flags available. copy from
		script, or copy from global variables */
		d0 = next_pc();

		/* useless code, just and with ~0xc0 and be done with it! */
		if (d0 & 0x40)
		{
			d0 = d0 & (~0x40);
			goto loc_75d8;
		}

		if (d0 & 0x80)
		{
			d0 = d0 & 0x7f;
			goto loc_75d8;
		}

		d2 = d2 - d0;

		while (d0 >= 0)
		{
			d0--;
			d1 = next_pc();
			auxvars[a2] = get_variable(d1);
			LOG(("auxvars[%d][%d] == var[%d] (=0x%x)\n", a2/32, a2%32, d1, auxvars[a2]));
			a2++;
		}

		goto loc_75e6;

		loc_75d8:
		d2 = d2 - d0;

		while (d0 >= 0)
		{
			d0--;
			auxvars[a2] = next_pc_word();
			LOG(("auxvars[%d][%d] == 0x%x\n", a2/32, a2%32, auxvars[a2]));
			a2++;
		}

		loc_75e6:
		d2--;
		if (d2 >= 0)
		{
			goto loc_75b4;
		}
	}
}

void destroy_tasks()
{
	int d0, d1, d2;

	/* op_0c: destroy tasks */

	d0 = next_pc();
	d1 = next_pc() & 0x3f;
	d1 = d1 - d0;
	if (d1 >= 0)
	{
		goto loc_76a2;
	}

	/* 0x880 is way over the limit! */
	assert(0); 
	d0 = 0x880;

	loc_76a2:
	d2 = next_pc();
	if (d2 != 3)
	{
		goto loc_76d2;
	}

	while (d1 >= 0)
	{
		task_pc[d0 + d1] = -1;
		new_task_pc[d0 + d1] = -2;
		d1--;
	}

	return;

	loc_76d2:
	if (d2 == 2)
	{
		goto loc_76f6;
	}

	if (d2 > 2)
	{
		assert(0);
		/* bhi decode */
	}

	while (d1 >= 0)
	{
		new_enabled_tasks[d0+d1] = d2;
		enabled_tasks[d1] = d2;
		d1--;
	}

	return;

	loc_76f6:
	d2 = -2;

	while (d1 >= 0)
	{
		new_task_pc[d0+d1] = -2;
		enabled_tasks[d1] = -2;
		d1--;
	}
}

int decode(int current_task, int start_pc)
{
	int leave;
	int compare_op;
	unsigned char opcode;
	unsigned char var1, var2;
	unsigned char imm8, imm8_2;
	short imm16, imm16_2;
	short x, y;
	unsigned short jmpto;
	short saved_d5, saved_d4;
	int sample_index, sample_volume, sample_channel;

	saved_d4 = 0;
	saved_d5 = 0;

	pc = start_pc;
	stack_ptr = 0;

	leave = 0;
	while (leave == 0)
	{
		opcode = next_pc();

		mark_opcode(opcode);
		LOG(("(%d) task: %d: %04x 0x%02x: ", stack_ptr, current_task, pc-1, opcode));
	
		switch(opcode)
		{
			case 0x00:
			/* mov var, imm16 */
			var1 = next_pc();
			imm16 = next_pc_word();
			set_variable(var1, imm16);
	
			LOG(("var[%d] = 0x%x\n", var1, imm16));
			break;
	
			case 0x01:
			/* var1 = var2 */
			var1 = next_pc();
			var2 = next_pc();
			set_variable(var1, get_variable(var2));
	
			LOG(("var[%d] = var[%d] (=0x%x)\n", var1, var2, get_variable(var2)));
			break;

			case 0x02:
			/* var1 += var2 */
			var1 = next_pc();
			var2 = next_pc();
			set_variable(var1, get_variable(var1) + get_variable(var2));

			LOG(("var[%d] += var[%d] (now 0x%x)\n", var1, var2, get_variable(var1)));
			break;

			case 0x03:
			/* var1 += imm16 */
			var1 = next_pc();
			imm16 = next_pc_word();
			set_variable(var1, get_variable(var1) + imm16);

			LOG(("var[%d] += 0x%x (now 0x%x)\n", var1, imm16, get_variable(var1)));
			break;
	
			case 0x04:
			/* call imm16 */
			imm16 = next_pc_word();
			push(pc);
			pc = imm16;
	
			LOG(("call 0x%x\n", imm16));
			break;
	
			case 0x05:
			pc = pop();
			LOG(("ret\n"));
			break;
	
			case 0x06:
			LOG(("yield\n"));
			leave = 1;
			break;
	
			case 0x07:
			jmpto = next_pc_word();
			pc = jmpto;
			LOG(("jmp 0x%x\n", jmpto));
			break;
	
			case 0x08:
			new_task();
			LOG(("\n"));
			break;
	
			case 0x09:
			/* dbf */
			var1 = next_pc();
			jmpto = next_pc_word();
	
			imm16 = get_variable(var1) - 1;
			set_variable(var1, imm16);
	
			if (imm16 != 0)
			{
				pc = jmpto;
			}
	
			LOG(("dbf var[%d], goto 0x%x(now=0x%x)\n", var1, jmpto, imm16));
			break;
	
			case 0x0a:
			compare_op = next_pc();
	
			/* lvalue variable */
			var1 = next_pc();
			imm16 = get_variable(var1);
	
			if (compare_op & 0x80)
			{
				/* rvalue variable */
				var2 = next_pc();
				jmpto = next_pc_word();
				imm16_2 = get_variable(var2);
				LOG(("if var[%d] %s var[%d] goto 0x%x(lv=%x, rv=%x)\n", var1, cmpopar[compare_op & 0x07], var2, jmpto, imm16, imm16_2));
			}
			else if (compare_op & 0x40)
			{
				/* rvalue imm16 */
				imm16_2 = next_pc_word();
				jmpto = next_pc_word();
				LOG(("(imm16) if var[%d] %s %d goto 0x%x (lv=%x)\n", var1, cmpopar[compare_op & 0x07], imm16_2, jmpto, imm16));
			}
			else
			{
				/* rvalue unsigned imm8 */
				imm16_2 = next_pc();
				jmpto = next_pc_word();
				LOG(("(imm8) if var[%d] %s %d goto 0x%x (lv=%x)\n", var1, cmpopar[compare_op & 0x07], imm16_2, jmpto, imm16));
			}
	
			switch(compare_op & 0x07)
			{
				case 0x00:
				if (imm16 == imm16_2)
				{
					pc = jmpto;
				}
				break;
	
				case 0x01:
				if (imm16 != imm16_2)
				{
					pc = jmpto;
				}
				break;
	
				case 0x02:
				if (imm16 > imm16_2)
				{
					pc = jmpto;
				}
				break;
	
				case 0x03:
				if (imm16 >= imm16_2)
				{
					pc = jmpto;
				}
				break;
	
				case 0x04:
				if (imm16 < imm16_2)
				{
					pc = jmpto;
				}
				break;
	
				case 0x05:
				if (imm16 <= imm16_2)
				{
					pc = jmpto;
				}
				break;
	
				default:
				LOG(("UNKNOWN COMPARE OPERATOR! %d\n", compare_op & 0x07));
				assert(0);
				break;
			}
			break;
	
			case 0x0b:
			/* set palette imm8 */
			imm8 = next_pc();
	
			set_palette(imm8);
			LOG(("set palette %d\n", imm8));
			break;
	
			case 0x0c:
			destroy_tasks();
			LOG(("\n"));
			break;

			case 0x0d:
			/* select screen */
			imm8 = next_pc();
	
			select_screen(imm8);
			LOG(("select screen %d\n", imm8));
			break;

			case 0x0e:
			/* fill screen */
			imm8 = next_pc();
			imm8_2 = next_pc();
	
			fill_screen(imm8, imm8_2);
			LOG(("fill screen%d with color %d\n", imm8, imm8_2));
			break;

			case 0x0f:
			/* copy screen */
			imm8 = next_pc();
			imm8_2 = next_pc();
	
			copy_screen(imm8_2, imm8);
			LOG(("screen%d <= screen%d\n", imm8_2, imm8));
			break;
	
			case 0x10:
			imm8 = next_pc();
			update_screen(imm8);
			LOG(("update screen %d\n", imm8));
			break;

			case 0x11:
			/* terminate task */
			leave = 1;
			pc = -1;
			LOG(("terminate task!\n"));
			break;
	
			case 0x13:
			var1 = next_pc();
			var2 = next_pc();
	
			LOG(("var[%d] -= var[%d]\n", var1, var2));

			set_variable(var1, get_variable(var1) - get_variable(var2));
			break;
	
			case 0x14:
			var1 = next_pc();
			imm16 = next_pc_word();
			set_variable(var1, get_variable(var1) & imm16);
	
			LOG(("var[%d] &= 0x%x\n", var1, imm16));
			break;
	
			case 0x15:
			var1 = next_pc();
			imm16 = next_pc_word();
			set_variable(var1, imm16 | get_variable(var1));
			LOG(("var[%d] |= 0x%x\n", var1, imm16));
			break;

			case 0x17:
			var1 = next_pc();
			imm8 = next_pc();
			imm16 = extl(get_variable(var1)) >> imm8;
			set_variable(var1, imm16);
			LOG(("var[%d] >>= %d (now 0x%x)\n", var1, imm8, imm16));
			break;

			case 0x18:
			LOG(("play sample"));
			sample_index = next_pc();
			sample_volume = next_pc();
			sample_channel = next_pc();
			play_sample(sample_index, sample_volume, sample_channel);
			break;
	
			case 0x19:
			/* load screen imm16 */
			imm16 = next_pc_word();
			LOG(("load screen %d (0x%x)\n", imm16, imm16));

			if (imm16 < 16000)
			{
				load_room_screen(current_room, imm16);
			}
			else if (imm16 >= 17000 && imm16 <= 17100)
			{
				/* original code reads 181 sectors here */
				toggle_aux(0);
				set_variable(220, imm16 % 10);

				next_script = (imm16 / 10) % 10;
				next_script++;

				/* hack: original game now loads specific
				 * sectors into memory, replacing the script
				 * currently running. when an animation is
				 * to be loaded, the executable code is
				 * overwritten as well. although this looks
				 * like a hack, consider the alternative :)
				 */
				if (next_script == 10)
				{
					next_script = 8;
				}
				else if (next_script == 7)
				{
					play_music_track(35, 0);
					play_animation("MAKE2MB.BIN", 0x109a);
					play_music_track(36, 0);
					play_animation("MID2.BIN", 0);
					stop_music();
					next_script = 6;
					leave = 1;
				}
				else if (next_script == 6)
				{
					play_music_track(37, 0);
					play_animation("END1.BIN", 0);
					play_music_track(38, 0);
					play_animation("END2.BIN", 0);
					play_music_track(39, 0);
					play_animation("END3.BIN", 0);
					play_music_track(40, 0);
					play_animation("END4.BIN", 0);

					/* return to password selection */
					next_script = 7;
					leave = 1;
				}

				//leave = 1;
			}
			else
			{
				assert(0);
			}
			break;
	
			case 0x1a:
			imm8 = next_pc();
			LOG(("play audio track %d\n", imm8));

			if (imm8 == 0)
			{
				/* stop music */
				stop_music();
			}
			else if (imm8 >= 100)
			{
				/* one shot */
				imm8 = imm8 - 100 - 1;

				LOG(("play single audio track %d\n", imm8));
				play_music_track(imm8, 0);
			}
			else
			{
				imm8 = imm8 - 1;
				LOG(("play audio track %d\n", imm8));
	
				play_music_track(imm8, 1);
			}
			break;

			case 0x1c:
			var1 = next_pc();
			imm16 = get_variable(var1);

			imm8 = next_pc();
			LOG(("switch of variable %d over %d cases\n", var1, imm8));

			while (imm8 > 0)
			{
				imm16_2 = extw(next_pc());
				jmpto = next_pc_word();
			
				if (imm16 == imm16_2)
				{
					pc = jmpto;
					break;
				}

				imm8--;
			}
			break;

			case 0x1d:
			var1 = next_pc();
			imm16 = get_variable(var1);

			imm8 = next_pc();
			LOG(("switch of variable %d over %d cases\n", var1, imm8));

			while (imm8 > 0)
			{
				imm16_2 = next_pc_word();
				jmpto = next_pc_word();
			
				if (imm16 == imm16_2)
				{
					pc = jmpto;
					break;
				}

				imm8--;
			}
			break;
	
			case 0x1e:
			var1 = next_pc();
			jmpto = next_pc_word();
	
			if (get_variable(var1) == 0)
			{
				pc = jmpto;
			}
	
			LOG(("if var[%d] == 0 goto 0x%x\n", var1, jmpto));
			break;
	
			case 0x1f:
			var1 = next_pc();
			jmpto = next_pc_word();
			
			if (get_variable(var1) != 0)
			{
				pc = jmpto;
			}
	
			LOG(("if var[%d] != 0x00 goto 0x%x(lv=0x%x)\n", var1, jmpto, get_variable(var1)));
			break;

			case 0x21:
			imm8 = next_pc() % 32;
			LOG(("play death animation %d\n", imm8));
			play_death_animation(imm8);
			break;
	
			case 0x24:
			op_24();
			LOG(("\n"));
			break;
	
			case 0x25:
			LOG(("add sprite\n"));
			add_sprite();
			break;
	
			case 0x26:
			draw_sprites();
			break;
	
			case 0x27:
			op_27();
			break;

			case 0x28:
			LOG(("collision against set\n"));
			collision_against_set();
			LOG(("\n"));
			break;

			case 0x29:
			imm8 = next_pc();
			LOG(("remove sprite %d\n", imm8));
			remove_sprite(imm8);
			break;
	
			case 0x2a:
			op_2a();
			LOG(("\n"));
			break;

			case 0x2b:
			op_2b();
			LOG(("\n"));
			break;

			case 0x2c:
			var1 = next_pc();
			imm16 = get_variable(var1);

			x = extw(next_pc());
			y = extw(next_pc());

			LOG(("move sprite %d by (%d, %d)\n", imm16, x, y));

			move_sprite_by(imm16, x, y);
			break;

			case 0x2d:
			case 0x2e:
			case 0x2f:
			case 0x30:
			LOG(("nop\n"));
			break;

			case 0x31:
			var1 = next_pc();
			imm16 = get_variable(var1);

			flip_sprite(imm16);
			LOG(("mirror/unmirror sprite %d\n", imm16));
			break;

			case 0x32:
			imm16 = get_variable(next_pc());

			mirror_sprite(imm16);
			LOG(("mirror sprite %d\n", imm16));
			break;

			case 0x33:
			imm16 = get_variable(next_pc());
			unmirror_sprite(imm16);
			LOG(("unmirror sprite %d\n", imm16));
			break;

			case 0x34:
			LOG(("d4=0\n"));
			saved_d4 = 0;
			break;

			case 0x35:
			LOG(("d5=0\n"));
			saved_d5 = 0;
			break;

			case 0x36:
			saved_d4 = next_pc_word();
			LOG(("saved_d4 = 0x%x\n", saved_d4));
			break;

			case 0x37:
			saved_d5 = next_pc_word();
			LOG(("saved_d5 = 0x%x\n", saved_d5));
			break;

			case 0x38:
			imm16 = next_pc_word();
			saved_d4 = saved_d4 + imm16;
			LOG(("d4 += %d (now %d)\n", imm16, saved_d4));
			break;

			case 0x39:
			imm16 = next_pc_word();
			saved_d5 = saved_d5 + imm16;
			LOG(("d5 += %d (now %d)\n", imm16, saved_d5));
			break;

			case 0x3a:
			imm16 = next_pc_word();
			saved_d4 &= imm16;
			LOG(("d4 &= 0x%x (now=%x)\n", imm16, saved_d4));
			break;

			case 0x3b:
			imm16 = next_pc_word();
			saved_d5 &= imm16;
			LOG(("d5 &= 0x%x (now=%x)\n", imm16, saved_d5));
			break;

			case 0x3e:
			imm8 = next_pc();
			LOG(("d4 <<= %d\n", imm8));
			saved_d4 <<= imm8;
			break;

			case 0x3f:
			imm8 = next_pc();
			LOG(("d5 <<= %d\n", imm8));
			saved_d5 <<= imm8;
			break;
	
			case 0x40:
			imm8 = next_pc();
			saved_d4 = extl(saved_d4) >> imm8;
			LOG(("saved_d4 >>= %d (now 0x%x)\n", imm8, saved_d4));
			break;

			case 0x41:
			imm8 = next_pc();
			saved_d5 = extl(saved_d5) >> imm8;
			LOG(("saved_d5 >>= %d (now 0x%x)\n", imm8, saved_d5));
			break;

			case 0x42:
			saved_d4 = saved_d5;
			LOG(("saved_d4 = saved_d5\n"));
			break;

			case 0x43:
			saved_d5 = saved_d4;
			LOG(("d5 = d4\n"));
			break;

			case 0x4a:
			var1 = next_pc();
			saved_d4 = get_variable(var1);
			LOG(("saved_d4 = var[%d] (=0x%x)\n", var1, saved_d4));
			break;

			case 0x4b:
			var1 = next_pc();
			saved_d5 = get_variable(var1);
			LOG(("saved_d5 = var[%d] (=0x%x)\n", var1, saved_d5));
			break;
			
			case 0x4c:
			var1 = next_pc();
			set_variable(var1, saved_d4);
			LOG(("var[%d] = saved_d4 (=0x%x)\n", var1, saved_d4));
			break;

			case 0x4d:
			var1 = next_pc();
			set_variable(var1, saved_d5);
			LOG(("var[%d] = saved_d5 (=0x%x)\n", var1, saved_d5));
			break;

			case 0x54:
			compare_op = next_pc() & 0x07;
			imm16 = next_pc_word();
			jmpto = next_pc_word();

			LOG(("if d4 %s 0x%04x goto 0x%x(lv=%x)\n", cmpopar[compare_op], imm16, jmpto, saved_d4));
			
			switch(compare_op)
			{
				case 0x00:
				if (saved_d4 == imm16)
				{
					pc = jmpto;
				}
				break;

				case 0x01:
				if (saved_d4 != imm16)
				{
					pc = jmpto;
				}
				break;

				case 0x02:
				if (saved_d4 > imm16)
				{
					pc = jmpto;
				}
				break;

				case 0x03:
				if (saved_d4 >= imm16)
				{
					pc = jmpto;
				}
				break;

				case 0x04:
				if (saved_d4 < imm16)
				{
					pc = jmpto;
				}
				break;
				                  
				case 0x05:
				if (saved_d4 <= imm16)
				{
					pc = jmpto;
				}
				break;
				
				default:
				LOG(("UNKNOWN COMPARE OP %d\n", compare_op));
			}
			break;

			case 0x56:
			/* var%d = 0 */
			var1 = next_pc();
			set_variable(var1, 0);
	
			LOG(("var[%d] = 0\n", var1));
			break;

			case 0x5c:
			var1 = next_pc();
			set_variable(var1, get_variable(var1) + saved_d4);
			LOG(("var[%d] += d4 (now 0x%04x)\n", var1, get_variable(var1)));
			break;

			case 0x5d:
			var1 = next_pc();
			set_variable(var1, get_variable(var1) + saved_d5);
			LOG(("var[%d] += d5 (now 0x%04x)\n", var1, get_variable(var1)));
			break;

			case 0x5e:
			var1 = next_pc();         
			saved_d4 = saved_d4 - get_variable(var1);
			LOG(("d4 -= var[%d] (now 0x%x)\n", var1, saved_d4));
			break;

			case 0x5f:
			var1 = next_pc();
			saved_d5 = saved_d5 - get_variable(var1);
			LOG(("d5 -= var[%d] (now 0x%x)\n", var1, saved_d5));
			break;

			case 0x60:
			var1 = next_pc();
			imm16 = get_variable(var1) - saved_d4;
			set_variable(var1, imm16);
			LOG(("var[%d] -= d4 (now 0x%x)\n", var1, imm16));
			break;

			case 0x66:
			compare_op = next_pc() & 0x07;
			var1 = next_pc();
			imm16 = get_variable(var1);
			jmpto = next_pc_word();
			                          
			LOG(("if d4 %s var[%d] goto 0x%x(lv=%x, rv=%x)\n", cmpopar[compare_op], var1, jmpto, saved_d4, imm16));
			
			switch(compare_op)
			{
				case 0x00:
				if (saved_d4 == imm16)
				{
					pc = jmpto;
				}
				break;

				case 0x01:
				if (saved_d4 != imm16)
				{
					pc = jmpto;
				}
				break;

				case 0x02:
				if (saved_d4 > imm16)
				{
					pc = jmpto;
				}
				break;

				case 0x03:
				if (saved_d4 >= imm16)
				{
					pc = jmpto;
				}
				break;

				case 0x04:
				if (saved_d4 < imm16)
				{
					pc = jmpto;
				}
				break;

				case 0x05:
				if (saved_d4 <= imm16)
				{
					pc = jmpto;
				}
				break;
				
				default:
				LOG(("UNKNOWN COMPARE OP %d\n", compare_op));
				assert(0);
			}
			break;

			case 0x67:
			/* FIXME: collapse! */
			compare_op = next_pc() & 0x07;
			var1 = next_pc();
			imm16 = get_variable(var1);
			jmpto = next_pc_word();

			LOG(("if d5 %s var[%d] goto 0x%x(lv=%x, rv=%x)\n", cmpopar[compare_op], var1, jmpto, saved_d4, imm16));
			
			switch(compare_op)
			{
				case 0x00:
				if (saved_d5 == imm16)
				{
					pc = jmpto;
				}
				break;

				case 0x01:
				if (saved_d5 != imm16)
				{
					pc = jmpto;
				}
				break;

				case 0x02:
				if (saved_d5 > imm16)
				{
					pc = jmpto;
				}
				break;

				case 0x03:
				if (saved_d5 >= imm16)
				{
					pc = jmpto;
				}
				break;

				case 0x04:
				if (saved_d5 < imm16)
				{
					pc = jmpto;
				}
				break;

				case 0x05:
				if (saved_d5 <= imm16)
				{
					pc = jmpto;
				}
				break;
				
				default:
				LOG(("UNKNOWN COMPARE OP %d\n", compare_op));
				assert(0);
			}
			break;

			case 0x68:
			imm8 = next_pc(); /* count */

			LOG(("switch of d4 with %d cases (d4 == 0x%x)\n", imm8, saved_d4));
			while (imm8 > 0)
			{
				imm16 = next_pc_word();
				jmpto = next_pc_word();
				LOG(("on 0x%x --> goto 0x%x\n", imm16, jmpto));

				if (imm16 == saved_d4)
				{
					pc = jmpto;
					break;
				}

				imm8--;
			}
			break;

			case 0x69:
			saved_d5 = extw(next_pc());
			LOG(("d5 = %d\n", saved_d5));
			break;

			case 0x6a:
			jmpto = next_pc_word();
			LOG(("if d4 == 0 goto 0x%x (d4=0x%x)\n", jmpto, saved_d4));
			if (saved_d4 == 0)
			{
				pc = jmpto;	
			}
			break;

			case 0x6b:
			jmpto = next_pc_word();
			LOG(("if d5 == 0 goto 0x%x (d5=0x%x)\n", jmpto, saved_d5));
			if (saved_d5 == 0)
			{
				pc = jmpto;	
			}
			break;     

			case 0x6c:
			case 0x6d:
			jmpto = next_pc_word();
			LOG(("if d4 != 0 goto 0x%x (d4=0x%x)\n", jmpto, saved_d4));
			if (saved_d4 != 0)
			{
				pc = jmpto;	
			}
			break;

			case 0x6e:
			saved_d4 = extw(next_pc());
			LOG(("d4 = %d (0x%x)\n", saved_d4, saved_d4));
			break;


			case 0x6f:
			saved_d5 = extw(next_pc());
			LOG(("d5 = %d (0x%x)\n", saved_d5, saved_d5));
			break;

			case 0x70:
			op_70();
			LOG(("\n");
			break;
	
			case 0x71:
			var1 = next_pc();
			set_variable(var1, get_variable(var1) + 1));
			LOG(("var[%d]++\n", var1));
			break;
	
			case 0x72: 
			var1 = next_pc();
			set_variable(var1, get_variable(var1) - 1);
			LOG(("var[%d]--\n", var1));
			break;

			case 0x73:
			saved_d4++;
			LOG(("d4++ (now 0x%x)\n", saved_d4));
			break;

			case 0x74:
			saved_d5++;
			LOG(("d5++ (now 0x%x)\n", saved_d5));
			break;

			case 0x75:
			saved_d4--;
			LOG(("d4-- (now 0x%x)\n", saved_d4));
			break;

			case 0x76:
			saved_d5--;               
			LOG(("d5-- (now 0x%x)\n", saved_d5));
			break;

			case 0x77:
			var1 = next_pc();
			imm8 = next_pc();
			imm16 = get_variable(var1);
			imm16 <<= imm8;   /* fixme: asl ? */
			set_variable(var1, imm16);

			LOG(("var[%d] asl %d\n", var1, imm8)); 
			break;

			case 0x78:
			var1 = next_pc();
			imm8 = next_pc();
			
			imm16 = get_variable(var1) / (1 << imm8);
			set_variable(var1, imm16);

			LOG(("asr %d var[%d] (now 0x%x)\n", imm8, var1, imm16));
			break;

			case 0x7d:
			imm16 = get_variable(next_pc());
			is_last_frame_in_animation(imm16);
			LOG(("check if current frame exceeded frame count for sprite %d\n", imm16));
			break;

			case 0x7e:
			var1 = next_pc();
			imm16 = get_variable(var1);
			
			x = get_variable(next_pc());
			move_sprite_by(imm16, x, 0);

			LOG(("shift sprite %d delta_x=%d (%x)\n", imm16, x, x));
			break;

			case 0x7f:
			var1 = next_pc();         
			imm16 = get_variable(var1);
			
			y = get_variable(next_pc());
			move_sprite_by(imm16, 0, y);

			LOG(("shift sprite %d delta_y=%d (%x)\n", imm16, y, y));
			break;

			case 0x80:
			toggle_aux(0);
			LOG(("using_aux = 0\n"));
			break;

			case 0x81:
			toggle_aux(1);
			LOG(("using_aux = 1\n"));
			break;

			case 0x82:                
			imm8 = next_pc();
			LOG(("breakpoint %d\n", imm8));
			break;

			case 0x83:
			copy_global_to_tls();
			break;

			case 0x84:
			copy_tls_to_global();
			break;
	
			case 0x87:
			var1 = next_pc();
			imm16 = next_pc_word();
			set_variable(var1, imm16);
			LOG(("var[%d] = 0x%x\n", var1, imm16));
			break;

			case 0x88:
			var1 = next_pc();
			var2 = next_pc();         
			imm16 = get_variable(var1) + get_variable(var2)*2;
			pc = get_word(script_ptr + imm16);
			LOG(("indirect jump to 0x%x\n", pc));
			break; 

			case 0x89:
			op_89();
			LOG(("\n"));
			break;
	
			case 0x8a:
			var1 = next_pc(); // d0
			imm16 = next_pc_word(); // d1
			var2 = next_pc(); // d2

			imm16_2 = imm16 + get_variable(var2);
			LOG(("read byte %d from array at 0x%x into var[%d] (%04x)\n", get_variable(var2), imm16, var1, extw(get_byte(script_ptr + imm16_2))));
			
			set_variable(var1, extw(get_byte(script_ptr + imm16_2)));
			break;
	
			case 0x8b:
			var1 = next_pc();
			imm16 = get_variable(var1);
			set_palette(imm16);
	
			LOG(("set palette var[%d] (%d)\n", var1, imm16));
			break;

			case 0x8c:
			imm8 = next_pc();
			LOG(("scroll reg = %d\n", extw(imm8)));
			set_scroll(extw(imm8));
			break;

			/*
			case 0xff:
			pc = 0xfff;
			return;
			*/

			default:
			LOG(("opcode 0x%x unknown\n", opcode));
			panic("unknown!");
			break;
		}
	}

	return pc;
}

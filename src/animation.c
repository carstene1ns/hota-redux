/*
 * Heart of The Alien Redux: Cutscene and deathscene animation player
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
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <SDL.h>

#include "vm.h"
#include "lzss.h"
#include "debug.h"
#include "cd_iso.h"
#include "render.h"
#include "common.h"

///////
extern int quit;
void rest(int fps);

extern short variables[256];
extern SDL_Surface *screen;
extern unsigned char memory[];
extern SDL_Color palette[256];

/* used for 4->8 bit convertion */
static unsigned char dummy[304*192/2];
static unsigned char screen0[192*304];
static unsigned char screen2[192*304];

static void post_render(int fps)
{
	check_events();
	rest(fps);
}

static void copy_to_screen()
{
	set_scroll(0); /////////
	render(screen0);
	SDL_UpdateRect(screen, 0, 0, 0, 0);
}

static void draw_pixel(char *out, int offset, int color)
{
	if (offset >= 304*192)
	{
		LOG(("out of screen %d\n", offset));
		return;
	}

	out[offset] = color & 0xf;
}

static void fillline(unsigned char *screen, int offset, int count, int color)
{
	while (count > 0)
	{
		draw_pixel(screen, offset++, color);
		count--;
	}
}

static void unpack_animation_delta(int a2, char *out)
{
	int a5;
	char *src;
	int d0, d2, d3, d5, d6, d7, d1;

	a5 = a2;

	/* bug in the original code, assumes a2 < 0x100 */
	d0 = get_word(a2);
	a2 += 2;
	a5 += d0;

	/*
	Original game uses:
	d0 = (d0 & 0xff00) | get_byte(a2);
	d0 = d0 >> 4;
	*/

	d0 = get_byte(a2) >> 4;
	*out++ = d0;

	d6 = 0;
	d7 = 0x0f;

	loc_d046:
	d0 = get_long(a5);
	a5 += 4;
	d1 = 0x1f;
	goto loc_d062;

	loc_d04c:
	if (d6 != 0)
	{
		d2 = get_byte(a2) >> 4;
	}
	else
	{
		d2 = get_byte(a2++) & d7;
	}

	*out++ = d2;
	d6 = ~d6;

	loc_d05e:
	d1--;
	if (d1 < 0)
	{
		goto loc_d046;
	}

	loc_d062:
	if (d0 & (1 << d1))
	{
		goto loc_d04c;
	}

	if (d6 != 0)
	{
		d3 = get_byte(a2++);
	}
	else
	{
		d3 = get_byte(a2++) & d7;
		d3 <<= 8;
		d3 |= get_byte(a2);
		d3 >>= 4;
	}

	d1--;
	if (d1 < 0)
	{
		goto loc_d0a2;
	}

	loc_d07e:
	d2 = 0;
	d5 = 1;

	loc_d082:
	if (d0 & (1 << d1))
	{
		goto loc_d0a8;
	}

	d1--;
	if (d1 < 0)
	{
		goto loc_d09c;
	}

	loc_d08a:
	if ((d0 & (1 << d1)) == 0)
	{
		goto loc_d090;
	}

	d2 += d5;

	loc_d090:
	d5 = d5 + d5;
	d1--;
	if (d1 >= 0)
	{
		goto loc_d082;
	}

	d0 = get_long(a5);
	a5 += 4;
	d1 = 0x1f;
	goto loc_d082;

	loc_d09c:
	d0 = get_long(a5);
	a5 += 4;
	d1 = 0x1f;
	goto loc_d08a;

	loc_d0a2:
	d0 = get_long(a5);
	a5 += 4;
	d1 = 0x1f;
	goto loc_d07e;

	loc_d0a8:
	d2 = d2 + d5;
	if (d2 >= 3000)
	{
		return;
	}

	d2++;
	src = out - d3;
	while (d2 >= 0)
	{
		*out++ = *src++;
		d2--;
	}

	goto loc_d05e;
}

static void anim_interesting(int a1, int a2, int a3, int d3)
{
	unsigned char *out;
	int d1, d2, d4, d5;
	int count, offset;
	int bitmask, color;

	/* d3 = color bitmask */
	out = screen0;
	d5 = 0;
	d1 = 0;

	loc_d2d8:
	if (d3 & (1 << d5))
	{
		goto loc_d2ea;
	}

	loc_d2dc:
	d5++;
	if (d5 != 16)
	{
		goto loc_d2d8;
	}

	return;

	loc_d2ea:
	d2 = d5 & 0x0f;	/* current color */
	d2 = (d2 << 8) | d2;
	color = d2;

	/* each bit is a different drawing pattern, 1x1 etc */
	bitmask = get_byte(a1++); 
	if ((bitmask & 1))
	{
		/* start drawing a line, and then continue from there
		 * where each line supplies deltax and delta-count
		 */
		loc_d308:
		offset = get_word(a1);
		a1 += 2;     	
		count = get_byte(a1++);
		if (count == 0xff)
		{
			/* two bytes */
			count = get_byte(a1++) + 0xff;
		}
	
		count++;
		fillline(out, offset, count, d2); 
	
		loc_d324:
		if (get_byte(a2) == 9 && get_byte(a3) == 9)
		{
			a2++;
			a3++;
			goto loc_d308;
		}
	
		if (get_byte(a2) == 8 && get_byte(a3) == 8)
		{
			a2++;
			a3++;
			goto loc_d36e;
		}
	
		offset += 304;
		d4 = extn(get_byte(a2++));
		offset += d4;
		count -= d4;
		d4 = extn(get_byte(a3++));
		count += d4;

		fillline(out, offset, count, d2); 
		goto loc_d324;
	}

	loc_d36e:
	if (bitmask & 0x10)
	{
		while (1)
		{
			/* block */
			offset = get_byte(a1++);
			if (offset == 0xff)
			{
				break;
			}
	
			offset = (offset << 8) | get_byte(a1++);
	
			/* highnibble=width-2, lownibble=height-1 */
			count = get_byte(a1++);
			d4 = count & 0x0f;
			count = (count >> 4) + 2;
			d4++;

			LOG(("block offset=%d w=%d h=%d color=%d\n", offset, count, d4, d2));

			loc_d394:
			fillline(out, offset, count, d2); 
			offset += 304;
			d4--;
			if (d4 >= 0)
			{
				goto loc_d394;
			}
		}
	}

	if (bitmask & 0x4)
	{
		/* horizontal line */
		while (1)
		{
			offset = get_byte(a1++);
			if (offset == 0xff)
			{
				break;
			}

			offset = (offset << 8) | get_byte(a1++);
			count = get_byte(a1++) + 1;

			LOG(("horizontal line offset=%d count=%d\n", offset, count));
			fillline(out, offset, count, d2); 
		}
	}

	if (bitmask & 0x8)
	{
		/* d3ce: vertical line */
		while (1)
		{
			offset = get_byte(a1++);
			if (offset == 0xff)
			{
				break;
			}
	
			offset = (offset << 8) | get_byte(a1++);
			count = get_byte(a1++);

			LOG(("vertical line offset=%d count=%d\n", offset, count));

			while (count >= 0)
			{
				draw_pixel(out, offset, color);
				offset = offset + 304;
				count--;
			}
		}
	}

	if (bitmask & 0x80)
	{
		while (1)
		{
			d1 = get_byte(a1++);
			if (d1 == 0xff)
			{
				break;
			}
		
			d1 <<= 8;
			d1 |= get_byte(a1++);
			d4 = get_word(a1) << 8;
			d4 |= get_byte(a1+2);
			a1 += 3;
			if (d4 & (1 << 0x17))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x16)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x15)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x14)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x13)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1 += 300;
			if ((d4 & (1 << 0x12)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x11)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x10)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x0f)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x0e)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1+=300;
			if ((d4 & (1 << 0x0d)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x0c)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			draw_pixel(out, d1, d2);
		
			d1++;
			if ((d4 & (1 << 0x0b)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x0a)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1+=300;
			if ((d4 & (1 << 0x09)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x08)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x07)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x06)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x05)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1+=300;
			if ((d4 & (1 << 0x04)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x03)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x02)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x00)))
			{
				draw_pixel(out, d1, d2);
			}
		}
	}

	if (bitmask & (1 << 6))
	{
		while (1)
		{		
			d1 = get_byte(a1++);
			if (d1 == 0xff)
			{
				break;
			}
		
			d1 <<= 8;
			d1 |= get_byte(a1++);
			d4 = get_word(a1);
			a1 += 2;
		
			if ((d4 & (1 << 0x0f)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x0e)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x0d)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x0c)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1+=301;
			if ((d4 & (1 << 0x0b)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x0a)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x09)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x08)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1+=301;
			if ((d4 & (1 << 0x07)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x06)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x05)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x04)))
			{
				draw_pixel(out, d1, d2);
			}
		
		
			d1 += 301;
			if ((d4 & (1 << 0x03)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x02)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x01)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0x00)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
		}
	}

	if (bitmask & (1 << 5))
	{
		while (1)
		{
			d1 = get_byte(a1++);
			if (d1 == 0xff)
			{
				break;
			}
		
			d1 <<= 8;
			d1 |= get_byte(a1++);
			d4 = get_byte(a1++);
		
			if ((d4 & (1 << 7)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 6)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 5)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1+=302;
			if ((d4 & (1 << 4)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			draw_pixel(out, d1, d2);
			d1++;
		
			if ((d4 & (1 << 3)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1+=302;
			if ((d4 & (1 << 2)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 1)))
			{
				draw_pixel(out, d1, d2);
			}
		
			d1++;
			if ((d4 & (1 << 0)))
			{
				draw_pixel(out, d1, d2);
			}
		}
	}
	
	if ((bitmask & 0x2))
	{
		/* collection of pixels */
		while (1)
		{
			offset = get_byte(a1++);
			if (offset == 0xff)
			{
				break;
			}
	
			offset = (offset << 8) | get_byte(a1++);

			draw_pixel(out, offset, d2);
		}
	}

	goto loc_d2dc;
}

void flip_screens(int d0)
{
	if (d0 == 0)
	{
		memcpy(screen2, screen0, sizeof(screen0));
	}
	else
	{
		memcpy(screen0, screen2, sizeof(screen0));
	}
}

void decompress_backdrop(unsigned char *out, int a2, int a3)
{
	int x, y;
	unsigned char *dummyp, c;

	/* unlzss backdrop */
	unlzss(dummy, a2, a3);

	/* convert 4bpp->8bpp */
	dummyp = dummy;
	for (y=0; y<192; y++)
	{
		for (x=0; x<304/2; x++)
		{
			c = *dummyp++;
			*out++ = (c >> 4);
			*out++ = c & 0xf;
		}
	}
}

int play_sequence(int offset, int fps)
{
	int d0, d1, d3, d4, d6, d7;
	int a0, a1, a2, a3, a4, a5;
	int byte_0_7FF96 = 0;

	/* This code is obscure. I have no idea why it's written this way,
	 * or how the hell it works, but .. it just works!.
	 */
	set_scroll(0);

	/* d0c6 */
	a5 = offset;
	d7 = 0;
	d6 = 1;
	a2 = get_long(a5);
	a5 += 4;
	if (a2 == 0)
	{
		goto loc_a2_is_0;
	}

	if (a2 == 1)
	{
		goto loc_a2_is_1;
	}

	if (a2 == 3)
	{
		goto loc_a2_is_3;
	}

	if (a2 != 2)
	{
		goto loc_a2_ne_2;
	}

	/* else d0f2 */	
	d6 = 3;
	a2 = get_long(a5);
	a5 += 4;
	goto loc_a2_ne_2;

	loc_a2_is_3:
	/* move.b  #2,($C0401).l */
	a2 = get_long(a5);
	a5 += 4;

	loc_a2_ne_2:
	/* decompress into screen0 */
	a3 = get_long(a5);
	a5 += 4;
	decompress_backdrop(screen0, a2, a3);
	
	d0 = 0;
	d1 = 3;
	flip_screens(d0);
	goto loc_d15e;

	loc_a2_is_1:
	d6 = 0;
	a5 += 8;
	d0 = 3;
	d1 = 0;
	flip_screens(d0);

	d0 = 3;
	d1 = 4;
	flip_screens(d0);

	/* decompress into screen2 */
	a2 = get_long(a5);
	a3 = get_long(a5+4);
	a5 += 8;
	decompress_backdrop(screen2, a2, a3);
	goto loc_d15e;

	loc_a2_is_0:
	/* decompress TWO screens */
	d6 = 2;
	a2 = get_long(a5);
	a3 = get_long(a5+4);
	a5 += 8;
	decompress_backdrop(screen0, a2, a3);
	a2 = get_long(a5);
	a3 = get_long(a5+4);
	a5 += 8;
	decompress_backdrop(screen2, a2, a3);	

	loc_d15e:
	d0 = 0;

	loc_d160:
	a1 = a5;
	d0 = get_word(a1);
	a1 += 2;
	if (d0 == 0)
	{
		return 0;
	}

	a5 = a5 + d0;
	if (d6 == 0)
	{
		goto loc_d194;
	}

	if (d6 >= 8)
	{
		goto loc_d198;
	}

	if (get_byte(a1) != 0)
	{
		goto loc_d1ae;
	}

	if (d6 == 2)
	{
		goto loc_d1ae;
	}

	if (d6 == 3 || d6 == 1)
	{
		goto loc_d1a4;
	}

	if (d6 != 0)
	{
		goto loc_d198;
	}

	loc_d194:
	d6 = 8;
	goto loc_d1f4;

	loc_d198:
	d0 = d6;
	d1 = 5;
	flip_screens(d0);
	d6 += 8;
	goto loc_d1f4;

	loc_d1a4:
	d0 = 3;
	d1 = 0;
	flip_screens(d0);
	goto loc_d1f4;

	loc_d1ae:
	/* what the hell ? */
	if (get_byte(a1) == 1)
	{
		goto loc_d1f4;
	}

	variables[255] = 100;
	// a4 = screen0
	d0 = 0;
	d1 = 0;
	a1 += 2;
	d0 = get_byte(a1++) * 304;   /* y */
	d1 = get_byte(a1++);

	loc_d1d4:
	/* clr.b   ($C0401).l */
	set_scroll(d0/304); ///////////////
	copy_to_screen();
	post_render(fps);
	/* LOG(("d1d4\n")); */
	a4 += d0;
	d1--;
	if (d1 >= 0)
	{
		goto loc_d1d4;
	}

	variables[255] = 6;
	return 0;

	loc_d1f4:
	if (quit != 0)
	{
		/* hack, to stop animations */
		return 1;
	}

	a1 += 2;
	a2 = a1;
	d0 = get_word(a1);
	a1 += 2;
	if (d0 != 0)
	{
		a2 += d0;
		d3 = get_word(a1); /* colors mask used in this frame */
		d4 = get_word(a1+2); /* offset of something */
		a1 += 4;

		/* XXX: move into a local array */
		a0 = 0xdc000;
		unpack_animation_delta(a2, &memory[a0]);
		a2 = a0;
		a3 = a0 + d4;
		anim_interesting(a1, a2, a3, d3);
	}
	
	/* a4 = screen0 */
	if (d6 != 3)
	{
		goto loc_d268;
	}

	if (d7 == 7 || d7 == 9)
	{
		goto loc_d244;
	}

	if (d7 != 17)
	{
		goto loc_d248;
	}

	loc_d244:
	a4 -= 2;
	goto loc_d268;

	loc_d248:
	if (d7 == 12 || d7 == 13 || d7 == 15 || d7 == 20)
	{
		goto loc_d266;
	}

	if (d7 != 21)
	{
		goto loc_d268;
	}

	loc_d266:
	a4 += 2;

	loc_d268:
	d7++;
	if (byte_0_7FF96 == 0)
	{
		goto loc_d2ae;
	}
	// clr.b   ($C0401).l
	copy_to_screen();
	post_render(fps);
	/* LOG(("d268\n")); */
	// clr.b   ($C0401).l
	//move.b  #-1,($C0400).l
	// FIXME: WHY TWICE??
	copy_to_screen();
	post_render(fps);
	//move.b  #-1,($C0400).l
	d0 = 7;

	loc_d29c:
	copy_to_screen();
	post_render(fps);
	d0--;
	if (d0 >= 0)
	{
		goto loc_d29c;
	}

	byte_0_7FF96 = 0;
	goto loc_d160;

	loc_d2ae:
//seg000:0000D2AE                 clr.b   ($C0401).l
	copy_to_screen();
	post_render(fps);
	/* LOG(("d2ae\n")); */
//seg000:0000D2B8                 move.b  #-1,($C0400).l
	goto loc_d160;

	return 0;
}

void play_death_animation(int index)
{
	unsigned long offset;

	/* set palette 2 ? */
	offset = 0xf910 + (index << 2);
	play_sequence(get_long(offset), 15);
}

int play_animation(const char *filename, int fileoffset)
{
	int pattern;
	int pattern_offset;
	int total_patterns;
	int scene;
	int scene_offset;
	int palette_offset;
	int fps_;
	int read_offset;
	int stop;

	LOG(("playing animation %s\n", filename));

	read_offset = 0x809a - fileoffset;
	if (read_file(filename, memory + read_offset) < 0)
	{
		LOG(("play_animation: unable to read %s\n", filename));
		return -1;
	}

	pattern = 0;
	pattern_offset = get_long(0x809e);
	total_patterns = get_long(0x80a2);

	stop = 0;
	while (stop == 0)
	{
		fps_ = 10;

		scene = get_byte(pattern_offset + pattern);
		if (scene == 0x17)
		{
			/* special case for credits */
			scene = 3;
			fps_ = 1;
		}

		palette_offset = get_long(0x809a) + (scene << 5);
		set_palette_rgb12(&memory[palette_offset]);

		scene_offset = get_long(0x80a6 + (scene << 2));
		stop = play_sequence(scene_offset, fps_);

		pattern++;
		if (pattern == total_patterns)
		{
			/* no more scenes */
			break;
		}

	}

	quit = 0;
	LOG(("leaving animation player\n"));
	return stop;
}

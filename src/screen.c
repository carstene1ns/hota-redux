/*
 * Heart of The Alien: Virtual screens handling
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
#include <memory.h>

#include "vm.h"
#include "debug.h"
#include "render.h"
#include "screen.h"

/* instead of allocations. makes porting to consoles easier. */
static char huge_buf[304*192*3];

static char *screens[4];
static char *screen_visible;
static char *screen_invisible;

static int selected_screen;
static char *selected_screen_ptr;

static short scroll_reg = 0;

void fill_screen(int dest, char color)
{
	memset((void *)selected_screen_ptr, color, 304*192);
}

void copy_screen(int dest, int src)
{
	int masked_src;
	int masked_dest;
	char *src_surface;
	char *dest_surface;

	if (src == 0xc0)
	{
		set_variable(249, scroll_reg);
		/* XXX return ? */
	}

	masked_src = src;
	if (masked_src < 0xfe)
	{
		masked_src = masked_src & 0xbf;
	}

	masked_dest = dest;
	if (masked_dest < 0xfe)
	{
		masked_dest = masked_dest & 0xbf;
	}

	src_surface = get_screen_ptr(masked_src);
	dest_surface = get_screen_ptr(masked_dest);

	LOG(("copying surface %x onto %x\n", (unsigned)src_surface, (unsigned)dest_surface));
	memcpy(dest_surface, src_surface, 304*192);
}

char *get_screen_ptr(int which)
{
	if (which <= 3)
	{
		return screens[which];
	}

	if (which == 0xff)
	{
		return screen_invisible;
	}

	if (which == 0xfe)
	{
		return screen_visible;
	}

	/* else */
	return screens[0];
}

void select_screen(int which)
{
	selected_screen = which;
	selected_screen_ptr = get_screen_ptr(which);
}

void update_screen(int which)
{
	char *src;

	if (which == 0xfe)
	{
		src = screen_visible;
	}	            
	else if (which == 0xff)
	{
		char *d1, *d2;

		/* swap between visible and invisible */
		d1 = screen_visible;
		d2 = screen_invisible;
		screen_invisible = d1;
		screen_visible = d2;

		src = screen_visible;
	}
	else
	{
		screen_visible = get_screen_ptr(which);
		src = screen_visible;
	}

	render(src);
}

int get_selected_screen()
{
	return selected_screen;
}

char *get_selected_screen_ptr()
{
	return selected_screen_ptr;
}

int screen_init()
{
	/* screen 1 and 2 share same framebuffer! */
	screens[0] = (char *)huge_buf + 0;
	screens[1] = (char *)huge_buf + 304*192;
	screens[2] = screens[1];
	screens[3] = (char *)huge_buf + 304*192*2;

	screen_visible = get_screen_ptr(3);
	screen_invisible = get_screen_ptr(1);
	select_screen(0xfe);
	return 0;
}

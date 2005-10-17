/*
 * Heart of The Alien: Renderer
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
#include <string.h>
#include <SDL.h>
#include "debug.h"
#include "render.h"
#include "game2bin.h"

#include "scale2x.h"
#include "scale3x.h"

static int fullscreen = 0;
static int scroll_reg = 0;

extern int fullscreen_flag;
extern int filtered_flag;
extern int scale;
extern int palette_changed;
extern SDL_Color palette[256];
extern SDL_Surface *screen;

static int palette_changed = 0;
static int current_palette = 0;
static SDL_Color palette[256];

/** Returns the current palette used
    @returns palette

    Palettes are stored in game2.bin
*/
int get_current_palette()
{
	return current_palette;
}

/** Sets VSCROLL value
    @param scroll
*/
void set_scroll(int scroll)
{
	scroll_reg = scroll;
}

/** Returns VSCROLL register value
    @returns vscroll
*/
int get_scroll_register()
{
	return scroll_reg;
}

void render1x(char *src)
{
	int y, p;

	if (scroll_reg == 0)
	{
		/* no scroll */
		for (y=0; y<192; y++)
		{
			memcpy((char *)screen->pixels + y*screen->pitch, src + 304*y, 304);
		}
	}
	else if (scroll_reg < 0)
	{
		/* scroll from bottom */
		p = 1 - scroll_reg;
		for (y=p; y<192; y++)
		{
			memcpy((char *)screen->pixels + (y-p)*screen->pitch, src + 304*y, 304);
		}

		for (y=192; y<192+p; y++)
		{
			memcpy((char *)screen->pixels + (y-p)*screen->pitch, src + 304*(y-192), 304);
		}
	}
	else 
	{
		/* scroll from top */
		p = scroll_reg;
		for (y=0; y<p; y++)
		{
			memcpy((char *)screen->pixels + y*screen->pitch, src + 304*(191-p+y), 304);
		}

		for (y=p; y<192; y++)
		{
			memcpy((char *)screen->pixels + y*screen->pitch, src + 304*(y-p), 304);
		}
	}
}

/* advmame2x scaler */
void render2x_scaled(char *src)
{
	scale2x(screen, src, 304, 304, 192);
}

/* advmame3x scaler */
void render3x_scaled(char *src)
{
	scale3x(screen, src, 304, 304, 192);
}

/** Simple X2 scaler
    @param src
*/
void render2x(char *src)
{
	int x, y;
	unsigned char wide[304*2];
        
	for (y=0; y<192; y++)
	{
		unsigned char *srcp, *widep;

		srcp = src + 304*y;
		widep = wide;
		for (x=0; x<304; x++)
		{
			*widep++ = *srcp;
			*widep++ = *srcp++;
		}

		memcpy((char *)screen->pixels + y*2*screen->pitch, wide, 304*2);
		memcpy((char *)screen->pixels + (y*2+1)*screen->pitch, wide, 304*2);
	}
}

/** Simple X3 scaler
    @param src
*/
void render3x(char *src)
{
	int x, y;
	unsigned char wide[304*3];
        
	for (y=0; y<192; y++)
	{
		unsigned char *srcp, *widep;

		srcp = src + 304*y;
		widep = wide;
		for (x=0; x<304; x++)
		{
			*widep++ = *srcp;
			*widep++ = *srcp;
			*widep++ = *srcp++;
		}

		memcpy((char *)screen->pixels + y*3*screen->pitch, wide, 304*3);
		memcpy((char *)screen->pixels + (y*3+1)*screen->pitch, wide, 304*3);
		memcpy((char *)screen->pixels + (y*3+2)*screen->pitch, wide, 304*3);
	}
}

/** Renders a virtual screen
    @param src
*/
void render(unsigned char *src)
{
	SDL_LockSurface(screen);

	if (palette_changed)
	{
		palette_changed = 0;
		SDL_SetColors(screen, palette, 0, 256);
	}

	switch(scale)
	{
		case 1:
		/* normal 1x */
		render1x(src);
		break;

		case 2:
		if (filtered_flag == 0)
		{
			render2x(src);
		}
		else
		{
			render2x_scaled(src);
		}
		break;

		case 3:
		if (filtered_flag == 0)
		{
			render3x(src);
		}
		else
		{
			render3x_scaled(src);
		}
		break;
	}

	scroll_reg = 0;
	SDL_UnlockSurface(screen);
}

/** Module initializer
    @returns zero on success
*/
int render_init()
{
	return 0;
}

/** Converts a Sega CD RGB444 to RGB888 
    @param rgb12   pointer to 16x2 of palette data
*/
void set_palette_rgb12(unsigned char *rgb12)
{
	int i;

	for (i=0; i<16; i++)
	{
		int c, r, g, b;

		c = (rgb12[i*2] << 8) | rgb12[i*2+1];
		r = (c & 0xf) << 4;
		g = ((c >> 4) & 0xf) << 4;
		b = ((c >> 8) & 0xf) << 4;

		palette[i].r = r;
		palette[i].g = g;
		palette[i].b = b;
	}

	palette_changed = 1;
}

void set_palette(int which)
{
	unsigned char rgb12[16*2];

	copy_from_game2bin(rgb12, 0x5cb8 + (which * 16 * 2), sizeof(rgb12));

	current_palette = which;
	set_palette_rgb12(rgb12);

	palette[255].r = 255;
	palette[255].g = 0;
	palette[255].b = 255;
}

void toggle_fullscreen()
{
	/* hack, fullscreen not supported at scale==3 */
	if (scale == 3)
	{
		return;
	}

	fullscreen = 1 ^ fullscreen;
	SDL_FreeSurface(screen);
	screen = 0;

	if (fullscreen == 0)
	{
		LOG(("create SDL surface of 304x192x8\n"));

		screen = SDL_SetVideoMode(304*scale, 192*scale, 8, SDL_SWSURFACE);
		SDL_SetColors(screen, palette, 0, 256);
		SDL_ShowCursor(1);
	}
	else
	{
		int w, h;

		w = 320*scale;
		h = 200*scale;

		LOG(("setting fullscreen mode %dx%dx8\n", w, h));

		screen = SDL_SetVideoMode(w, h, 8, SDL_SWSURFACE|SDL_HWSURFACE|SDL_FULLSCREEN);

		SDL_SetColors(screen, palette, 0, 256);
		SDL_ShowCursor(0);
	}
}

int render_create_surface()
{
	screen = SDL_SetVideoMode(304*scale, 192*scale, 8, SDL_SWSURFACE);
	if (screen == NULL) 
	{
		return -1;
	}

	SDL_WM_SetCaption("Heart of The Alien Redux", 0);

	if (fullscreen_flag)
	{
		toggle_fullscreen();
	}

	return 0;
}

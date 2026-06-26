/*
 * Heart of The Alien: Renderer
 * Copyright (c) 2004 Gil Megidish
 * Copyright (c) 2016-2026 carstene1ns
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
#include <SDL3/SDL.h>

#include "debug.h"
#include "render.h"
#include "game2bin.h"
#include "client.h"

static int fullscreen = 0;
static int scroll_reg = 0;
extern int fullscreen_flag;

static SDL_Palette *palette;
static SDL_Color palette_colors[16];
static SDL_Surface *surface;
static SDL_Texture *texture;
static SDL_Window *window;
static SDL_Renderer *renderer;

static int palette_changed = 0;
static int current_palette = 0;

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

/** Renders a virtual screen
    @param src
*/
void render(unsigned char *src)
{
	int y, p;

	if (palette_changed)
	{
		palette_changed = 0;
		SDL_SetPaletteColors(palette, palette_colors, 0, 16);
	}

	if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);

	if (scroll_reg == 0)
	{
		/* no scroll */
		for (y=0; y<192; y++)
		{
			memcpy((char *)surface->pixels + y*surface->pitch, src + 304*y, 304);
		}
	}
	else if (scroll_reg < 0)
	{
		/* scroll from bottom */
		p = - scroll_reg;
		for (y=p; y<192; y++)
		{
			memcpy((char *)surface->pixels + (y-p)*surface->pitch, src + 304*y, 304);
		}

		for (y=192; y<192+p; y++)
		{
			memcpy((char *)surface->pixels + (y-p)*surface->pitch, src + 304*191, 304);
		}
	}
	else 
	{
		/* scroll from top */
		p = scroll_reg;
		for (y=0; y<p; y++)
		{
			memcpy((char *)surface->pixels + y*surface->pitch, src, 304);
		}

		for (y=p; y<192; y++)
		{
			memcpy((char *)surface->pixels + y*surface->pitch, src + 304*(y-p), 304);
		}
	}

	if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);

	scroll_reg = 0;

	SDL_RenderClear(renderer);
	SDL_UpdateTexture(texture, NULL, surface->pixels, surface->pitch);
	SDL_RenderTexture(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
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

		palette_colors[i].r = r | (r >> 4);
		palette_colors[i].g = g | (g >> 4);
		palette_colors[i].b = b | (b >> 4);
		palette_colors[i].a = 0xFF;
	}

	palette_changed = 1;
}

void set_palette(int which)
{
	unsigned char rgb12[16*2];

	copy_from_game2bin(rgb12, 0x5cb8 + (which * 16 * 2), sizeof(rgb12));

	current_palette = which;
	set_palette_rgb12(rgb12);

	//palette_colors[255].r = 255;
	//palette_colors[255].g = 0;
	//palette_colors[255].b = 255;
}

void toggle_fullscreen()
{
	fullscreen = 1 ^ fullscreen;

	SDL_SetWindowFullscreen(window, fullscreen);

	if (fullscreen == 0)
	{
		SDL_ShowCursor();
	}
	else
	{
		LOG(("setting fullscreen mode\n"));
		SDL_HideCursor();
	}
}

int render_create_surface()
{
	surface = SDL_CreateSurface(304, 192, SDL_PIXELFORMAT_INDEX8);
	if (surface == NULL)
	{
		return -1;
	}

	window = SDL_CreateWindow("Heart of The Alien Redux", 304 * cls.scale,
		192 * cls.scale, SDL_WINDOW_RESIZABLE);
	if (window == NULL)
	{
		return -2;
	}

	renderer = SDL_CreateRenderer(window, NULL);
	if (renderer == NULL)
	{
		return -3;
	}

	SDL_SetRenderLogicalPresentation(renderer, 304, 192,
		SDL_LOGICAL_PRESENTATION_LETTERBOX);

	texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_INDEX8,
		SDL_TEXTUREACCESS_STREAMING, 304, 192);
	if (texture == NULL)
	{
		return -4;
	}

	if (cls.filtered)
	{
		SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
	}
	else
	{
		SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_PIXELART);
	}

	palette = SDL_CreatePalette(16);
	if (palette == NULL)
	{
		return -5;
	}

	SDL_SetSurfacePalette(surface, palette);
	SDL_SetTexturePalette(texture, palette);

	if (fullscreen_flag)
	{
		toggle_fullscreen();
	}

	return 0;
}

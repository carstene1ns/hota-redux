/*
 * Heart of The Alien: Music player (cd and mp3)
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
#include <SDL.h>
#include <SDL_mixer.h>
#include "debug.h"
#include "music.h"
#include "cd_iso.h"
#include "client.h"

static SDL_CD *sdl_cd;
static Mix_Music *current_track;

/** (The Underdogs version of) Heart of The Alien's tracks are formatted like this */
#define ISO_PREFIX "Heart Of The Alien (U) "

/** Stops cd player */
static void stop_music_cd()
{
	SDL_CDStop(sdl_cd);
}

/** Stops mp3 player */
static void stop_music_mp3()
{
	if (current_track != NULL)
	{
		Mix_FreeMusic(current_track);
		current_track = NULL;
	}
}

/** Stops music */
void stop_music()
{
	if (cls.nosound == 0)
	{
		cls.use_iso ? stop_music_mp3() : stop_music_cd();
	}
}

/** Plays a single track from cd 
    @param track   track to play
    @param loop    loop count (not supported yet)
*/
static void play_music_track_cd(int track, int loop)
{
	LOG(("play_music_track_cd(track=%d, loop=%d, sdlcd=0x%x)\n", track, loop, (unsigned)sdl_cd));

	if (CD_INDRIVE(SDL_CDStatus(sdl_cd)))
	{
		int err = SDL_CDPlayTracks(sdl_cd, track, 0, 1, 0);
		LOG(("SDL_CDPlay returned %d\n", err));
	}
	else
	{
		LOG(("can't play cd audio, CD_INDRIVE failed\n"));
	}
}

/** Plays an mp3 in background 
    @param track    track to play (appended to PREFIX)
*/
static void play_music_track_mp3(int track, int loop)
{
	char filename[256];

	stop_music_mp3();

	sprintf(filename, ISO_PREFIX "%02d.mp3", track + 1);
	LOG(("playing mp3 %s\n", filename));

	current_track = Mix_LoadMUS(filename);
	Mix_PlayMusic(current_track, loop);
}

/** Plays audio track */
    @param track   track to play
    @param loop    loop count
*/
void play_music_track(int track, int loop)
{
	if (cls.nosound == 0)
	{
		stop_music();

		cls.use_iso ? play_music_track_mp3(track, loop) : play_music_track_cd(track, loop);
	}
}

/** Callback after a frame has been rendered */
void music_update()
{
}

/** Module initializer */
void music_init()
{
	if (cls.nosound == 0)
	{
		if (cls.use_iso == 0)
		{
			/* initialize SDL CDROM, for playing audio tracks */
			sdl_cd = SDL_CDOpen(0);
		}
	}
}

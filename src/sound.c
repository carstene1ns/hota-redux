/*
 * Heart of The Alien: SFX handling
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

#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "debug.h"
#include "sound.h"
#include "common.h"
#include "vm.h"

extern int nosound_flag;

/* cached samples, useless convertions and allocations hurt my eyes! */
static Mix_Chunk *cached_samples[256];

/* stop all channels from playing */
static void stop_all_channels()
{
	int p;

	for (p=0; p<4; p++)
	{
		Mix_HaltChannel(p);
	}
}

/* play a single sample, on a specific channel (out of 4) */
void play_sample(int index, int volume, int channel)
{
	int length, outlen, ptr;
	int p, sample_ptr;
	char *current_sample;
	SDL_AudioCVT cvt;
	Mix_Chunk *chunk;

	if (nosound_flag)
	{
		/* day off! */
		return;
	}

	if (index == 0)
	{
		/* stop all sounds */
		stop_all_channels();
		return;
	}

	index = index - 1;
	LOG(("playing sample %d, volume %d, channel %d\n", index, volume, channel));

	if (cached_samples[index] != NULL)
	{
		LOG(("play cached sample\n"));
		Mix_PlayChannel(channel, cached_samples[index], 0);
		return;
	}

	sample_ptr = get_long(0xf90c);
	ptr = get_long(sample_ptr + index * 4);
	length = get_long(ptr);

	/* length plus some unknown flags */
	ptr = ptr + 8;

	LOG(("sample data starts at 0x%x\n", ptr));

	/* convert from 8000 mono 8bit, to 44100 stereo 16bit */
	SDL_BuildAudioCVT(&cvt, AUDIO_S8, 1, 8000, AUDIO_S16, 2, 44100);
	outlen = length * cvt.len_mult;

	cvt.len = length;
	cvt.buf = (char *)malloc(outlen);
	if (cvt.buf == NULL)
	{
		fprintf(stderr, "failed to allocate %d bytes for sample\n", outlen);
		return;
	}

	current_sample = (char *)cvt.buf;
	for (p=0; p<length; p++)
	{
		unsigned char u;
		signed char s;

		u = get_byte(ptr++);
		if (u > 0x80)
		{
			s = 0 - (u & 0x7f);
		}
		else if (u < 0x80)
		{
			s = u;
		}
		else
		{
			s = 0x00;
		}
	
		*current_sample++ = s;
	}

	SDL_ConvertAudio(&cvt);

	LOG(("sample length %d\n", length));
	
	chunk = (Mix_Chunk *)malloc(sizeof(Mix_Chunk));
	chunk->allocated = 1;
	chunk->abuf = cvt.buf;
	chunk->alen = outlen;
	chunk->volume = volume >> 1;

	cached_samples[index] = chunk;
	Mix_PlayChannel(channel, chunk, 0);
}

/* module initializer */
void sound_init()
{
	memset((void *)cached_samples, '\0', sizeof(cached_samples));
}

/* free all memory allocated for cached sound effects */
void sound_flush_cache()
{
	int i, elem;

	elem = sizeof(cached_samples) / sizeof(cached_samples[0]);
	for (i=0; i<elem; i++)
	{
		if (cached_samples[i] != NULL)
		{
			Mix_FreeChunk(cached_samples[i]);
			cached_samples[i] = NULL;
		}
	}
}

/* callback when game script has been unloaded */
void sound_done()
{
	sound_flush_cache();
}

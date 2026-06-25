/*
 * Heart of The Alien: SFX handling
 * Copyright (c) 2004-2005 Gil Megidish
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
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "audio.h"
#include "common.h"
#include "vm.h"
#include "client.h"
#include "cd_iso.h"

static MIX_Mixer *mixer;
static MIX_Track *music;
static MIX_Track *sounds[4];

/** (The Underdogs version of) Heart of The Alien's tracks are formatted like this */
#define ISO_PREFIX "Heart Of The Alien (U) "

/** Stops all 4 channels
*/
static void stop_all_channels()
{
	for(int i = 0; i < 4; i++)
		MIX_StopTrack(sounds[i], 0);
}

/** Start playing a sample
    @param index     sample identifier
    @param volume    volume [0 .. 0xff]
    @param channel   mixing channel [0 .. 3]

    Samples on sega-cd are 8000 hz, 8bit signed. each sample is
    converted to native format before being mixed.
*/
void play_sample(int index, int volume, int channel)
{
	static SDL_AudioSpec spec = {
		.format = SDL_AUDIO_S8,
		.channels = 1,
		.freq = 8000
	};

	if (cls.nosound)
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

	int sample_ptr = get_long(0xf90c);
	int ptr = get_long(sample_ptr + index * 4);
	int length = get_long(ptr);

	/* length plus some unknown flags */
	ptr = ptr + 8;
	LOG(("sample data starts at 0x%x, length is %d\n", ptr, length));

	/* convert from 8000 mono 8bit, to 44100 stereo 16bit */
	char *buffer = SDL_malloc(length);
	if (buffer == NULL)
	{
		fprintf(stderr, "failed to allocate %d bytes for sample\n", length);
		return;
	}

	char *current_sample = buffer;
	for (int p=0; p<length; p++)
	{
		unsigned char u;
		signed char s;

		u = get_byte(ptr++);
		if (u > 0x80)
		{
			s = 0 - (u & 0x7f);
		}
		else if (u <= 0x80)
		{
			s = u;
		}

		*current_sample++ = s;
	}

	MIX_Audio *a = MIX_LoadRawAudioNoCopy(mixer, buffer, length, &spec, true);
	MIX_SetTrackAudio(sounds[channel], a);
	MIX_SetTrackGain(sounds[channel], volume/255.0f);
	MIX_PlayTrack(sounds[channel], 0);
	MIX_DestroyAudio(a);
}

/** Initialize audio module
    @returns zero on success, negative value on error
*/
int audio_init()
{
	MIX_Init();
	mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);

	music = MIX_CreateTrack(mixer);
	for(int i = 0; i < 4; i++)
		sounds[i] = MIX_CreateTrack(mixer);

	return 0;
}

/** Purges all cached sound effects from memory

    Releases memory taken by sfx previously played by play_sample()
*/
void sound_flush_cache()
{
}

/** callback when game script has been unloaded */
void audio_done()
{
	sound_flush_cache();
	for(int i = 0; i < 4; i++)
		MIX_DestroyTrack(sounds[i]);
	stop_music();
	MIX_DestroyTrack(music);
	MIX_DestroyMixer(mixer);

	MIX_Quit();
}

/** Stops music */
void stop_music()
{
	if (cls.nosound == 0)
	{
		if (music != NULL)
		{
			MIX_StopTrack(music, 100);
		}
	}
}

/** Plays audio track
    @param track   track to play
    @param loop    loop count
*/
void play_music_track(int track, int loop)
{
	if (cls.nosound == 0)
	{
		char filename[256];

		stop_music();

		sprintf(filename, ISO_PREFIX "%02d.mp3", track + 1);
		LOG(("playing mp3 %s\n", filename));

		MIX_Audio *a = MIX_LoadAudio(mixer, filename, false);
		MIX_SetTrackAudio(music, a);
		MIX_PlayTrack(music, 0);
		MIX_SetTrackLoops(music, -1);
		MIX_DestroyAudio(a);
	}
}

/** Callback after a frame has been rendered */
void music_update()
{
}

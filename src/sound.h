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
#ifndef __SOUND_INCLUDED__
#define __SOUND_INCLUDED__

/** Initialize sound module
    @returns zero on success, negative value on error
*/
int sound_init();

/** callback when game script has been unloaded */
void sound_done();

/** Purges all cached sound effects from memory

    Releases memory taken by sfx previously played by play_sample()
*/
void sound_flush_cache();

/** Start playing a sample
    @param index     sample identifier
    @param volume    volume [0 .. 0xff]
    @param channel   mixing channel [0 .. 3]

    Samples on sega-cd are 8000 hz, 8bit signed. each sample is
    converted to native format before being mixed. since it is quite
    an overkill to convert every time, converted samples are stored
    in cache until purged manually.
*/
void play_sample(int index, int volume, int channel);

#endif

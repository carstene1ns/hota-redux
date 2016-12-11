/*
 * Heart of The Alien Redux: Cutscene and deathscene animation player
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
#ifndef __ANIMATION_INCLUDED__
#define __ANIMATION_INCLUDED__

/** Plays a death sequence
    @param index    animation index in resources

    Death animations are stored as resources in the room file itself,
    along with the rest of the level
*/
int play_death_animation(int index);

/** Plays an animation file
    @param filename    name as appears on cd
    @param fileoffset  offset in bytes where to start reading from
    @returns zero if played completely, 1 if aborted, negative on error
*/
int play_animation(const char *filename, int fileoffset);

#endif

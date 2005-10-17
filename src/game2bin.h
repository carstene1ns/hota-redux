/*
 * Heart of The Alien: GAME2BIN file loader
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
#ifndef __GAME2BIN_INCLUDED__
#define __GAME2BIN_INCLUDED__

/** loads GAME2.BIN file into memory
    @returns 0 on success, negative value on error
*/
int game2bin_init();

/** copies a chunk from game2bin
    @param dst     address to destination
    @param offset  offset from game2bin
    @param length  length in bytes
    @returns bytes copied (length on success)
*/
int  copy_from_game2bin(void *dst, int offset, int length);

#endif

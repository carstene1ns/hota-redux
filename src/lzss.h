/*
 * Heart of The Alien: lzss decompressor
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
#ifndef __LZSS_INCLUDED__
#define __LZSS_INCLUDED__

/** Decompresses lzss packed byte buffer
    @param out      output buffer (must be of size 304*192/2 bytes)

    Executable offset: 0xcf5c
    There are 16 common distances

    @returns 0 on success
*/
int unlzss(unsigned char *out, int a2, int a3);

#endif

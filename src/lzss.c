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
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "vm.h"

/** Decompresses lzss packed byte buffer
    @param out      output buffer (must be of size 304*192/2 bytes)

    Executable offset: 0xcf5c
    There are 16 common distances

    @returns 0 on success
*/
int unlzss(unsigned char *out, int ptr1, int ptr2)
{
	int packed;
	int bitmask, bits_left, d2, d3, d4, d6;
	int common_distances;
	unsigned char *copy_from;

	/* 
	bits_left bits left (and bit checked)
	bitmask 32bits 
	d3 negative offset against output
	*/

	d4 = get_word(ptr2);
	packed = ptr2 + 20;
	common_distances = ptr2 + 3;
	*out++ = get_byte(packed++);

	loc_cf6e:
	bitmask = get_long(ptr1);
	ptr1 = ptr1 + 4;
	bits_left = 31;

	goto loc_cf7a;

	loc_cf76:
	bits_left--;
	if (bits_left < 0)
	{
		goto loc_cf6e;
	}

	loc_cf7a:
	if (bitmask & (1 << bits_left))
	{
		/* escaped literal */
		*out++ = get_byte(packed++);
		goto loc_cf76;
	}

	bits_left--; 
	if (bits_left < 0)
	{
		bitmask = get_long(ptr1);
		ptr1 = ptr1 + 4;
		bits_left = 31;
		goto loc_cf82;
	}

	loc_cf82:
	if (bitmask & (1 << bits_left))
	{
		goto loc_cfa4;
	}

	bits_left--;
	if (bits_left < 0)
	{
		bitmask = get_long(ptr1);
		ptr1 = ptr1 + 4;
		bits_left = 31;
	}

	d3 = get_byte(packed++);
	if ((bitmask & (1 << bits_left)))
	{
		d3 = d3 | 0x100;
	}

	goto loc_cfe6;

	loc_cfa4:
	bits_left--;
	if (bits_left < 0)
	{
		bitmask = get_long(ptr1);
		ptr1 = ptr1 + 4;
		bits_left = 31;
	}

	d2 = 0;
	d6 = 1;

	loc_cfac:
	if (bitmask & (1 << bits_left))
	{
		goto loc_cfd2;
	}

	bits_left--;
	if (bits_left < 0)
	{
		bitmask = get_long(ptr1);
		ptr1 = ptr1 + 4;
		bits_left = 31;
		goto loc_cfb4;
	}

	loc_cfb4:
	if (bitmask & (1 << bits_left))
	{
		d2 = d2 + d6;
	}

	d6 = d6 << 1;
	bits_left--;
	if (bits_left < 0)
	{
		bitmask = get_long(ptr1);
		ptr1 = ptr1 + 4;
		bits_left = 31;
	}

	goto loc_cfac;

	loc_cfd2:
	d2 = d2 + d6;
	if (d2 == 16)
	{
		/* break loop! */
		goto loc_d026;
	}

	d3 = get_byte(common_distances + d2);
	if (d4 & (1 << d2))
	{
		d3 = d3 | 0x100;
	}

	loc_cfe6:
	bits_left--;
	if (bits_left < 0)
	{
		bitmask = get_long(ptr1);
		ptr1 = ptr1 + 4;
		bits_left = 31;
	}

	d2 = 0;
	d6 = 1;

	loc_cfee:
	if (bitmask & (1 << bits_left))
	{
		/* direct copy */
		int count = d2 + d6 + 2;
		copy_from = out - d3;

		while (count--)
		{
			*out++ = *copy_from++;
		}

		goto loc_cf76;
	}

	bits_left--;
	if (bits_left < 0)
	{
		bitmask = get_long(ptr1);
		ptr1 = ptr1 + 4;
		bits_left = 31;
	}

	if ((bitmask & (1 << bits_left)))
	{
		d2 = d2 + d6;
	}

	d6 = d6 << 1;

	bits_left--;
	if (bits_left < 0)
	{
		bitmask = get_long(ptr1);
		ptr1 = ptr1 + 4;
		bits_left = 31;
	}

	goto loc_cfee;

	loc_d026:
	return 0;
}

/*
 * Heart of The Alien: LZSS decompressor
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
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "vm.h"

int unlzss(unsigned char *out, int a2, int a3)
{
	int a1;
	int d0, d1, d2, d3, d4, d6;
	int common_distances;
	unsigned char *copy_from;

	/* 
	d1 bits left (and bit checked)
	d0 32bits 
	d3 negative offset against output
	*/

	// cf5c
	a1 = a3 + 20;
	d4 = get_word(a3);
	a3 = a3 + 3;
	common_distances = a3;
	*out++ = get_byte(a1++);

	// cf6e
	loc_cf6e:
	d0 = get_long(a2);
	a2 = a2 + 4;
	d1 = 0x1f;
	goto loc_cf7a;

	loc_cf74:
	/* escaped literal */
	*out++ = get_byte(a1++);

	loc_cf76:
	d1--;
	if (d1 < 0)
	{
		goto loc_cf6e;
	}

	loc_cf7a:
	if (d0 & (1 << d1))
	{
		goto loc_cf74;
	}

	d1--; 
	if (d1 < 0)
	{
		goto loc_cf98;
	}

	loc_cf82:
	if (d0 & (1 << d1))
	{
		goto loc_cfa4;
	}

	d1--;
	if (d1 < 0)
	{
		goto loc_cf9e;
	}

	loc_cf8a:
	d3 = get_byte(a1);
	a1++;
	if ((d0 & (1 << d1)) == 0)
	{
		goto loc_cfe6;
	}

	d3 = d3 | 0x100;
	goto loc_cfe6;

	loc_cf98:
	d0 = get_long(a2);
	a2 = a2 + 4;
	d1 = 0x1f;
	goto loc_cf82;

	loc_cf9e:
	d0 = get_long(a2);
	a2 = a2 + 4;
	d1 = 0x1f;
	goto loc_cf8a;

	loc_cfa4:
	d1--;
	if (d1 < 0)
	{
		goto loc_cfcc;
	}

	loc_cfa8:
	d2 = 0;
	d6 = 1;

	loc_cfac:
	if (d0 & (1 << d1))
	{
		goto loc_cfd2;
	}

	d1--;
	if (d1 < 0)
	{
		goto loc_cfc6;
	}

	loc_cfb4:
	if ((d0 & (1 << d1)) == 0)
	{
		goto loc_cfba;
	}
	
	d2 = d2 + d6;

	loc_cfba:
	d6 = d6 << 1;
	d1--;
	if (d1 >= 0)
	{
		goto loc_cfac;
	}
	
	d0 = get_long(a2);
	a2 = a2 + 4;
	d1 = 0x1f;
	goto loc_cfac;

	loc_cfc6:
	d0 = get_long(a2);
	a2 = a2 + 4;
	d1 = 0x1f;
	goto loc_cfb4;

	loc_cfcc:
	d0 = get_long(a2);
	a2 = a2 + 4;
	d1 = 0x1f;
	goto loc_cfa8;

	loc_cfd2:
	d2 = d2 + d6;
	if (d2 == 16)
	{
		/* break loop! */
		goto loc_d026;
	}

	//printf("d2 = %d, a3 = 0x%x\n", d2, a3);
	d3 = get_byte(common_distances + d2);
	if (d4 & (1 << d2))
	{
		d3 = d3 | 0x100;
	}

	loc_cfe6:
	d1--;
	if (d1 < 0)
	{
		goto loc_d00e;
	}

	loc_cfea:
	d2 = 0;
	d6 = 1;

	loc_cfee:
	if (d0 & (1 << d1))
	{
		goto loc_d014;
	}

	d1 = d1 - 1;
	if (d1 < 0)
	{
		goto loc_d008;	
	}

	loc_cff6:
	if ((d0 & (1 << d1)))
	{
		d2 = d2 + d6;
	}

	d6 = d6 << 1;
	d1--;
	if (d1 >= 0)
	{
		goto loc_cfee;
	}

	d0 = get_long(a2);
	a2 = a2 + 4;
	d1 = 0x1f;
	goto loc_cfee;

	loc_d008:
	d0 = get_long(a2);
	a2 = a2 + 4;
	d1 = 0x1f;
	goto loc_cff6;

	loc_d00e:
	d0 = get_long(a2);
	a2 = a2 + 4;
	d1 = 0x1f;
	goto loc_cfea;

	loc_d014:
	d2 = d2 + d6 + 1;
	copy_from = out - d3;

	while (d2 >= 0)
	{
		*out++ = *copy_from++;
		d2--;
	}

	goto loc_cf76;

	loc_d026:
	return 0;
}

/*
 * Heart of The Alien: Backdrop decoder
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
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "vm.h"
#include "lzss.h"

int unpack_room(void *out, int entry)
{
	int d0, a2, a3;
	int resources;

	resources = get_long(0xf908);

	if (entry >= get_word(resources))
	{
		return -1;
	}

	d0 = get_long(resources + 2 + (entry << 2));
	a2 = get_long(d0);
	a3 = get_long(d0 + 4);

	return unlzss(out, a2, a3);
}


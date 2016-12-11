/*
 * Heart of The Alien: VM debugging facilities
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
#include "debug.h"

static int marked_opcodes[256] = {0};

void print_hex(unsigned char *ptr, int length)
{
	while (length > 0)
	{
		LOG(("%02x ", *ptr++));
		length--;
	}

	LOG(("\n"));
}

void mark_opcode(int opcode)
{
	int i;

	if (marked_opcodes[opcode] == 0)
	{
		marked_opcodes[opcode] = 1;

		LOG(("opcodes used: "));
		for (i=0; i<256; i++)
		{
			if (marked_opcodes[i])
			{
				LOG(("%02x ", i));
			}
		}

		LOG(("\n"));
	}
}

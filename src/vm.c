/*
 * Heart of The Alien: Virtual machine primitives (memory and variables)
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

#include <memory.h>
#include "vm.h"

/* FIXME: need more memory just for temp animation buffer */
#define MAX_VARIABLES 256

short variables[MAX_VARIABLES];
unsigned char memory[512*1024*2]; 

int auxptr;
static int using_aux = 0;
short auxvars[MAX_TASKS*32];

void set_aux_bank(int bank)
{
	auxptr = (bank * 32);
}

unsigned char get_byte(int offset)
{
	return memory[offset];
}

unsigned short get_word(int offset)
{
	return (get_byte(offset) << 8) | get_byte(offset + 1);
}

unsigned long get_long(int offset)
{
	return (get_word(offset) << 16) | get_word(offset + 2);
}

short get_variable(int var)
{
	if (using_aux)
	{
		return auxvars[auxptr + var];
	}

	return variables[var];
}

void set_variable(int var, short value)
{
	if (using_aux)
	{
		auxvars[auxptr + var] = value;
		return;
	}

	variables[var] = value;
}

int toggle_aux(int toggle)
{
	int old = using_aux;
	using_aux = toggle;
	return old;
}

void vm_reset()
{
	auxptr = 0;
	using_aux = 0;
	memset(variables, '\0', sizeof(variables));
	memset(auxvars, '\0', sizeof(auxvars));
}

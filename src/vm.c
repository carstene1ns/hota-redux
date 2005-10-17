/*
 * Heart of The Alien: Virtual machine primitives (memory and variables)
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

#include <memory.h>
#include "vm.h"

#define MAX_VARIABLES 256

short variables[MAX_VARIABLES];
static unsigned char memory[512*1024*2]; 

int auxptr;
static int using_aux = 0;
short auxvars[MAX_TASKS*32];

void set_aux_bank(int bank)
{
	auxptr = (bank * 32);
}

/** Loads byte from memory offset
    @param offset    address (512K)
    @returns value
*/
unsigned char get_byte(int offset)
{
	return memory[offset];
}

/** Loads word from memory offset
    @param offset    address (512K)
    @return value

    No alignment restrictions
*/
unsigned short get_word(int offset)
{
	return (get_byte(offset) << 8) | get_byte(offset + 1);
}

/** Loads long from memory offset
    @param offset    address (512K)
    @return value

    No alignment restrictions
*/
unsigned long get_long(int offset)
{
	return (get_word(offset) << 16) | get_word(offset + 2);
}

/** Gets variable either from global memory, or from auxiliry memory
    @param var   index in memory

    Note that each variable is 16 bit. in global memory there are 256
    variables, while in auxiliry memory, there are only 32. use 
    toggle_aux() to select between these two memory sets
*/
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

/** Copies variables from aux storage to global set
    @param dst_index first element in aux storage
    @param src_index first element in global storage
    @param count     number of elements to copy
*/
void copy_tls_to_global(int dst_index, int src_index, int count)
{
	/* copy variables from variables to active-var-vector */
	while (count >= 0)
	{
		variables[dst_index++] = get_variable(src_index++);
		count--;
	}
}

/** Copies variables from global set, to aux storage
    @param dst_index first element in aux storage
    @param src_index first element in global storage
    @param count     number of elements to copy
*/
void copy_global_to_tls(int dst_index, int src_index, int count)
{
	while (count >= 0)
	{
		set_variable(dst_index++, variables[src_index++]);
		count--;
	}
}

/** Toggles use of auxiliry memory for getters and setters
    @param toggle  non-zero to use aux memory
    @returns old toggle value
*/
int toggle_aux(int toggle)
{
	int old = using_aux;
	using_aux = toggle;
	return old;
}

/** Returns a pointer to the specific offset in virtual memory
    @param offset  position in bytes from begining of memory
    @returns pointer to that region
*/
unsigned char *get_memory_ptr(int offset)
{
	return (unsigned char *)memory + offset;
}

/** Resets VM, zeros all variables and aux memory
*/
void vm_reset()
{
	auxptr = 0;
	using_aux = 0;
	memset(variables, '\0', sizeof(variables));
	memset(auxvars, '\0', sizeof(auxvars));
}

/*
 * Heart of The Alien: Common 68000 opcodes
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
#include "debug.h"
#include "common.h"

int extn(unsigned char n)
{
	if (n & 0x08)
	{
		int x = -16;
		return (x | n);
	}
	else
	{
		return n;
	}
}

int extw(unsigned int b)
{
	assert(b < 0x100);
	if (b & 0x80)
	{
		long v = -256;
		return (v | b);
	}

	return b;
}

int extl(unsigned short w)
{
	if (w & 0x8000)
	{
		long v = -65536;
		return (v | w);
	}

	return w;
}

void panic(const char *string)
{
	printf("panic: %s\n", string);
	exit(0);
}

void fputw(unsigned short s, FILE *fp)
{
	fputc(s >> 8, fp);
	fputc(s & 0xff, fp);
}

unsigned short fgetw(FILE *fp)
{
	unsigned short s;

	s = fgetc(fp);
	s = (s << 8) | fgetc(fp);
	return s;
}


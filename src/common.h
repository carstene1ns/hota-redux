/*
 * Heart of The Alien: Common/68000 opcodes
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
#ifndef COMMON_H
#define COMMON_H

#define BIT(x) (1 << x)

// 68000 opcodes

int extn(unsigned char n);
int extw(unsigned int b);
int extl(unsigned short w);

void panic(const char *string);

unsigned short fgetw(FILE *fp);
void fputw(unsigned short s, FILE *fp);

#endif // COMMON_H

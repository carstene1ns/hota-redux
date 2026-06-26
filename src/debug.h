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
#ifndef DEBUG_H
#define DEBUG_H

#include <stdio.h>

enum DEBUG_FLAGS {
	DEBUG_DEFAULT = 1<<0,
	DEBUG_SCRIPT = 1<<1,
	DEBUG_TASK = 1<<2,
	DEBUG_SPRITE = 1<<3,
	DEBUG_SCREEN = 1<<4,
	DEBUG_AUDIO = 1<<5,
	DEBUG_MAIN = 1<<6,
	DEBUG_ANIM = 1<<7,
	DEBUG_FILE = 1<<8
};

extern int debug_flag;

#ifdef ENABLE_DEBUG
#define WARN(x, ...) printf("WARN: " x, ## __VA_ARGS__)
#define LOG(f, x, ...) if (debug_flag & f) { printf (x, ## __VA_ARGS__); }
#define LOG_D(x, ...) LOG(DEBUG_DEFAULT, x, ## __VA_ARGS__)
#define LOG_SCRIPT(x, ...) LOG(DEBUG_SCRIPT, x, ## __VA_ARGS__)
#define LOG_TASK(x, ...) LOG(DEBUG_TASK, x, ## __VA_ARGS__)
#define LOG_SPRITE(x, ...) LOG(DEBUG_SPRITE, x, ## __VA_ARGS__)
#define LOG_SCREEN(x, ...) LOG(DEBUG_SCREEN, x, ## __VA_ARGS__)
#define LOG_AUDIO(x, ...) LOG(DEBUG_AUDIO, x, ## __VA_ARGS__)
#define LOG_MAIN(x, ...) LOG(DEBUG_MAIN, x, ## __VA_ARGS__)
#define LOG_ANIM(x, ...) LOG(DEBUG_ANIM, x, ## __VA_ARGS__)
#define LOG_FILE(x, ...) LOG(DEBUG_FILE, x, ## __VA_ARGS__)
#else
#define WARN(x, ...)
#define LOG_D(x, ...)
#define LOG_SCRIPT(x, ...)
#define LOG_TASK(x, ...)
#define LOG_SPRITE(x, ...)
#define LOG_SCREEN(x, ...)
#define LOG_AUDIO(x, ...)
#define LOG_MAIN(x, ...)
#define LOG_ANIM(x, ...)
#define LOG_FILE(x, ...)
#endif

#define ERROR(x, ...) printf("ERROR: " x, ## __VA_ARGS__)

void mark_opcode(int opcode);
void print_hex(unsigned char *ptr, int length);

#endif // DEBUG_H

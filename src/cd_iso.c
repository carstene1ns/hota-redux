/*
 * Heart of The Alien: ISO handling
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
#include <string.h>
#include "debug.h"
#include "cd_iso.h"

static char *ISO_FILENAME = "Heart Of The Alien (U).iso";
static int use_iso = 0;

typedef struct file_offset_t 
{
	char *filename;
	int offset;
	int size;
} file_offset_t;

static file_offset_t file_offsets[] = 
{
	{"END1.BIN", 0x510800, 0x69800},
	{"END2.BIN", 0x57a000, 0x69800},
	{"END3.BIN", 0x5e3800, 0x69800},
	{"END4.BIN", 0x64d000, 0x69800},
	{"GAME2.BIN", 0x21d000, 0x64000},
	{"INTRO1.BIN", 0x77000, 0x69800},
	{"INTRO2.BIN", 0xe0800, 0x69800},
	{"INTRO3.BIN", 0x14a000, 0x69800},
	{"INTRO4.BIN", 0x1b3800, 0x69800},
	{"MAKE2MB.BIN", 0x721000, 0x6a800},     /* contains animation! */
	{"MID2.BIN", 0x78b800, 0x69800},
	{"ROOMS1.BIN", 0x8ca800, 0x5a800},
	{"ROOMS2.BIN", 0x281000, 0x5a800},
	{"ROOMS3.BIN", 0x2db800, 0x5a800},
	{"ROOMS4.BIN", 0x336000, 0x5a800},
	{"ROOMS5.BIN", 0x390800, 0x5a800},
	{"ROOMS6.BIN", 0x815800, 0x5a800},
	{"ROOMS7.BIN", 0x3eb000, 0x2743a},
	{"ROOMS8.BIN", 0x870000, 0x5a800}
};

int toggle_use_iso(int toggle)
{
	int old_toggle = use_iso;
	use_iso = toggle;
	return old_toggle;
}

int get_iso_toggle()
{
	return use_iso;
}

static int get_file_offset(const char *filename, int *offset, int *size)
{
	int p, elements;

	elements = sizeof(file_offsets) / sizeof(file_offsets[0]);
	for (p=0; p<elements; p++)
	{
		if (strcmp(file_offsets[p].filename, filename) == 0)
		{
			/* found file */
			*offset = file_offsets[p].offset;
			*size = file_offsets[p].size;
			return 0;
		}
	}

	return -1;
}

static int read_file_internal(const char *filename, int size, int offset, void *out)
{
	FILE *fp;

	fp = fopen(filename, "rb");
	if (fp == NULL)
	{
		LOG(("read_file: error opening file %s\n", filename));
		return -1;
	}

	fseek(fp, offset, SEEK_SET);
	if (fread(out, size, 1, fp) != 1)
	{
		fclose(fp);
		LOG(("read_file: error reading file %d bytes from offset 0x%x\n", size, offset));
		return -1;
	}

	fclose(fp);
	return 0;
}

int read_file(const char *filename, void *out)
{
	int size, offset;
	char archive[256];
	
	if (get_file_offset(filename, &offset, &size) < 0)
	{
		/* not found */
		LOG(("read_file: %s not found\n", filename));
		return -1;
	}

	if (use_iso)
	{
		strcpy(archive, ISO_FILENAME);
	}
	else
	{
		const char *cdname = SDL_CDName(0);

		if (cdname != NULL)
		{
			strcpy(archive, cdname);
			strcat(archive, filename);
		}
		else
		{
			strcpy(archive, filename);
		}

		offset = 0;
	}

	return read_file_internal(archive, size, offset, out);
}

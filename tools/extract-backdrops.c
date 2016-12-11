/*
 * Heart of The Alien: Extract all backdrops (304x192x4bpp)
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

#include "../src/rooms.h"

unsigned char memory[512*1024];

int main(int argc, char **argv)
{
	int i, p, entries;
	char filename[64];
	char buffer[304*192/2];
	FILE *fp;

	memset(memory, '\0', sizeof(memory));

	for (i=1; i<=8; i++)
	{
		sprintf(filename, "rooms%d.bin", i);
		fp = fopen(filename, "rb");
		fread(memory + 0xf900, 370688, 1, fp);
		fclose(fp);

		printf("processing %s... ", filename);

		p = 0;
		while (1)
		{
			if (unpack_room(buffer, p) < 0)
			{
				break;
			}

			sprintf(filename, "%d-%02d", i, p);
			fp = fopen(filename, "wb");
			fwrite(buffer, 304*192/2, 1, fp);
			fclose(fp);

			p++;
		}

		printf("%d entries\n", p);
	}

	return 0;
}

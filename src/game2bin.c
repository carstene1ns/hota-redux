#include <memory.h>
#include "debug.h"
#include "cd_iso.h"
#include "game2bin.h"

static char game2bin[409600];

void copy_from_game2bin(void *dst, int offset, int length)
{
	memcpy(dst, game2bin + offset, length);
}

int game2bin_init()
{
	if (read_file("GAME2.BIN", game2bin) < 0)
	{
		LOG(("failure reading game2bin file\n"));
		return -1;
	}
	
	LOG(("success reading game2bin file\n"));
	return 0;
}

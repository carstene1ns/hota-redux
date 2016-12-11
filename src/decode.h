#ifndef __DECODE_INCLUDED__
#define __DECODE_INCLUDED__

#define INVALID_PC        -1

unsigned char next_pc();
unsigned short next_pc_word();

int decode(int current_task, int start_pc);

#endif

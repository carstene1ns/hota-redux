#ifndef __VM_INCLUDED__
#define __VM_INCLUDED__

#define MAX_TASKS 64

short get_variable(int var);
void set_variable(int var, short value);

void set_aux_bank(int bank);
int toggle_aux(int toggle);

unsigned char get_byte(int offset);
unsigned short get_word(int offset);
unsigned long get_long(int offset);

void vm_reset();

#endif

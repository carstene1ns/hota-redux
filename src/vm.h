#ifndef __VM_INCLUDED__
#define __VM_INCLUDED__

#define MAX_TASKS 64

/** Gets variable either from global memory, or from auxiliry memory
    @param var   index in memory

    Note that each variable is 16 bit. in global memory there are 256
    variables, while in auxiliry memory, there are only 32. use 
    toggle_aux() to select between these two memory sets
*/
short get_variable(int var);

void set_variable(int var, short value);

void set_aux_bank(int bank);

/** Toggles use of auxiliry memory for getters and setters
    @param toggle  non-zero to use aux memory
    @returns old toggle value
*/
int toggle_aux(int toggle);

/** Loads byte from memory offset
    @param offset    address (512K)
    @returns value
*/
unsigned char get_byte(int offset);

/** Loads word from memory offset
    @param offset    address (512K)
    @return value

    No alignment restrictions
*/
unsigned short get_word(int offset);

/** Loads long from memory offset
    @param offset    address (512K)
    @return value

    No alignment restrictions
*/
unsigned long get_long(int offset);

/** Copies variables from global set, to aux storage
    @param dst_index first element in aux storage
    @param src_index first element in global storage
    @param count     number of elements to copy
*/
void copy_global_to_tls(int dst_index, int src_index, int count);

/** Copies variables from aux storage to global set
    @param dst_index first element in aux storage
    @param src_index first element in global storage
    @param count     number of elements to copy
*/
void copy_tls_to_global(int dst_index, int src_index, int count);

unsigned char *get_memory_ptr(int offset);

void vm_reset();

#endif

#ifndef __SPRITES_INCLUDED__
#define __SPRITES_INCLUDED__

#define FLAG_HIDDEN 0x80

#define MAX_SPRITES 64

typedef struct sprite_t
{
	unsigned char index;
	unsigned char u1;
	unsigned char next;
	unsigned char frame;
	short x;
	short y;
	unsigned char u2;
	unsigned char u3;
	short w4;
	short w5;
	unsigned char u6;
	unsigned char u7;
} sprite_t;

/////////
extern sprite_t sprites[];
extern const char *sprite_data_byte_str[16];
/////////

void print_sprite(int p);
short get_sprite_data_word(int entry, int i);
void set_sprite_data_word(int entry, int i, short value);
unsigned char get_sprite_data_byte(int entry, int i);
void set_sprite_data_byte(int entry, int i, unsigned char value);
void reset_sprite_list();
void move_sprite_by(int sprite, int dx, int dy);
void flip_sprite(int sprite);
void mirror_sprite(int sprite);
void unmirror_sprite(int sprite);
void remove_sprite(int var);

#endif

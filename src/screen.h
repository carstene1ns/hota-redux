#ifndef __SCREEN_INCLUDED__
#define __SCREEN_INCLUDED__

int screen_init();

int get_selected_screen();
char *get_selected_screen_ptr();

void select_screen(int which);
void update_screen(int which);
char *get_screen_ptr(int which);

void copy_screen(int dest, int src);
void fill_screen(int dest, char color);

#endif


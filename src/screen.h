#ifndef __SCREEN_INCLUDED__
#define __SCREEN_INCLUDED__

int screen_init();

int get_selected_screen();
char *get_selected_screen_ptr();

void select_screen(int which);
void update_screen(int which);
char *get_screen_ptr(int which);

void copy_screen(int dest, int src);

/** Fills the entire screen with a single color
    @param dest     --unused--
    @param color    entry from 4 bit palette
*/
void fill_screen(int dest, char color);

void fill_line(int count, int x, int y, int color);
void fill_line_reversed(int count, int x, int y, int color);

#endif


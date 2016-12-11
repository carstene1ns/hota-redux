#ifndef __RENDER_INCLUDED__
#define __RENDER_INCLUDED__

int render_init();
int render_create_surface();

int get_scroll_register();
void set_scroll(int scroll);

void render(unsigned char *src);

void set_palette(int which);
void set_palette_rgb12(unsigned char *rgb12);

void toggle_fullscreen();

int get_current_palette();

#endif

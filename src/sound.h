#ifndef __SOUND_INCLUDED__
#define __SOUND_INCLUDED__

void sound_init();
void sound_done();
void sound_flush_cache();
void play_sample(int index, int volume, int channel);

#endif

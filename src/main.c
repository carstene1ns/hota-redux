/*
 * Heart of The Alien: Game loop and main
 * Copyright (c) 2004-2005 Gil Megidish
 * Copyright (c) 2016-2026 carstene1ns
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
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <assert.h>
#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>
#include "parg.h"

#include "client.h"
#include "vm.h"
#include "rooms.h"
#include "debug.h"
#include "audio.h"
#include "common.h"
#include "files.h"
#include "decode.h"
#include "render.h"
#include "screen.h"
#include "sprites.h"
#include "game2bin.h"
#include "animation.h"

static char *QUICKSAVE_FILENAME = "quicksave";
static char *RECORDED_KEYS_FILENAME = "recorded-keys";

typedef struct anm_file_s
{
	int track;
	const char *filename;
	int offset;
} anm_file_t;

static anm_file_t anm_files[] = 
{
	{31, "INTRO1.BIN", 0},
	{32, "INTRO2.BIN", 0},
	{33, "INTRO3.BIN", 0},
	{34, "INTRO4.BIN", 0},
	{35, "MAKE2MB.BIN", 0x109a},
	{36, "MID2.BIN", 0},
	{37, "END1.BIN", 0},
	{38, "END2.BIN", 0},
	{39, "END3.BIN", 0},
	{40, "END4.BIN", 0}
};

///////
extern int script_ptr;

int next_script;
int current_backdrop;
int current_room;

bool speed_throttle = false;

int debug_flag = 0;
int test_flag = 0;
int record_flag = 0;
int replay_flag = 0;
int fullscreen_flag = 0;
int fastest_flag = 0;

short task_pc[64];
short new_task_pc[64];
short enabled_tasks[64];
short new_enabled_tasks[64];

enum {
	KEY_UP = BIT(0),
	KEY_DOWN = BIT(1),
	KEY_LEFT = BIT(2),
	KEY_RIGHT = BIT(3),
	KEY_A = BIT(4),
	KEY_B = BIT(5),
	KEY_C = BIT(6),
	KEY_START = BIT(7)
};

static int key_state;

#define JOY_DEADZONE 11000

static unsigned int last_tick = 0, last_tick_fp = 0;

#define RECORDED_KEYS_CACHE 4096
static int cached_keys_offset = 0;
static unsigned char cached_recorded_keys[RECORDED_KEYS_CACHE];

/** file descriptor where keys are written to, or read from */
FILE *record_fp = 0;

/** scratchpad used for unpacking code */
static unsigned char scratchpad[29184];

#ifdef ENABLE_DEBUG
short old_var[256+64*32];
#endif

static int load_room(int index)
{
	char filename[16];
	unsigned char *ptr;

	strcpy(filename, "ROOMS0.BIN");
	filename[5] = (index + '0');

	LOG_MAIN("loading %s\n", filename);
	ptr = get_memory_ptr(0xf900);
	if (read_file(filename, ptr) < 0)
	{
		panic("load_room failed");
	}

	script_ptr = get_long(0xf900);
	LOG_MAIN("script ptr %x\n", script_ptr);

	sound_flush_cache();

	return 0;
}

/** atexit() callback
*/
static void atexit_callback()
{
	audio_done();
	SDL_Quit();

	if(cls.iso_prefix != NULL)
	{
		free(cls.iso_prefix);
	}
	if(cls.music_prefix != NULL)
	{
		free(cls.music_prefix);
	}
}

static int initialize()
{
	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO|SDL_INIT_GAMEPAD);
	atexit(atexit_callback);

	if (cls.nosound == 0)
	{
		audio_init();
	}

	if (render_init() < 0)
	{
		panic("failed to initialize renderer module");
	}

	if (game2bin_init() < 0)
	{
		panic("can't read GAME2.BIN file");
	}

	screen_init();
	
	vm_reset();
	set_variable(227, 1);

	// reset keys
	key_state = 0;

	if (render_create_surface() < 0)
	{
		panic("failed to create video surface");
	}

	return 0;
}

/** Loads a screen from room file
    @param room    suffix for room%d.bin file
    @param index   screen number
*/
void load_room_screen(int room, int index)
{
	int i;
	unsigned char *pixels;

	LOG_MAIN("loading room screen %d from room file %d\n", index - 1, room);

	unpack_room(scratchpad, index - 1);           

	/* convert 4bpp -> 8bpp */
	pixels = get_screen_ptr(0);
	for (i=0; i<304*192/2; i++)
	{
		pixels[i*2+0] = scratchpad[i] >> 4;
		pixels[i*2+1] = scratchpad[i] & 0xf;
	}

	current_backdrop = index;
}

/** Rewinds (clears) the recorded-keys cache
*/
void rewind_recorded_keys()
{
	cached_keys_offset = 0;
}

/** Writes down all the recored keys, and rewinds
*/
void flush_recorded_keys()
{
	fwrite(cached_recorded_keys, 1, cached_keys_offset, record_fp);
	cached_keys_offset = 0;
}

/** Reads keystate from record file

    Will read information sufficient for one rendered frame; if the
    record file ended already, all keys will be considered 'released'
*/
void read_keys_from_record()
{
	int c = fgetc(record_fp);

	if (c == EOF)
	{
		c = 0;
		ERROR("record file ended!\n");
	}

	key_state = c;
}

/** Adds a single key to the record file

    Adds a key to the cache array of recorded keys; if the array 
    is full, it will force flushing
*/
void add_keys_to_record()
{
	cached_recorded_keys[cached_keys_offset++] = key_state;
	if (cached_keys_offset == sizeof(cached_recorded_keys))
	{
		/* if the player never quicksaves or quickloads */
		flush_recorded_keys();
	}
}

/** Translates gamepad presses and updates variables

    Entry at b6c4
*/
void update_keys()
{
	short flags;

	toggle_aux(0);           /* me and my paranoia */
	set_variable(253, 0);
	set_variable(252, 0);
	set_variable(229, 0);
	set_variable(251, 0);

	flags = 0;

	if (key_state & KEY_RIGHT)
	{
		set_variable(252, 1);
		flags |= 1;
	}
	else if (key_state & KEY_LEFT)
	{
		set_variable(252, -1);
		flags |= 2;
	}
	else if (key_state & KEY_DOWN)
	{
		set_variable(251, 1);
		set_variable(229, 1);
		flags |= 4;
	}

	if (key_state & KEY_UP)
	{
		set_variable(229, -1);
		flags |= 8;
	}

	if (key_state & KEY_C)
	{
		set_variable(251, -1);
		flags |= 8;
		/* some if  here! */
	}

	set_variable(253, flags);

	/* b748 */
	set_variable(250, 0);
	set_variable(254, get_variable(253));

	if (key_state & KEY_B)
	{
		set_variable(254, (unsigned short)(get_variable(254) | 0x40));
	}
	else if (key_state & KEY_A)
	{
		set_variable(250, 1);
		set_variable(254, (unsigned short)(get_variable(254) | 0x80));
	}

	// FIXME c1: this allows leaving game over screen
	//if (key_state & KEY_START) set_variable(226, 1);
}

/** Loads a quicksave file
*/
void quickload()
{
	int i, j;
	int palette_used;
	FILE *fp;

	fp = fopen(QUICKSAVE_FILENAME, "rb");
	if (fp == NULL)
	{
		perror("failed to load 'quicksave' file\n");
		return;
	}

	current_room = fgetc(fp);
	current_backdrop = fgetc(fp);
	palette_used = fgetc(fp);

	load_room(current_room);
	load_room_screen(0, current_backdrop);
	set_palette(palette_used);

	/* must be ran out of thread loop, so no active thread */
	toggle_aux(0);
	for (i=0; i<256; i++)
	{
		set_variable(i, fgetw(fp));
	}

	toggle_aux(1);
	for (j=0; j<MAX_TASKS; j++)
	{
		set_aux_bank(j);
		for (i=0; i<32; i++)	
		{
			set_variable(i, fgetw(fp));
		}
	}

	/* when frame ends, aux is always zero anyway */
	toggle_aux(0);

	for (i=0; i<MAX_TASKS; i++)
	{
		task_pc[i] = fgetw(fp);
		new_task_pc[i] = fgetw(fp);
		enabled_tasks[i] = fgetw(fp);
		new_enabled_tasks[i] = fgetw(fp);
	}

	quickload_sprites(fp);

	fclose(fp);
}

/** Creates a quicksave file

    Quicksave file includes all that is required so later on a user can
    use the quickload and be provided with the exact same game state. 
*/    
void quicksave()
{
	int i, j;
	FILE *fp;

	fp = fopen(QUICKSAVE_FILENAME, "wb");
	if (fp == NULL)
	{
		perror("failed to create 'quicksave' file\n");
		return;
	}

	fputc(current_room, fp);
	fputc(current_backdrop, fp);
	fputc(get_current_palette(), fp);

	/* must be ran out of thread loop, so no active thread */
	toggle_aux(0);
	for (i=0; i<256; i++)
	{
		fputw(get_variable(i), fp);
	}

	toggle_aux(1);
	for (j=0; j<MAX_TASKS; j++)
	{
		set_aux_bank(j);
		for (i=0; i<32; i++)	
		{
			fputw(get_variable(i), fp);
		}
	}

	toggle_aux(0);

	for (i=0; i<MAX_TASKS; i++)
	{
		fputw(task_pc[i], fp);
		fputw(new_task_pc[i], fp);
		fputw(enabled_tasks[i], fp);
		fputw(new_enabled_tasks[i], fp);
	}

	quicksave_sprites(fp);
	fclose(fp);
}

/** Processes SDL events

    This processes windows messages, keyboard pressed, joystick moves,
    and general SDL events
*/
void check_events()
{
	SDL_Event event;

	#define SET_KEY(x, b) \
		(key_state = (b) ? key_state | (x) : key_state & ~(x))

	while (SDL_PollEvent(&event))
	{
		if(event.type == SDL_EVENT_QUIT) {
			cls.quit = true;
		}
		else if (event.type == SDL_EVENT_GAMEPAD_ADDED)
		{
			const SDL_JoystickID which = event.gdevice.which;
			SDL_Gamepad *gamepad = SDL_OpenGamepad(which);
			if (gamepad && cls.joy_index == -1)
			{
				LOG_MAIN("Opened GamePad %d\n", which);
				cls.joy_index = which;
			}
		}
		else if (event.type == SDL_EVENT_GAMEPAD_REMOVED)
		{
			const SDL_JoystickID which = event.gdevice.which;
			SDL_Gamepad *gamepad = SDL_GetGamepadFromID(which);
			if (gamepad)
			{
				SDL_CloseGamepad(gamepad);
				LOG_MAIN("Closed GamePad %d\n", which);
				if (cls.joy_index == which)
				{
					cls.joy_index = -1;
				}
			}
		}
		else if (event.type == SDL_EVENT_GAMEPAD_AXIS_MOTION)
		{
			const SDL_JoystickID which = event.gaxis.which;
			if (cls.joy_index == which)
			{
				if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTX)
				{
					SET_KEY(KEY_LEFT, event.gaxis.value < -JOY_DEADZONE);
					SET_KEY(KEY_RIGHT, event.gaxis.value > JOY_DEADZONE);
				}
				else if (event.gaxis.axis == SDL_GAMEPAD_AXIS_LEFTY)
				{
					SET_KEY(KEY_UP, event.gaxis.value < -JOY_DEADZONE);
					SET_KEY(KEY_DOWN, event.gaxis.value > JOY_DEADZONE);
				}
			}
		}
		else if ((event.type == SDL_EVENT_GAMEPAD_BUTTON_UP) || (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN))
		{
			const SDL_JoystickID which = event.gbutton.which;
			if (cls.joy_index == which)
			{
				switch(event.gbutton.button)
				{
					case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
					SET_KEY(KEY_RIGHT, event.gbutton.down);
					break;

					case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
					SET_KEY(KEY_LEFT, event.gbutton.down);
					break;

					case SDL_GAMEPAD_BUTTON_DPAD_UP:
					SET_KEY(KEY_UP, event.gbutton.down);
					break;

					case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
					SET_KEY(KEY_DOWN, event.gbutton.down);
					break;

					case SDL_GAMEPAD_BUTTON_WEST:
					SET_KEY(KEY_A, event.gbutton.down);
					break;

					case SDL_GAMEPAD_BUTTON_SOUTH:
					SET_KEY(KEY_B, event.gbutton.down);
					break;

					case SDL_GAMEPAD_BUTTON_EAST:
					SET_KEY(KEY_C, event.gbutton.down);
					break;

					case SDL_GAMEPAD_BUTTON_NORTH:
					SET_KEY(KEY_START, event.gbutton.down);
					break;

					case SDL_GAMEPAD_BUTTON_GUIDE:
					speed_throttle = event.gbutton.down;
					break;

					default:
					/* keep -Wall happy */
					break;
				}

				if (event.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN)
				{
					switch(event.gbutton.button)
					{
						case SDL_GAMEPAD_BUTTON_BACK:
						cls.quit = true;
						break;

						case SDL_GAMEPAD_BUTTON_START:
						toggle_filter();
						break;

						case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
						quicksave();
						break;

						case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
						quickload();
						break;

						default:
						/* keep -Wall happy */
						break;
					}
				}
			}
		}
		else if ((event.type == SDL_EVENT_KEY_UP) || (event.type == SDL_EVENT_KEY_DOWN))
		{
			switch(event.key.key)
			{
				case SDLK_RIGHT:
				SET_KEY(KEY_RIGHT, event.key.down);
				break;

				case SDLK_LEFT:
				SET_KEY(KEY_LEFT, event.key.down);
				break;

				case SDLK_UP:
				SET_KEY(KEY_UP, event.key.down);
				break;

				case SDLK_DOWN:
				SET_KEY(KEY_DOWN, event.key.down);
				break;

				case SDLK_Z:
				case SDLK_A:
				SET_KEY(KEY_A, event.key.down);
				break;

				case SDLK_X:
				case SDLK_S:
				SET_KEY(KEY_B, event.key.down);
				break;

				case SDLK_C:
				case SDLK_D:
				SET_KEY(KEY_C, event.key.down);
				break;

				case SDLK_LCTRL:
				SET_KEY(KEY_START, event.key.down);
				break;

				case SDLK_SPACE:
				speed_throttle = event.key.down;
				break;

				default:
				/* keep -Wall happy */
				break;
			}

			if (event.type == SDL_EVENT_KEY_DOWN)
			{
				switch(event.key.key)
				{
					#ifdef ENABLE_DEBUG
					/* enable/disable sprites */
					case SDLK_1:
					case SDLK_2:
					case SDLK_3:
					case SDLK_4:
					case SDLK_5:
					case SDLK_6:
					case SDLK_7:
					case SDLK_8:
					case SDLK_9:
					{
						int tmp = event.key.key - SDLK_1 + 1;

						if (event.key.mod & SDL_KMOD_SHIFT)
						{
							tmp = tmp + 10;
						}

						sprites[tmp].u1 ^= 0x80;
					}
					break;

					/* enable/disable debug messages */
					case SDLK_KP_1:
					case SDLK_KP_2:
					case SDLK_KP_3:
					case SDLK_KP_4:
					case SDLK_KP_5:
					case SDLK_KP_6:
					case SDLK_KP_7:
					case SDLK_KP_8:
					//case SDLK_KP_9:
					{
						int tmp = event.key.key - SDLK_KP_1 + 1;
						debug_flag ^= BIT(tmp);
					}
					break;
					#endif

					case SDLK_ESCAPE:
					cls.quit = true;
					break;

					case SDLK_F:
					toggle_filter();
					break;

					case SDLK_F5:
					quicksave();
					break;

					case SDLK_F7:
					quickload();
					break;

					case SDLK_F12:
					screenshot();
					break;

					case SDLK_RETURN:
					if (event.key.mod & SDL_KMOD_ALT)
					{
						toggle_fullscreen();
					}
					break;

					default:
					/* keep -Wall happy */
					break;
				}
			}
		}
	}

	#undef SET_KEY
}

void rest(int fps)
{
	if (fastest_flag == 0)
	{
		// only set reference
		if (fps == 0)
		{
			last_tick = SDL_GetTicks();
			last_tick_fp = 0;
			return;
		}

		if (speed_throttle)
		{
			/* 10 times faster */
			fps = fps*10;
		}

		unsigned int diff = ((1000 << 16) / fps) + last_tick_fp;
		last_tick_fp = diff & 0xffff;
		diff = diff >> 16;
		unsigned int current_tick = SDL_GetTicks();
		while (current_tick - last_tick < diff)
		{
			SDL_Delay(1);
			current_tick = SDL_GetTicks();
		}
		last_tick += diff;
	}
}

/** Initializes all tasks to stopped state
*/
void init_tasks()
{
	int i;

	for (i=0; i<MAX_TASKS; i++)
	{
		task_pc[i] = -1;
		new_task_pc[i] = -1;

		enabled_tasks[i] = 0;
		new_enabled_tasks[i] = 0;
	}

	toggle_aux(0);
	task_pc[0] = 0;
}

/** Plays several animations at once
    @param anm        pointer to an array of anm_file_t entries
    @param n          number of elements in array
    @param skippable  skip to next animation if key pressed (otherwise breaks)
    @returns zero on completion of all videos, 1 if skipped, negative on error

    Note that cls.quit might be true; in that case, return value of one will 
    be returned
*/
int play_anm(anm_file_t *anm, int n, int skippable)
{
	int seq;
	int ret;

	ret = 0;
	for (seq = 0; seq < n; seq++)
	{
		if (!cls.quit)
		{
			int ok;

			play_music_track(anm[seq].track, 0);
			ok = play_animation(anm[seq].filename, anm[seq].offset);
			if (ok < 0)
			{
				ret = ok;
				break;
			}

			if (ok == 1 && skippable == 0)
			{
				ret = 1;
				break;
			}
		}
	}

	stop_music();
	return ret;
}

/** Plays the introduction to the game

    Introduction is split into 4 files, since sega-cd was limited with 
    512 KB of ram, and video is loaded into memory before it can be
    played. also, each such sequence comes with it's own audio track
*/
static void play_intro()
{
	play_anm(anm_files, 4, 0);
}

/** Main game loop

    This is where all the magic happens!
*/
static void run()
{
	cls.quit = false;
	init_tasks();

	if (!next_script)
	{
		/* if no room specified, then follow the original flow:
		 * first play intro, then jump to code entry script.
		 */
		play_intro();
		next_script = 7;
	}

	rest(0);

	while (!cls.quit)
	{
		int i;

		if (next_script)
		{
			current_room = next_script;
			reset_sprite_list();
			init_tasks();
			LOG_MAIN("loading room %d\n", current_room);
			load_room(current_room);
			next_script = 0;
		}

		check_events();

		if (replay_flag)
		{
			read_keys_from_record();
		}

		update_keys();

		if (record_flag)
		{
			add_keys_to_record();
		}
	
		LOG_MAIN("*new frame*\n");

		for (i=0; i<MAX_TASKS; i++)
		{
			/* 70d0 */
			enabled_tasks[i] = new_enabled_tasks[i];

			int d0 = new_task_pc[i];
			if (d0 == -1)
			{
				continue;
			}
			else if (d0 == -2)
			{
				d0 = -1;
			}		

			task_pc[i] = d0;
			new_task_pc[i] = -1;
		}

		for (i=0; i<MAX_TASKS; i++)
		{
			int pc = task_pc[i];
			if (pc != INVALID_PC && enabled_tasks[i] == 0)
			{
				toggle_aux(0);
				set_aux_bank(i);
				LOG_TASK("task %d starts at 0x%x\n", i, pc);
				task_pc[i] = decode(i, pc);
				LOG_TASK("task %d ended at 0x%x\n", i, pc);
			}

			if (next_script != 0)
			{
				/* script has been changed */
				break;
			}
		}

		music_update();

		rest(12);
	}
}

/** Runs the animation-test, enabled with --animation-test
*/
static void animation_test()
{
	int files = sizeof(anm_files) / sizeof(anm_files[0]);
	play_anm(anm_files, files, 1);
}

/** Runs the sprite-test 

    Interactively show the sprites in this 'room' file. Use the --room
    parameter to view sprites in other rooms
*/
void sprite_test()
{
	int redraw;

	load_room(next_script);

	sprites[0].index = 0;
	sprites[0].frame = 0;
	sprites[0].x = 10;
	sprites[0].y = 10;

	cls.quit = false;

	redraw = 1;
	set_palette(0x11);
	rest(0);

	while (!cls.quit)
	{
		int a4;

		a4 = get_long(0xf904) + (sprites[0].index << 2);
		a4 = get_long(a4);

		if (redraw)
		{
			int selected_screen;
			void *background;

			selected_screen = get_selected_screen();
			background = get_screen_ptr(selected_screen);
			memset(background, 0xff, 304*192);

			render_sprite(0);
			render(background);
			redraw = 0;
			print_sprite(0);
		}

		check_events();
		rest(12);

		update_keys();

		if (get_variable(252) == 1) // right
		{
			int i = (sprites[0].frame & 0x7f) + 1;
			sprites[0].frame = (sprites[0].frame & 0x80) | i;
			if (sprites[0].frame > get_byte(a4))
			{
				sprites[0].frame &= 0x80;
			}

			redraw = 1;
		}

		if (get_variable(252) == -1) // left
		{
			if (sprites[0].frame > 0)
			{
				int i = (sprites[0].frame & 0x7f) - 1;
				redraw = 1;
				sprites[0].frame = (sprites[0].frame & 0x80) | i;
			}
		}

		if (get_variable(229) == 1) // down
		{
			redraw = 1;
			sprites[0].index++;
			sprites[0].frame &= 0x80;
		}

		if (get_variable(229) == -1) // up
		{
			if (sprites[0].index > 0)
			{
				redraw = 1;
				sprites[0].index--;
				sprites[0].frame &= 0x80;
			}
		}

		if (get_variable(250)) // a
		{
			/* flip */
			sprites[0].frame ^= 0x80;
		}

		// reset direction
		set_variable(229, 0);
		set_variable(252, 0);
	}
}

static void help()
{
	printf("Heart of The Alien Redux %s\n", HOTA_VERSION);
	puts("USAGE:");
	#ifdef ENABLE_DEBUG
	puts("\t--debug        turn on debugging");
	#endif
	puts("\t--scale x      rescale by factor <x>");
	puts("\t--filter       use bilinear filter");
	puts("\t--fullscreen   start in fullscreen");
	puts("\t--room n       start from room <n>");
	puts("\t--iso prefix   use ISO image <prefix.iso>");
	puts("\t--music prefix use music <prefix.[wav/mp3/ogg/opus/flac]>");
	puts("\t--joystick n   use joytick <n> instead of autodetect");
	puts("\t--sprite-test  run sprite test (use with room)");
	puts("\t--intro-test   play all animations");
	puts("\t--fastest      speed throttle");
	puts("\t--record       record keys");
	puts("\t--replay       replay keys");
	puts("\t--help         this help");
}

static struct parg_option options[] =
{
	{"debug", PARG_NOARG, &debug_flag, 1},
	{"room", PARG_REQARG, NULL, 'r'},
	{"iso", PARG_OPTARG, NULL, 'i'},
	{"music", PARG_REQARG, NULL, 'm'},
	{"sprite-test", PARG_NOARG, &test_flag, 1},
	{"intro-test", PARG_NOARG, &test_flag, 2},
	{"help", PARG_NOARG, NULL, 'h'},
	{"no-sound", PARG_NOARG, NULL, 'n'},
	{"fullscreen", PARG_NOARG, &fullscreen_flag, 1},
	{"record", PARG_NOARG, &record_flag, 1},
	{"replay", PARG_NOARG, &replay_flag, 1},
	{"scale", PARG_REQARG, NULL, 's'},
	{"joystick", PARG_REQARG, NULL, 'j'},
	{"filter", PARG_NOARG, NULL, 'f'},
	{"fastest", PARG_NOARG, &fastest_flag, 1},
	{0, 0, 0, 0}
};

int main(int argc, char **argv)
{
	next_script = 0;

	// set default options
	cls.paused = false;
	cls.speed_throttle = false;

	cls.scale = 2;
	cls.filtered = 0;
	cls.fullscreen = 0;

	cls.nosound = 0;

	cls.joy_index = -1;

	cls.use_iso = false;
	cls.iso_prefix = NULL;
	cls.music_prefix = NULL;

	// parse CLI options
	struct parg_state ps;
	parg_init(&ps);
	int c;
	while ((c = parg_getopt_long(&ps, argc, argv, "hdr:ns:fi::m:", options, NULL)) != -1)
	{
		switch(c)
		{
			case 'r':
			next_script = atoi(ps.optarg);
			break;

			case 'h':
			help();
			return 0;

			case 's':
			cls.scale = atoi(ps.optarg);
			if (cls.scale < 2)
			{
				panic("invalid scale factor");
				return 1;
			}
			break;

			case 'f':
			cls.filtered = 1;
			break;

			case 'n':
			cls.nosound = 1;
			break;

			case 'j':
			cls.joy_index = atoi(ps.optarg);
			break;

			case 'i':
			cls.use_iso = true;
			/* manually check for optional argument as next arg */
			if (ps.optarg == NULL && ps.optind < argc && argv[ps.optind][0] != '-')
			{
				ps.optarg = argv[ps.optind++];
			}
			if (ps.optarg != NULL)
			{
				cls.iso_prefix = strdup(ps.optarg);
			}
			break;

			case 'm':
			cls.music_prefix = strdup(ps.optarg);
			break;

			case '?':
			panic("invalid argument");
			return 1;
		}
	}

	if (replay_flag && record_flag)
	{
		fprintf(stderr, "cant specify both replay and record\n");
		return 1;
	}

	if (replay_flag)
	{
		record_fp = fopen(RECORDED_KEYS_FILENAME, "rb");
	}
	else if (record_flag)
	{
		record_fp = fopen(RECORDED_KEYS_FILENAME, "wb");
	}

	initialize();

	switch(test_flag)
	{
		case 0:
		run();
		break;

		case 1:
		sprite_test();
		break;

		case 2:
		animation_test();
		break;

		default:
		fprintf(stderr, "unknown test_flag %d\n", test_flag);
		return 1;
	}

	if (record_fp != NULL)
	{
		flush_recorded_keys();
		fclose(record_fp);
	}

	return 0;
}

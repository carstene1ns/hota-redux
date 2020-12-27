all: alien

CC=gcc
SDL_CFLAGS = `sdl-config --cflags`
SDL_LIBS = `sdl-config --libs`
SMPEG_CFLAGS = `smpeg-config --cflags`
SMPEG_LIBS = `smpeg-config --libs`
CFLAGS=-ggdb -DENABLE_DEBUG -Wall $(SDL_CFLAGS) $(SMPEG_CFLAGS)
OBJS=\
	client.o common.o vm.o      sprites.o decode.o   animation.o   \
	rooms.o  render.o main.o    music.o   debug.o    lzss.o        \
	sound.o  screen.o scale2x.o scale3x.o game2bin.o cd_iso.o

LIBS=$(SDL_LIBS) $(SMPEG_LIBS) -lSDL_mixer

alien: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS) alien


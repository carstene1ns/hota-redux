all: alien

CC=gcc
SDL_CFLAGS = `sdl-config --cflags`
SDL_LIBS = `sdl-config --libs`
CFLAGS=-ggdb -DENABLE_DEBUG -Wall $(SDL_CFLAGS)
OBJS=\
	common.o vm.o sprites.o decode.o animation.o rooms.o \
	render.o main.o music.o debug.o lzss.o cd_iso.o sound.o \
	screen.o

LIBS=-Llib -lSDLmain -lsmpeg -lSDL_mixer $(SDL_LIBS)

alien: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm $(OBJS)

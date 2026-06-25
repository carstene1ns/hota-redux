all: alien

CC=gcc
SDL_CFLAGS = `pkg-config --cflags sdl3 sdl3-mixer`
SDL_LIBS = `pkg-config --libs sdl3 sdl3-mixer`
CFLAGS=-ggdb -DENABLE_DEBUG -Wall $(SDL_CFLAGS)
OBJS=\
	client.o common.o vm.o      sprites.o decode.o   animation.o   \
	rooms.o  render.o main.o    audio.o   debug.o    lzss.o        \
	screen.o game2bin.o cd_iso.o

LIBS=$(SDL_LIBS)

alien: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -f $(OBJS) alien

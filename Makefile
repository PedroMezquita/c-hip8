chip8:
#	gcc chip8.c $(sdl2-config --cflags --libs) -o chip8.h
	gcc src/chip8.c -o ./chip8 -lSDL2main -lSDL2

clean:
	rm chip8 schip

CFLAGS = -I/Library/Frameworks/SDL2.framework/Headers
LDFLAGS = -Wl -F/Library/Frameworks -framework SDL2

DEBUG = -v -g

all : mandelbrot

mandelbrot : mandelbrot.o
	gcc $(DEBUG) -o mandelbrot mandelbrot.o $(LDFLAGS)

mandelbrot.o : mandelbrot.c
	gcc -g -o mandelbrot.o -c mandelbrot.c $(CFLAGS)


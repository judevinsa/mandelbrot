CFLAGS = -I/Library/Frameworks/SDL2.framework/Headers
LDFLAGS = -Wl -F/Library/Frameworks -framework SDL2

DEBUG = -v

all : mandelbrot

mandelbrot : mandelbrot.o
	gcc $(DEBUG) -o mandelbrot mandelbrot.o $(LDFLAGS)

mandelbrot.o : mandelbrot.c
	gcc -o mandelbrot.o -c mandelbrot.c $(CFLAGS)


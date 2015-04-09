LIB = framework SDL2


mandelbrot : mandelbrot.c
	gcc -${LIB} -o mandelbrot mandelbrot.c

verb : mandelbrot.c
	gcc -${LIB} -v -o mandelbrot mandelbrot.c

debug : mandelbrot.c
	gcc -${LIB} -g -o mandelbrot mandelbrot.c
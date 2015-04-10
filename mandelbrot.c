#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <limits.h>

#define uint32 unsigned int // Certified to be 32bits size on a Mac

void defineMandelbrotColors(uint32 * pixelColors, uint32 iterations, int isColored);
void updateMandelbrotPixels(uint32 * pixels, uint32 * pixelColors, uint32 width,
		uint32 height, uint32 iterations);


int main(int argc, char * argv[]) {

	// Various variables
	uint32 width = 1024;
	uint32 height = 768;
	uint32 * pixels;
	uint32 * pixelColors;
	uint32 iterations = 0;
	int isColored = 0;

	int quit = 0;
	SDL_Event event;

	// We redefine the parameters
	if (argc != 5) {
		printf("Error, usage : mandelbrot width height iterations colored\n");
		printf("For the colors : 0 is Black and White, 1 is Colored\n");
		return 1;
	}

	// If negative numbers are entered, they will be huged after cast
	width = (uint32)atoi(argv[1]);
	height = (uint32)atoi(argv[2]);
	iterations = (uint32)atoi(argv[3]);
	isColored = atoi(argv[4]);

	// Test of the conditions
	if (width > 1024) {
		printf("Error : maximal width is 1024 (%u chosen)\n", width);
		return 1;
	}
	if (height > 768) {
		printf("Error : maximal height is 768 (%u chosen)\n", height);
		return 1;
	}
	if (iterations > 1000) {
		printf("Error : maximal number of iterations is 30 (%u chosen)\n", iterations);
		return 1;
	}
	if (isColored != 0 && isColored != 1) {
		printf("Error : choose you color properly, 0 is Black and White, 1 is Colored "
			"(%d chosen)\n", isColored);
		return 1;
	}

	// We allocate some memory for the pixel colors
	pixelColors = (uint32 *)calloc(iterations + 1, (iterations + 1) * sizeof(uint32));

	// Defines the color of the mandelbrot pixels
	defineMandelbrotColors(pixelColors, iterations, isColored);

	// Video initializing
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("SDL_Init Error: %s\n", SDL_GetError());
		return 1;
	}

	// Gets a new window for rendering
	SDL_Window * theWindow = SDL_CreateWindow("Mandelbrot", 200, 200, width, height, 0);
	if (!theWindow) {
		printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
	}

	// Creates the renderer for the latter window
	SDL_Renderer * theRenderer = SDL_CreateRenderer(theWindow, -1,0);
	if (!theRenderer) {
		SDL_DestroyWindow(theWindow);
		printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	// Creates a new texture for the window
	//	Texture is made of ARGB pixels, we well draw pixel by pixel
	SDL_Texture * theTexture = SDL_CreateTexture(theRenderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STATIC, width, height);

	// A big bunch of pixels (pixel infos fits in an uint32)
	pixels = (uint32 *)calloc(width * height, width * height * sizeof(uint32));

	updateMandelbrotPixels(pixels, pixelColors, width, height, iterations);

	// While loop of the application
	while (!quit) {

		// Updates the texture at each cycle
		SDL_UpdateTexture(theTexture, NULL, pixels, width * sizeof(uint32));

		// Listens for any user event
		SDL_WaitEvent(&event);

		switch (event.type) {

			case SDL_QUIT :
				quit = 1;
				break;
		}

		// Renders the updated window
		SDL_RenderClear(theRenderer);
		SDL_RenderCopy(theRenderer, theTexture, NULL, NULL);
		SDL_RenderPresent(theRenderer);
	}

	free(pixels);
	free(pixelColors);
	SDL_DestroyTexture(theTexture);
	SDL_DestroyRenderer(theRenderer);
	SDL_DestroyWindow(theWindow);
	SDL_Quit();

	return 0;
}


/*
 *	Redefines the colors that will be used on the pixels with the parameters entered
 */
void defineMandelbrotColors(uint32 * pixelColors, uint32 iterations, int isColored) {
	for (int i = 0; i < iterations; i++) {
		if (isColored) {
			unsigned char red = (unsigned char)(212 + 2 * i * (255 / iterations));
			unsigned char green = (unsigned char)(149 + 3 * i * (255 / iterations));
			unsigned char blue = (unsigned char)(97 + i * (255 / iterations));
			pixelColors[i] = (((((255 << 8) + red) << 8) + green) << 8) + blue;
		} else {
			unsigned char gray = (unsigned char)(55 + i * (200 / iterations));
			pixelColors[i] = (((((255 << 8) + gray) << 8) + gray) << 8) + gray;
		}
	}
}



/*
 * Overwrites the pixels following the calculation of the mandelbrot figure
 *	Algorithm from : http://villemin.gerard.free.fr/Wwwgvmm/Suite/FracPrat.htm
 */
void updateMandelbrotPixels(uint32 * pixels, uint32 * pixelColors, uint32 width,
		uint32 height, uint32 iterations) {
	float minX = -2.4;
	float maxX = 2.4;
	float minY = -1.5;
	float maxY = 1.5;

	for (uint32 x = 0; x < width; x++) {
		for (uint32 y = 0; y < height; y++) {
			float realC = minX + (maxX - minX) / ((float)width) * (float)x;
			float imC = minY + (maxY - minY) / ((float)height) * (float)y;
			float realZ = 0;
			float imZ = 0;
			uint32 a = 0;

			for (; a < iterations; a++) {
				float realPart = realZ;
				float imPart = imZ;

				realZ = realPart * realPart - imPart * imPart + realC;
				imZ = 2 * realPart * imPart + imC;

				if ((realZ * realZ + imZ * imZ) >= 4.0) {
					break;
				}
			}

			pixels[width * y + x] = pixelColors[a];
		}
	}
}




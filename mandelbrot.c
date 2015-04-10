#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#define uint32 unsigned int // Certified to be 32bits size on a Mac



int main(int argc, char * argv[]) {

	// Various variables
	int quit = 0;
	SDL_Event event;

	// Video initializing
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("SDL_Init Error: %s\n", SDL_GetError());
		return 1;
	}

	// Gets a new window for rendering
	SDL_Window * theWindow = SDL_CreateWindow("Mandelbrot", 200, 200, 1024, 768, 0);
	if (!theWindow) {
		printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
	}

	// Creates the renderer for the latter window
	SDL_Renderer * theRenderer = SDL_CreateRenderer(theWindow, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!theRenderer) {
		SDL_DestroyWindow(theWindow);
		printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	// Creates a new texture for the window
	//	Texture is made of ARGB pixels, we well draw pixel by pixel
	SDL_Texture * theTexture = SDL_CreateTexture(theRenderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STATIC, 1024, 768);

	// A big bunch of pixels (pixel infos fits in an uint32)
	uint32 * pixels = (uint32 *)malloc(1024 * 768 * sizeof(uint32));


	// While loop of the application
	while (!quit) {


		// Listens for any user event
		SDL_WaitEvent(&event);

		switch (event.type) {

			case SDL_QUIT :
				quit = 1;
				break;
		}
	}

	SDL_DestroyTexture(theTexture);
	SDL_DestroyRenderer(theRenderer);
	SDL_DestroyWindow(theWindow);
	SDL_Quit();

	return 0;
}
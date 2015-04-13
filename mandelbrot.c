#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <limits.h>
#include <pthread.h>

#define uint32 unsigned int // Certified to be 32bits size on a Mac


// Definition of threads
struct thread_data {
	int threadId;
	uint32 * pixels;
	uint32 * pixelColors;
	uint32 width, height, iterations, bandWidth, threadNumber, computationStart;
};

void * threadHandler(void * thread_data);


// Functions used in the computation process
void defineMandelbrotColors(uint32 * pixelColors, uint32 iterations, int isColored);
void updateMandelbrotPixels(uint32 * pixels, uint32 * pixelColors, uint32 width,
		uint32 height, uint32 iterations, uint32 startX, uint32 endX);


int main(int argc, char * argv[]) {

	// Eseential Variables
	uint32 width = 1024;
	uint32 height = 768;
	uint32 bandWidth = 0;
	uint32 threadNumber = 1;
	uint32 * pixels;
	uint32 * pixelColors;
	uint32 iterations = 0;
	int isColored = 0;

	int quit = 0;
	SDL_Event event;

	// We redefine the parameters
	if (argc != 7) {
		printf("Error, usage : mandelbrot width height iterations colored"
			" band_width number_of_threads\n");
		printf("For the colors : 0 is Black and White, 1 is Colored\n");
		return 1;
	}

	// If negative numbers are entered, they will be huged after cast
	width = (uint32)atoi(argv[1]);
	height = (uint32)atoi(argv[2]);
	iterations = (uint32)atoi(argv[3]);
	isColored = atoi(argv[4]);
	bandWidth = atoi(argv[5]);
	threadNumber = atoi(argv[6]);

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
	if (bandWidth < 1 || bandWidth > width) {
		printf("Error : bandWidth must be between 1 and Width (%u chosen)\n", bandWidth);
		return 1;
	}
	if (threadNumber < 1 || threadNumber > 100) {
		printf("Error : threadNumber must be between 1 and 100 (%u chosen)\n", threadNumber);
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

	// We define here the content of the pthread that will computes the Mandelbrot
	pthread_t pMandelbrot[threadNumber];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	struct thread_data data_to_be_passed[threadNumber];

	for (int i = 0; i < threadNumber; i++) {
		data_to_be_passed[i].threadId = i+1;
		data_to_be_passed[i].pixels = pixels;
		data_to_be_passed[i].pixelColors = pixelColors;
		data_to_be_passed[i].width = width;
		data_to_be_passed[i].height = height;
		data_to_be_passed[i].iterations = iterations;
		data_to_be_passed[i].bandWidth = bandWidth;
		data_to_be_passed[i].threadNumber = threadNumber;
		data_to_be_passed[i].computationStart = i * bandWidth;
	}

	for (int i = 0; i < threadNumber; i++) {
		int isThreadCreated = pthread_create(&pMandelbrot[i], &attr, threadHandler,
				(void *)&data_to_be_passed[i]);
	
		if (isThreadCreated) {
			printf("Error, computation thread not created : error %d\n", isThreadCreated);
			exit(1);
		}

	}

	// While loop of the application
	while (!quit) {
			

		// Updates the texture at each cycle
		SDL_UpdateTexture(theTexture, NULL, pixels, width * sizeof(uint32));

		// Listens for any user event
		SDL_WaitEventTimeout(&event, 5);

		switch (event.type) {

			case SDL_QUIT :
				quit = 1;
				for (int i = 0; i < threadNumber; i++) {
					void * status;
					pthread_cancel(pMandelbrot[i]);
					pthread_join(pMandelbrot[i], status);
				}
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
 *	Defines the functions handeld by the computing thread
 */
void * threadHandler (void * passed_thread_data) {
	struct thread_data * used_data = (struct thread_data *)passed_thread_data;

	uint32 startX = used_data->computationStart;
	uint32 endX = startX + used_data->bandWidth;
	while(1) {
		updateMandelbrotPixels(used_data->pixels, used_data->pixelColors, 
			used_data->width, used_data->height, used_data->iterations,
			startX, endX);

		if(startX + (used_data->threadNumber + 1) * used_data->bandWidth
				< used_data->width) {
			startX = startX + used_data->threadNumber * used_data->bandWidth;
			endX = startX + used_data->bandWidth;
		} else if (startX + used_data->threadNumber * used_data->bandWidth
				< used_data->width) {
			startX = startX + used_data->threadNumber * used_data->bandWidth;
			endX = used_data->width;
		} else {
			break;
		}
	}

	pthread_exit((void *) 0);
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
			unsigned char gray = (unsigned char)(i * (255 / iterations));
			pixelColors[i] = (((((255 << 8) + gray) << 8) + gray) << 8) + gray;
		}
	}
}



/*
 * Overwrites the pixels following the calculation of the mandelbrot figure
 *	Algorithm from : http://villemin.gerard.free.fr/Wwwgvmm/Suite/FracPrat.htm
 */
void updateMandelbrotPixels(uint32 * pixels, uint32 * pixelColors, uint32 width,
		uint32 height, uint32 iterations, uint32 startX, uint32 endX) {
	float minX = -2.4;
	float maxX = 2.4;
	float minY = -1.5;
	float maxY = 1.5;

	for (uint32 x = startX; x < endX; x++) {
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
			SDL_Delay(1);
		}
	}
}




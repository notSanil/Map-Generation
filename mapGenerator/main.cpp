#include <stdio.h>
#include <SDL.h>

#include "Application.h"


int main(int argc, char** argv)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("SDL failed to initialise. SDL_ERROR: %s", SDL_GetError());
	}

	const int WIDTH = 600;
	const int HEIGHT = WIDTH;

	SDL_Window* window = nullptr;
	SDL_Renderer* renderer = nullptr;
	SDL_CreateWindowAndRenderer(WIDTH, HEIGHT, SDL_WINDOW_SHOWN, &window, &renderer);
	
	if (window == nullptr)
	{
		printf("Window creation failed. SDL_ERROR: %s", SDL_GetError());
	}
	if (renderer == nullptr)
	{
		printf("Renderer creation failed. SDL_ERROR: %s", SDL_GetError());
	}

	Application app(renderer);
	app.onExecute();
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
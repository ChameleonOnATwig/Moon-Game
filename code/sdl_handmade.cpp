#include <SDL2/SDL.h>
#include <stdlib.h>

static SDL_Texture *gp_texture;
static void *gp_pixels;
static int g_texture_width;

static void SDLResizeTexture(SDL_Renderer *renderer, int width, int height) {
	if (gp_pixels) { free(gp_pixels); }
	if (gp_texture) { SDL_DestroyTexture(gp_texture); }

	gp_texture = SDL_CreateTexture(renderer,
							 SDL_PIXELFORMAT_ARGB8888,
							 SDL_TEXTUREACCESS_STREAMING,
							 width,
							 height);

	g_texture_width = width;
	gp_pixels = malloc(width * height * 4);
}

static void SDLUpdateWindow(SDL_Window *window, SDL_Renderer *renderer) {
	SDL_UpdateTexture(gp_texture,
				   0,
				   gp_pixels,
				   g_texture_width * 4);

	SDL_RenderCopy(renderer,
				gp_texture,
				0,
				0);

	SDL_RenderPresent(renderer);
}

bool HandleEvent(SDL_Event *event) {
	bool should_quit = false;

	switch(event->type)	{
		case SDL_QUIT: {
			printf("SDL_QUIT\n");
			should_quit = true;
		} break;

		case SDL_WINDOWEVENT: {
			switch(event->window.event) {
				case SDL_WINDOWEVENT_SIZE_CHANGED: {
					SDL_Window *window = SDL_GetWindowFromID(event->window.windowID);
					SDL_Renderer *renderer = SDL_GetRenderer(window);

					printf("SDL_WINDOWEVENT_SIZE_CHANGED (%d, %d)\n", event->window.data1, event->window.data2);

					SDLResizeTexture(renderer, event->window.data1, event->window.data2);
				} break;

				case SDL_WINDOWEVENT_EXPOSED: {
					SDL_Window *window = SDL_GetWindowFromID(event->window.windowID);
					SDL_Renderer *renderer = SDL_GetRenderer(window);

					SDLUpdateWindow(window, renderer);
				} break;
			}
		} break;
	}
	return(should_quit);
}

int main(int argc, char *argv[]) {
	// Makes a Simple Text Message Box that Waits until Ok is Pressed
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
						  "Handmade Hero",
						  "This is Handmade Hero",
						  0);

	// Create a Window
	SDL_Window *window = SDL_CreateWindow("Handmade Hero",
									   SDL_WINDOWPOS_UNDEFINED,
									   SDL_WINDOWPOS_UNDEFINED,
									   640,
									   480,
									   SDL_WINDOW_RESIZABLE);

	if (window) {
		// Create a Renderer for the Window
		SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);

		if (renderer) {
			while(true) {
				SDL_Event event;
				SDL_WaitEvent(&event);
				if (HandleEvent(&event)) { break; }
			}
		}
	}

	SDL_Quit();

	return 0;
}

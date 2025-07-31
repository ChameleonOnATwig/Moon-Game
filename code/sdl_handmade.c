#include <SDL2/SDL.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <sys/mman.h>

static SDL_Texture *gp_texture;
static void *gp_bitmap_memory;
static int g_bitmap_width;
static int g_bitmap_height;
static int g_bytes_per_pixel = 4;

static void SDLResizeTexture(SDL_Renderer *renderer, int width, int height) {
	if (gp_bitmap_memory) {
		munmap(gp_bitmap_memory,
		 g_bitmap_width * g_bitmap_height * g_bytes_per_pixel);
	}
	if (gp_texture) { SDL_DestroyTexture(gp_texture); }

	gp_texture = SDL_CreateTexture(renderer,
							 SDL_PIXELFORMAT_ARGB8888,
							 SDL_TEXTUREACCESS_STREAMING,
							 width,
							 height);

	g_bitmap_width = width;
	g_bitmap_height = height;
	gp_bitmap_memory = mmap(0,
						 width * height * g_bytes_per_pixel,
						 PROT_READ | PROT_WRITE,
						 MAP_ANONYMOUS | MAP_PRIVATE,
						 -1,
						 0);
}

static void SDLUpdateWindow(SDL_Window *window, SDL_Renderer *renderer) {
	SDL_UpdateTexture(gp_texture,
				   0,
				   gp_bitmap_memory,
				   g_bitmap_width * g_bytes_per_pixel);

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
			int width, height;
			SDL_GetWindowSize(window, &width, &height);
			SDLResizeTexture(renderer, width, height);

			bool game_running = true;
			while(game_running) {
				SDL_Event event;
				while(SDL_PollEvent(&event)) {
					if (HandleEvent(&event)) {
						game_running = false;
					}
				}
				SDLUpdateWindow(window, renderer);
			}
		}
	}

	SDL_Quit();
	return 0;
}

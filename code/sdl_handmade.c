#include <SDL2/SDL.h>
#include <stdbool.h>
#include <sys/mman.h>

struct SDL_Offscreen_Buffer {
	// NOTE: Pixels are always 32-bits wide, Memory Order: BB GG RR XX
	SDL_Texture *p_texture;
	void *p_memory;
	int width;
	int height;
	int pitch;
};

struct SDL_Window_Dimension {
	int width;
	int height;
};

static struct SDL_Offscreen_Buffer g_backbuffer;

static struct SDL_Window_Dimension SDLGetWindowDimension(SDL_Window *p_window) {
	struct SDL_Window_Dimension result;
	SDL_GetWindowSize(p_window, &result.width, &result.height);
	return result;
}

static void SDLResizeTexture(struct SDL_Offscreen_Buffer *p_buffer, SDL_Renderer *p_renderer, int width, int height) {
	const int bytes_per_pixel = 4;

	if (p_buffer->p_memory) {
		munmap(p_buffer->p_memory,
		 p_buffer->width * p_buffer->height * bytes_per_pixel);
	}
	if (p_buffer->p_texture) { SDL_DestroyTexture(p_buffer->p_texture); }

	p_buffer->p_texture = SDL_CreateTexture(p_renderer,
							 SDL_PIXELFORMAT_ARGB8888,
							 SDL_TEXTUREACCESS_STREAMING,
							 width,
							 height);

	p_buffer->width = width;
	p_buffer->height = height;
	p_buffer->pitch = width * bytes_per_pixel;
	p_buffer->p_memory = mmap(0,
						 width * height * bytes_per_pixel,
						 PROT_READ | PROT_WRITE,
						 MAP_ANONYMOUS | MAP_PRIVATE,
						 -1,
						 0);
}

static void SDLUpdateWindow(SDL_Window *p_window, SDL_Renderer *p_renderer, struct SDL_Offscreen_Buffer buffer) {
	SDL_UpdateTexture(buffer.p_texture,
				   0,
				   buffer.p_memory,
				   buffer.pitch);

	SDL_RenderCopy(p_renderer,
				buffer.p_texture,
				0,
				0);

	SDL_RenderPresent(p_renderer);
}

bool HandleEvent(SDL_Event *p_event) {
	bool should_quit = false;

	switch(p_event->type)	{
		case SDL_QUIT: {
			printf("SDL_QUIT\n");
			should_quit = true;
		} break;

		case SDL_WINDOWEVENT: {
			switch(p_event->window.event) {
				case SDL_WINDOWEVENT_SIZE_CHANGED: {
					SDL_Window *p_window = SDL_GetWindowFromID(p_event->window.windowID);
					SDL_Renderer *p_renderer = SDL_GetRenderer(p_window);

					printf("SDL_WINDOWEVENT_SIZE_CHANGED (%d, %d)\n", p_event->window.data1, p_event->window.data2);

					SDLResizeTexture(&g_backbuffer, p_renderer, p_event->window.data1, p_event->window.data2);
				} break;

				case SDL_WINDOWEVENT_EXPOSED: {
					SDL_Window *p_window = SDL_GetWindowFromID(p_event->window.windowID);
					SDL_Renderer *p_renderer = SDL_GetRenderer(p_window);

					SDLUpdateWindow(p_window, p_renderer, g_backbuffer);
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
	SDL_Window *p_window = SDL_CreateWindow("Handmade Hero",
									   SDL_WINDOWPOS_UNDEFINED,
									   SDL_WINDOWPOS_UNDEFINED,
									   640,
									   480,
									   SDL_WINDOW_RESIZABLE);

	if (p_window) {
		// Create a Renderer for the Window
		SDL_Renderer *p_renderer = SDL_CreateRenderer(p_window, -1, 0);

		if (p_renderer) {
			int width, height;
			SDLGetWindowDimension(p_window);
			SDLResizeTexture(&g_backbuffer, p_renderer, width, height);

			bool game_running = true;
			while(game_running) {
				SDL_Event event;
				while(SDL_PollEvent(&event)) {
					if (HandleEvent(&event)) {
						game_running = false;
					}
				}
				SDLUpdateWindow(p_window, p_renderer, g_backbuffer);
			}
		}
	}

	SDL_Quit();
	return 0;
}

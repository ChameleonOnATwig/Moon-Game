#include <SDL2/SDL.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_video.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/mman.h>


#define internal static
#define local_persist static
#define global_variable static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;


// MAP_ANONYMOUS is not defined on MAC OS X and some UNIX systems.
// MAP_ANON is used instead for those.
#ifndef MAP_ANONYMOUS
#define MAP_ANONYMOUS MAP_ANON
#endif

// Matches SDL_PIXELFORMAT_ARGB8888
#define BYTES_PER_PIXEL 4
#define MAX_CONTROLLERS 4

// Controller Global Variables
global_variable SDL_GameController *g_controller_handles[MAX_CONTROLLERS];
global_variable SDL_Haptic *g_rumble_handles[MAX_CONTROLLERS];

// Weird Gradient Global Variables
global_variable int g_x_offset = 0;
global_variable int g_y_offset = 0;

global_variable struct SDL_Offscreen_Buffer g_backbuffer;

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

static struct SDL_Window_Dimension SDLGetWindowDimension(SDL_Window *p_window) {
	struct SDL_Window_Dimension result;
	SDL_GetWindowSize(p_window, &result.width, &result.height);
	return result;
}


internal void RenderWeirdGradient(struct SDL_Offscreen_Buffer *p_buffer,
								  int blue_offset,
								  int green_offset) {    

    uint8 *p_row = (uint8 *)p_buffer->p_memory;

    for (int y = 0; y < p_buffer->height; ++y) {
        uint32 *p_pixel = (uint32 *)p_row;

        for (int x = 0; x < p_buffer->width; ++x) {
            uint8 blue = (x + blue_offset);
            uint8 green = (y + green_offset);
            
            *p_pixel++ = ((green << 8) | blue);
        }

        p_row += p_buffer->pitch;
    }
}

internal void SDLResizeTexture(struct SDL_Offscreen_Buffer *p_buffer,
							   SDL_Renderer *p_renderer,
							   int width,
							   int height) {

	if (p_buffer->p_memory) {
		munmap(p_buffer->p_memory,
			   p_buffer->width * p_buffer->height * BYTES_PER_PIXEL);
	}

	if (p_buffer->p_texture) {
		SDL_DestroyTexture(p_buffer->p_texture);
	}

	p_buffer->p_texture = SDL_CreateTexture(p_renderer,
											SDL_PIXELFORMAT_ARGB8888,
							 				SDL_TEXTUREACCESS_STREAMING,
							 				width,
							 				height);

	p_buffer->width = width;
	p_buffer->height = height;
	p_buffer->pitch = width * BYTES_PER_PIXEL;
	p_buffer->p_memory = mmap(0,
							  width * height * BYTES_PER_PIXEL,
						   	  PROT_READ | PROT_WRITE,
						   	  MAP_PRIVATE | MAP_ANONYMOUS,
						   	  -1,
						   	  0);
}

internal void SDLUpdateWindow(SDL_Window *p_window,
							  SDL_Renderer *p_renderer,
							  const struct SDL_Offscreen_Buffer *p_buffer) {

	SDL_UpdateTexture(p_buffer->p_texture,
					  0,
				   	  p_buffer->p_memory,
				   	  p_buffer->pitch);

	SDL_RenderCopy(p_renderer,
				   p_buffer->p_texture,
				   0,
				   0);

	SDL_RenderPresent(p_renderer);
}

bool HandleEvent(SDL_Event *p_event) {
	bool should_quit = false;

	switch (p_event->type) {
		case SDL_QUIT: {
			printf("SDL_QUIT\n");
			should_quit = true;
		} break;

		case SDL_KEYDOWN:

		case SDL_KEYUP: {
			SDL_Keycode key_code = p_event->key.keysym.sym;
			bool is_down = (p_event->key.state == SDL_PRESSED);
			bool was_down = false;

			if (p_event->key.state == SDL_RELEASED) {
				was_down = true;
			} else if (p_event->key.repeat != 0) {
				was_down = true;
			}

			if (p_event->key.repeat == 0) {
				switch (key_code) {
					case SDLK_w: {
						--g_y_offset;
					} break;

					case SDLK_a: {
						--g_x_offset;
					} break;

					case SDLK_s: {
						++g_y_offset;
					} break;

					case SDLK_d: {
						++g_x_offset;
					} break;

					case SDLK_ESCAPE: {
						printf("ESCAPE: ");
						if (is_down) {
							printf("IsDown ");
						}
						if (was_down) {
							printf("WasDown");
						}
						printf("\n");
					} break;
				}
            }

		} break;

		case SDL_WINDOWEVENT: {
			switch (p_event->window.event) {
				case SDL_WINDOWEVENT_SIZE_CHANGED: {
					SDL_Window *p_window = SDL_GetWindowFromID(p_event->window.windowID);
					SDL_Renderer *p_renderer = SDL_GetRenderer(p_window);

					printf("SDL_WINDOWEVENT_SIZE_CHANGED (%d, %d)\n",
						   p_event->window.data1,
						   p_event->window.data2);

					SDLResizeTexture(&g_backbuffer,
									 p_renderer,
									 p_event->window.data1,
									 p_event->window.data2);
				} break;

				case SDL_WINDOWEVENT_FOCUS_GAINED: {
					printf("SDL_WINDOWEVENT_FOCUS_GAINED\n");
				} break;

				case SDL_WINDOWEVENT_EXPOSED: {
					SDL_Window *p_window = SDL_GetWindowFromID(p_event->window.windowID);
					SDL_Renderer *p_renderer = SDL_GetRenderer(p_window);

					RenderWeirdGradient(&g_backbuffer, g_x_offset, g_y_offset);
					SDLUpdateWindow(p_window, p_renderer, &g_backbuffer);
				} break;
			}
		} break;
	}

	return should_quit;
}

internal void SDLOpenGameControllers() {
	int max_joysticks = SDL_NumJoysticks();
	int controller_index = 0;

	for (int joystick_index = 0; joystick_index < max_joysticks; ++joystick_index) {
		if (!SDL_IsGameController(joystick_index)) {
			continue;
		}
		if (controller_index >= MAX_CONTROLLERS) {
			break;
		}

		g_controller_handles[controller_index] = SDL_GameControllerOpen(joystick_index);
		g_rumble_handles[controller_index] = SDL_HapticOpen(joystick_index);

		if (g_rumble_handles[controller_index]
			&& SDL_HapticRumbleInit(g_rumble_handles[controller_index]) != 0) {

			SDL_HapticClose(g_rumble_handles[controller_index]);
            g_rumble_handles[controller_index] = 0;
		}

		++controller_index;
	}
}

internal void SDLCloseGameControllers() {
	for (int controller_index = 0; controller_index < MAX_CONTROLLERS; ++controller_index) {
		if (g_controller_handles[controller_index]) {
			if (g_rumble_handles[controller_index]) {
                SDL_HapticClose(g_rumble_handles[controller_index]);
			}

			SDL_GameControllerClose(g_controller_handles[controller_index]);
		}
	}
}

int main(int argc, char *argv[]) {

	// Makes a Simple Text Message Box that Waits until Ok is Pressed
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
							 "Handmade Hero",
							 "This is Handmade Hero",
							 0);

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC);
	SDLOpenGameControllers();

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
			struct SDL_Window_Dimension dimension = SDLGetWindowDimension(p_window);
			SDLResizeTexture(&g_backbuffer,
							 p_renderer,
							 dimension.width,
							 dimension.height);

			bool game_running = true;
			while (game_running) {
				SDL_Event event;
				while (SDL_PollEvent(&event)) {
					if (HandleEvent(&event)) {
						game_running = false;
					}
				}

				for (int controller_index = 0; controller_index < MAX_CONTROLLERS; ++controller_index) {
					SDL_GameController *p_controller = g_controller_handles[controller_index];

					if (p_controller != 0 && SDL_GameControllerGetAttached(p_controller)) {

						// NOTE: We have a g_controller with index controller_index.
						bool up = SDL_GameControllerGetButton(p_controller, SDL_CONTROLLER_BUTTON_DPAD_UP);
						bool down = SDL_GameControllerGetButton(p_controller, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
						bool left = SDL_GameControllerGetButton(p_controller, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
						bool right = SDL_GameControllerGetButton(p_controller, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
						bool start = SDL_GameControllerGetButton(p_controller, SDL_CONTROLLER_BUTTON_START);
						bool back = SDL_GameControllerGetButton(p_controller, SDL_CONTROLLER_BUTTON_BACK);
						bool left_shoulder = SDL_GameControllerGetButton(p_controller, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
						bool right_shoulder = SDL_GameControllerGetButton(p_controller, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
						bool a_button = SDL_GameControllerGetButton(p_controller, SDL_CONTROLLER_BUTTON_A);
						bool b_button = SDL_GameControllerGetButton(p_controller, SDL_CONTROLLER_BUTTON_B);
						bool x_button = SDL_GameControllerGetButton(p_controller, SDL_CONTROLLER_BUTTON_X);
						bool y_button = SDL_GameControllerGetButton(p_controller, SDL_CONTROLLER_BUTTON_Y);

						int16 stick_x = SDL_GameControllerGetAxis(p_controller, SDL_CONTROLLER_AXIS_LEFTX);
						int16 stick_y = SDL_GameControllerGetAxis(p_controller, SDL_CONTROLLER_AXIS_LEFTY);
					} else {
						// TODO: This controller is note plugged in.
					}
				}

				RenderWeirdGradient(&g_backbuffer, g_x_offset, g_y_offset);
				SDLUpdateWindow(p_window, p_renderer, &g_backbuffer);
			}
		}
	}

    SDLCloseGameControllers();
	SDL_Quit();
	return 0;
}

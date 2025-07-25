#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

bool HandleEvent(SDL_Event *Event) {

	bool ShouldQuit = false;

	switch(Event->type)	{

		case SDL_QUIT: {

			printf("SDL_QUIT\n");
			ShouldQuit = true;

		} break;

		case SDL_WINDOWEVENT: {

			switch(Event->window.event) {

				case SDL_WINDOWEVENT_RESIZED: {
					
					printf("SDL_WINDOWEVENT_RESIZED (%d, %d)\n", Event->window.data1, Event->window.data2);

				} break;

				case SDL_WINDOWEVENT_EXPOSED: {

					SDL_Window *Window = SDL_GetWindowFromID(Event->window.windowID);
					SDL_Renderer *Renderer = SDL_GetRenderer(Window);

					static bool IsWhite = true;
					
					if (IsWhite == true) {
						SDL_SetRenderDrawColor(Renderer, 255, 255, 255, 255);
						IsWhite = false;
					} else {
						SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 255);
						IsWhite = true;
					}

					SDL_RenderClear(Renderer);
					SDL_RenderPresent(Renderer);
				} break;
			}
		} break;
	}

	return(ShouldQuit);
}

int main(int argc, char *argv[]) {
	// Makes a Simple Text Message Box that Waits until Ok is Pressed
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,
						  "Handmade Hero",
						  "This is Handmade Hero",
						  0);

	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		//TODO: SDL_Init didn't work!
	}

	SDL_Window *Window = SDL_CreateWindow("Handmade Hero",
						   SDL_WINDOWPOS_UNDEFINED,
						   SDL_WINDOWPOS_UNDEFINED,
						   640,
						   480,
						   SDL_WINDOW_RESIZABLE);

	if (Window) {

		SDL_Renderer *Renderer = SDL_CreateRenderer(Window, -1, 0);

		if (Renderer) {
			while(true) {
				SDL_Event Event;
				SDL_WaitEvent(&Event);
				if (HandleEvent(&Event)) {
					break;
				}
			}
		}
	}

	SDL_Quit();

	return 0;
}

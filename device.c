#include <stdlib.h>
#include <stdio.h>

#include <SDL2/SDL.h>

#include "device.h"


pixeldata pixels = {
	{0},
	RES_X * 4
};

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
SDL_Event event;

bool is_running = false;


bool set_up_device() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		return false;
	}

	window = SDL_CreateWindow(
			"buffdog",
			SDL_WINDOWPOS_UNDEFINED,
			SDL_WINDOWPOS_UNDEFINED,
			RES_X,
			RES_Y,
			SDL_WINDOW_SHOWN);

	if (window == NULL) {
		SDL_Log("Unable to create SDL window: %s", SDL_GetError());
		return false;
	}

	renderer = SDL_CreateRenderer(window, -1, 0);

	if (renderer == NULL) {
		SDL_Log("Unable to create SDL renderer: %s", SDL_GetError());
		return false;
	}

	texture = SDL_CreateTexture(
			renderer,
			SDL_PIXELFORMAT_RGBX8888, // display mode is SDL_PIXELFORMAT_ARGB8888?
			SDL_TEXTUREACCESS_STATIC,
			RES_X,
			RES_Y);

	if (texture == NULL) {
		SDL_Log("Unable to create SDL texture: %s", SDL_GetError());
		return false;
	}

	// capture mouse
	SDL_SetRelativeMouseMode(SDL_TRUE);

	// wait for the window to be exposed
	{
		do {
			SDL_PollEvent(&event);

			if (
					event.type == SDL_WINDOWEVENT &&
					event.window.event == SDL_WINDOWEVENT_SHOWN) {
				is_running = true;
			}
		} while (!is_running);
	}

	return true;
}

void close_device() {
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyTexture(texture);
	SDL_Quit();
}

bool running() {
	return is_running;
}

void process_events() {
	while (SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_QUIT:
				spit("quitting");
				is_running = false;
				break;

			default:
				// noop;
			break;
		}
	}
}

uint32_t color(double red, double green, double blue) {
	unsigned char red_value = 255 * red;
	unsigned char green_value = 255 * green;
	unsigned char blue_value = 255 * blue;
	return (red_value << 24) + (green_value << 16) + (blue_value << 8);
}

void clear_screen() {
	for (int y = 0; y < RES_Y; ++y) {
		for (int x = 0; x < RES_X; ++x) {
			draw_pixel(x, y, color(0, 0, 0));
		}
	}
}

void draw_pixel(int x, int y, int color) {
	// invert y since it starts at the top
	y = RES_Y - y - 1;

  // uint32_t r = red & 0x000000FF;
  // r = r << 24;
	//
  // uint32_t g = green & 0x000000FF;
  // g = g << 16;
	//
  // uint32_t b = blue & 0x000000FF;
  // b = b << 8;
	//
	// unsigned char *raw_data = (unsigned char *)(pixels.data);
	// size_t size = sizeof(uint32_t);
	//
	// raw_data[(y * RES_X + x) * size] = 0;
	// raw_data[(y * RES_X + x) * size + 1] = blue;
	// raw_data[(y * RES_X + x) * size + 2] = green;
	// raw_data[(y * RES_X + x) * size + 3] = red;

	pixels.data[(y * RES_X + x)] = color;
}

void update_screen() {
	SDL_UpdateTexture(
			texture,
			NULL,
			pixels.data,
			RES_X * sizeof(uint32_t));

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, texture, NULL, NULL);
	SDL_RenderPresent(renderer);
}

unsigned int get_xres() {
	return RES_X;
}

unsigned int get_yres() {
	return RES_Y;
}

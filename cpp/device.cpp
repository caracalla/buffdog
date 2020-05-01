#include <stdlib.h>
#include <stdio.h>

#include <SDL2/SDL.h>

#include "device.h"
#include "util.h"


typedef struct {
	uint32_t data[RES_X * RES_Y];
	int bytes_per_line;
} pixeldata;


pixeldata pixels = {
	{0},
	RES_X * 4
};

SDL_Window *window;
SDL_Renderer *renderer;
SDL_Texture *texture;
SDL_Event event;

bool is_running = false;

key_input key_queue[64];
key_input last_key;
mouse_input mouse_motion;


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
			SDL_PIXELFORMAT_RGBX8888,
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

key_states_t key_states;

key_states_t get_key_states() {
	return key_states;
}

void process_input() {
	last_key = no_key;
	mouse_motion.x = 0;
	mouse_motion.y = 0;

	while (SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_QUIT:
				spit("quitting");
				is_running = false;
				break;

			case SDL_KEYDOWN:
				switch(event.key.keysym.scancode) {
					case SDL_SCANCODE_UP:
					case SDL_SCANCODE_W:
						last_key = up;
						key_states.up = true;
						break;

					case SDL_SCANCODE_DOWN:
					case SDL_SCANCODE_S:
						last_key = down;
						key_states.down = true;
						break;

					case SDL_SCANCODE_LEFT:
					case SDL_SCANCODE_A:
						last_key = left;
						key_states.left = true;
						break;

					case SDL_SCANCODE_RIGHT:
					case SDL_SCANCODE_D:
						last_key = right;
						key_states.right = true;
						break;

					case SDL_SCANCODE_X:
						last_key = x_key;
						break;

					case SDL_SCANCODE_Z:
						last_key = z_key;
						break;

					default:
						break;
				}

				break;

			case SDL_KEYUP:
				switch(event.key.keysym.scancode) {
					case SDL_SCANCODE_UP:
					case SDL_SCANCODE_W:
						key_states.up = false;
						break;

					case SDL_SCANCODE_DOWN:
					case SDL_SCANCODE_S:
						key_states.down = false;
						break;

					case SDL_SCANCODE_LEFT:
					case SDL_SCANCODE_A:
						key_states.left = false;
						break;

					case SDL_SCANCODE_RIGHT:
					case SDL_SCANCODE_D:
						key_states.right = false;
						break;

					default:
						break;
				}

				break;

			case SDL_MOUSEMOTION:
				mouse_motion.x = event.motion.xrel;
				mouse_motion.y = event.motion.yrel;
				break;

			default:
				// noop;
			break;
		}
	}
}

key_input get_next_key() {
	return last_key;
}

mouse_input get_mouse_motion() {
	return mouse_motion;
}

uint32_t color(double red, double green, double blue) {
	unsigned char red_value = 255 * red;
	unsigned char green_value = 255 * green;
	unsigned char blue_value = 255 * blue;
	return (red_value << 24) + (green_value << 16) + (blue_value << 8);
}

void clear_screen(int color) {
	for (int y = 0; y < RES_Y; ++y) {
		for (int x = 0; x < RES_X; ++x) {
			draw_pixel(x, y, color);
		}
	}
}

void draw_pixel(int x, int y, int color) {
	if (x < 0 || y < 0 || x >= RES_X || y >= RES_Y) {
		char message[1024];
		snprintf(
				message,
				sizeof(message),
				"trying to draw a pixel at x=%d and y=%d which is crazy illegal!!!\n",
				x,
				y);

		terminate(message);
	}

	// invert y since it starts at the top
	y = RES_Y - y - 1;
	size_t index = y * RES_X + x;

	pixels.data[index] = color;
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

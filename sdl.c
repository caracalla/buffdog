#include <SDL2/SDL.h>
#include <stdio.h>
#include <unistd.h>

#define RES_X 640
#define RES_Y 480

#define spit(x) printf("%s\n", x)

uint32_t pixels[RES_X * RES_Y];

int min(int v1, int v2) {
  return v1 >= v2 ? v2 : v1;
}

int max(int v1, int v2) {
  return v1 >= v2 ? v1 : v2;
}

// int interpolate(int v1, int v2, int t) {
//   return v1 + ((v2 - v1) * t) / FIVETWELVE;
// }

void setPixel(
    int x,
    int y,
    uint8_t red,
    uint8_t green,
    uint8_t blue) {
  uint32_t r = red & 0x000000FF;
  r = r << 24;

  uint32_t g = green & 0x000000FF;
  g = g << 16;

  uint32_t b = blue & 0x000000FF;
  b = b << 8;

  pixels[y * RES_X + x] = r | g | b;
}

void draw_pixel(int x, int y) {
  setPixel(x, y, 0x000000FF, 0x000000FF, 0x000000FF);
}



void draw_vertical_line(int x, int y1, int y2) {
	int y;

	if (y2 < y1) {
		y = y1;
		y1 = y2;
		y2 = y;
	}

	for (y = y1; y <= y2; y++) {
		draw_pixel(x, y);
	}
}

void draw_horizontal_line(int y, int x1, int x2) {
	int x;

	if (x2 < x1) {
		x = x1;
		x1 = x2;
		x2 = x;
	}

	for (x = x1; x <= x2; x++) {
		draw_pixel(x, y);
	}
}

void draw_line(int x1, int y1, int x2, int y2) {
	int dx = x2 - x1;
	int dy = y2 - y1;
	int decider;
	int increment_straight;
	int increment_diagonal;
	int x;
	int y;

	if (dx == 0) {
		draw_vertical_line(x1, y1, y2);
		return;
	}

	if (dy == 0) {
		draw_horizontal_line(y1, x1, x2);
		return;
	}

	if (dx < 0) {
		x = x1;
		x1 = x2;
		x2 = x;
		y = y1;
		y1 = y2;
		y2 = y;
		dx = -dx;
		dy = -dy;
	}

	x = x1;
	y = y1;

	draw_pixel(x, y);

	if (dy > 0) {
		if (dx > dy) {

			decider = 2 * dy - dx;
			increment_straight = 2 * dy;
			increment_diagonal = 2 * (dy - dx);

			while (x < x2) {
				if (decider <= 0) {
					decider += increment_straight;
					x++;
				} else {
					decider += increment_diagonal;
					x++;
					y++;
				}

				draw_pixel(x, y);
			}
		} else {
			decider = 2 * dx - dy;
			increment_straight = 2 * dx;
			increment_diagonal = 2 * (dx - dy);

			while (y < y2) {
				if (decider <= 0) {
					decider += increment_straight;
					y++;
				} else {
					decider += increment_diagonal;
					y++;
					x++;
				}

				draw_pixel(x, y);
			}
		}
	} else {
		dy = -dy;

		if (dx > dy) {
			decider = 2 * dy - dx;
			increment_straight = 2 * dy;
			increment_diagonal = 2 * (dy - dx);

			while (x < x2) {
				if (decider <= 0) {
					decider += increment_straight;
					x++;
				} else {
					decider += increment_diagonal;
					x++;
					y--;
				}

				draw_pixel(x, y);
			}
		} else {
			decider = 2 * dx - dy;
			increment_straight = 2 * dx;
			increment_diagonal = 2 * (dx - dy);

			while (y > y2) {
				if (decider <= 0) {
					decider += increment_straight;
					y--;
				} else {
					decider += increment_diagonal;
					y--;
					x++;
				}

				draw_pixel(x, y);
			}
		}
	}
}



void clearScreen() {
  for (uint16_t y = 0; y < RES_Y; ++y) {
    // int t = min(FIVETWELVE, ((y * FIVETWELVE) / RES_Y) * 4);
    //
    // uint32_t r = interpolate(200, 242, t);
    // uint32_t g = interpolate(102, 255, t);
    // uint32_t b = interpolate(255, 230, t);
    //
    // uint32_t color = (r << 24) | (g << 16 ) | (b << 8);

    for (uint16_t x = 0; x < RES_X; ++x) {
      setPixel(x, y, 0, 0, 0);
    }
  }
}

int clamp(int v, int v1, int v2) {
  return v >= v1 ? (v <= v2 ? v : v2) : v1;
}

void drawMouseLine(int xrel, int yrel) {
  int midx = RES_X / 2;
  int midy = RES_Y / 2;

  int endx = clamp(midx + xrel, 0, RES_X);
  int endy = clamp(midy + yrel, 0, RES_Y);

  draw_line(midx, midy, endx, endy);
}

int main() {
  SDL_Window *window = SDL_CreateWindow(
      "sdl demo",
      SDL_WINDOWPOS_UNDEFINED,
      SDL_WINDOWPOS_UNDEFINED,
      RES_X,
      RES_Y,
      SDL_WINDOW_SHOWN);
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
  SDL_Texture *textureSDL = SDL_CreateTexture(
      renderer,
      SDL_PIXELFORMAT_RGBX8888,
      SDL_TEXTUREACCESS_STATIC,
      RES_X,
      RES_Y);
  SDL_Surface *screenSurface = SDL_GetWindowSurface(window);
  SDL_Event event;

  // capture mouse
  SDL_SetRelativeMouseMode(SDL_TRUE);

  int running = 1;

  int xrel = 0;
  int yrel = 0;

  // wait for the window to be exposed
  {
    do {
      SDL_PollEvent(&event);

      if (
          event.type == SDL_WINDOWEVENT &&
          event.window.event == SDL_WINDOWEVENT_EXPOSED) {
        running = 1;
      }
    } while (!running);
  }

  // main loop
  while (running) {
    usleep(15000);

    xrel = 0;
    yrel = 0;

    while (SDL_PollEvent(&event)) {
      switch(event.type) {
        case SDL_WINDOWEVENT:
          printf("window event for id: %d, event: %d\n", event.window.windowID, event.window.event);
          break;

        case SDL_QUIT:
          spit("quitting");
          running = 0;
          break;

        case SDL_KEYDOWN:
          spit("key down event!");
          break;

        case SDL_KEYUP:
          spit("key up event!");
          break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
          spit("mouse button up/down event!");
          printf(
              "type: %d, which: %d, button: %d, state: %d, x: %d, y: %d\n",
              event.button.type,
              event.button.which,
              event.button.button,
              event.button.state,
              event.button.x,
              event.button.y);
          break;

        case SDL_MOUSEMOTION:
          xrel = event.motion.xrel;
          yrel = event.motion.yrel;
          break;

        default:
          // noop;
          break;
      }
    }

    clearScreen();
    drawMouseLine(xrel, yrel);

    // draw a line for the mouse motion

    SDL_UpdateTexture(
        textureSDL,
        NULL,
        pixels,
        RES_X * sizeof(uint32_t));

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, textureSDL, NULL, NULL);
    SDL_RenderPresent(renderer);
  }

  return 0;
}

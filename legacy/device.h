#ifndef BUFFDOG_DEVICE
#define BUFFDOG_DEVICE

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>


#define RES_X 640
#define RES_Y 480

#define spit(x) printf("%s\n", x)


typedef enum {
	no_key,
	up,
	down,
	left,
	right
} key_input;

typedef struct {
	int x;
	int y;
} mouse_input;


// This must be called first
// returns false on failure
bool set_up_device();

// Should be called before exiting the program
void close_device();

bool running();

// must be called, otherwise there is no way to quit
// call before rendering on each frame
void process_input();

// get the last key pressed
// TODO: handle multiple key presses, keyup and keydown events separately
key_input get_next_key();

mouse_input get_mouse_motion();

// converts color values into a pixel color value
// red, green, and blue must be between 0.0 and 1.0
uint32_t color(double red, double green, double blue);

// draw background
void clear_screen(int color);

// TODO: document the layout of the arguments
// (0, 0) is bottom left of the screen
void draw_pixel(int x, int y, int color);

// call after drawing the background and all desired pixels
void update_screen();

unsigned int get_xres();
unsigned int get_yres();



#endif

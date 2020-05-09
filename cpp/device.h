#ifndef BUFFDOG_DEVICE
#define BUFFDOG_DEVICE

#include <cstdint>
#include <cstdio>


#define RES_X 640
#define RES_Y 480

#define spit(x) printf("%s\n", x)

#define terminateFatal(message) device::selfDestruct(message, __LINE__, __FILE__)


typedef enum {
	no_key,
	up,
	down,
	left,
	right,
	x_key,
	z_key
} key_input;

typedef struct {
	int x;
	int y;
} mouse_input;

typedef struct {
	bool up;
	bool down;
	bool left;
	bool right;
	bool yup;
	bool ydown;
} key_states_t;

namespace device {
	double& zBufferAt(size_t x, size_t);

	key_states_t get_key_states();

	// This must be called first
	// returns false on failure
	bool setUp();

	// Should be called before exiting the program
	void tearDown();

	// if it all goes wrong somewhere
	void selfDestruct(const char *message, int line, const char* file);

	bool running();

	// must be called, otherwise there is no way to quit
	// call before rendering on each frame
	void processInput();

	// get the last key pressed
	// TODO: handle multiple key presses, keyup and keydown events separately
	key_input get_next_key();

	mouse_input getMouseMotion();

	// converts color values into a pixel color value
	// red, green, and blue must be between 0.0 and 1.0
	uint32_t color(double red, double green, double blue);

	// draw background
	void clearScreen(int color);

	// TODO: document the layout of the arguments
	// (0, 0) is bottom left of the screen
	void setPixel(int x, int y, int color);

	// call after drawing the background and all desired pixels
	void updateScreen();

	unsigned int getXRes();
	unsigned int getYRes();
}

#endif

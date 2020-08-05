#ifndef BUFFDOG_DEVICE
#define BUFFDOG_DEVICE

#include <cstdint>
#include <cstdio>


// the color of the "sky" (for now)
#define DEFAULT_BACKGROUND_COLOR device::color(0.1, 0.1, 0.1)

// easy debug printing
#define spit(message) printf("%s\n", message)

#define terminateFatal(message) device::selfDestruct(message, __LINE__, __FILE__)


typedef enum {
	no_key,
	up,
	down,
	left,
	right,
	x_key,  // also secretly the mousedown key
	z_key
} key_input;

// x and y are adjusted by a sensitivity factor, see processInput definition
typedef struct {
	double x;
	double y;
	int pos_x;
	int pos_y;
} mouse_input;

typedef struct {
	bool forward;
	bool reverse;
	bool left;
	bool right;
	bool yup;
	bool ydown;
	bool sprint;
	bool spew;
} key_states_t;

namespace device {
	// ***************************************************************************
	// device management
	// ***************************************************************************

	// This must be called first
	// returns false on failure
	bool setUp();

	// Should be called before exiting the program
	void tearDown();

	// if it all goes wrong somewhere, this lets us tearDown and exit safely
	// (I think)
	void selfDestruct(const char *message, int line, const char* file);

	bool running();

	// draw background
	void clearScreen(int color);

	// TODO: document the layout of the arguments
	// (0, 0) is bottom left of the screen
	void setPixel(int x, int y, int color);

	// call after drawing the background and all desired pixels
	void updateScreen();

	// ***************************************************************************
	// IO
	// ***************************************************************************

	// must be called, otherwise there is no way to quit
	// call before rendering on each frame
	void processInput();

	// get the last key pressed
	// TODO: handle multiple key presses, keyup and keydown events separately
	key_input get_next_key();

	mouse_input getMouseMotion();

	key_states_t get_key_states();

	// ***************************************************************************
	// utility and other
	// ***************************************************************************

	// converts color values into a pixel color value
	// red, green, and blue must be between 0.0 and 1.0
	uint32_t color(double red, double green, double blue);

	// get/set z buffer value for pixel
	double& zBufferAt(size_t x, size_t y);

	unsigned int getXRes();
	unsigned int getYRes();

	bool insideViewport(int x, int y);

	// [lower_bound, upper_bound)
	double randomDouble(double lower_bound, double upper_bound);
	int randomInt(int lower_bound, int upper_bound);
}

#endif

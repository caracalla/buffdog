#ifndef BUFFDOG_DEVICE
#define BUFFDOG_DEVICE

#include <cstdint>
#include <cstdio>

#include "input.h"


constexpr double kPi = 3.14159265358979323846264338327950288;
constexpr double kHalfPi = kPi / 2;
constexpr double kQuarterPi = kHalfPi / 2;
constexpr double kTau = kPi * 2;

// the color of the "sky" (for now)
#define DEFAULT_BACKGROUND_COLOR device::getColorValue(0.1, 0.1, 0.1)

// easy debug printing
#define spit(message) device::logOncePerSecond("%s\n", message)

#define terminateFatal(message) device::selfDestruct(message, __LINE__, __FILE__)


typedef enum {
	no_key,
	up,
	down,
	left,
	right,
	x_key,
	z_key,
	mouse_1
} key_input;


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
	// input
	// ***************************************************************************

	// must be called, otherwise there is no way to quit the program.
	// call before rendering on each frame
	void processInput();

	InputState* getInputState();

	// get the last key pressed (but not necessarily held down)
	// TODO: handle multiple key presses, keyup and keydown events separately
	key_input getNextKey();

	// ***************************************************************************
	// utility and other
	// ***************************************************************************

	// converts color values into a pixel color value
	// red, green, and blue must be between 0.0 and 1.0
	uint32_t getColorValue(double red, double green, double blue);

	// get/set z buffer value for pixel
	double& zBufferAt(size_t x, size_t y);

	unsigned int getXRes();
	unsigned int getYRes();

	bool insideViewport(int x, int y);

	// logs FPS once per second
	void logFPS();

	// helpers for per-second debug logging, to avoid spamming every frame
	void logOncePerSecond(const char* fmt, ...);
}

#endif

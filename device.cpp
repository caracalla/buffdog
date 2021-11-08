#include <chrono>
#include <cstdlib>
#include <cstdio>
#include <string>

#ifdef _MSC_VER
#include <SDL.h>
#else
#include <SDL2/SDL.h>
#endif

#include "device.h"
#include "util.h"


// TODO: make this stuff configurable
#define RES_X 640
#define RES_Y 480

#ifdef _MSC_VER
#define MOUSE_SENSITIVITY_FACTOR 100
#else
#define MOUSE_SENSITIVITY_FACTOR 1000
#endif


// logic to log the number of C++ memory allocations are happening behind the
// scenes
#define LOG_MEMORY_ALLOCATIONS 0
#if LOG_MEMORY_ALLOCATIONS
int new_calls = 0;
int delete_calls = 0;

void* operator new(size_t size) {
	new_calls += 1;
	return malloc(size);
}

void operator delete(void* mem) {
	delete_calls += 1;
	free(mem);
}

void printMemoryAllocations() {
	printf("new calls: %d, delete calls: %d\n", new_calls, delete_calls);
	new_calls = 0;
	delete_calls = 0;
}
#endif


// the framebuffer
typedef struct {
	uint32_t data[RES_X * RES_Y];
	int bytes_per_line;  // not actually used anywhere
} pixeldata;

pixeldata pixels = {
		{0},
		RES_X * 4};

double zbuffer[RES_X * RES_Y];

void zBufferReset() {
	memset(zbuffer, 0, RES_X * RES_Y * sizeof(double));
}

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Texture* texture;
SDL_Event event;

bool is_running = false;

// IO info storage
key_input key_queue[64];
key_input last_key;

InputState input_state;

// FPS logging stuff
int last_second_frame_count = 0;
std::chrono::steady_clock::time_point last_fps_print_time;


namespace device {
	// ***************************************************************************
	// device management
	// ***************************************************************************

	bool setUp() {
		util::initRandom();
		clearScreen(DEFAULT_BACKGROUND_COLOR);

		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
			SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
			return false;
		}

		SDL_Log("SDL Initialization successful");

		window = SDL_CreateWindow(
				"buffdog",
				SDL_WINDOWPOS_UNDEFINED,
				SDL_WINDOWPOS_UNDEFINED,
				RES_X,
				RES_Y,
				SDL_WINDOW_SHOWN);

		if (window == nullptr) {
			SDL_Log("Unable to create SDL window: %s", SDL_GetError());
			return false;
		}

		SDL_Log("Window created successfully");

		renderer = SDL_CreateRenderer(window, -1, 0);

		if (renderer == nullptr) {
			SDL_Log("Unable to create SDL renderer: %s", SDL_GetError());
			return false;
		}

		SDL_Log("Renderer created successfully");

		texture = SDL_CreateTexture(
				renderer,
				SDL_PIXELFORMAT_RGBX8888,
				SDL_TEXTUREACCESS_STATIC,
				RES_X,
				RES_Y);

		if (texture == nullptr) {
			SDL_Log("Unable to create SDL texture: %s", SDL_GetError());
			return false;
		}

		SDL_Log("Texture created successfully");

		// capture mouse
		SDL_SetRelativeMouseMode(SDL_TRUE);

		SDL_Log("Grabbed mouse successfully");

		// wait for the window to be exposed
		{
			auto start_time = std::chrono::steady_clock::now();

			do {
				SDL_PollEvent(&event);

				if (
						event.type == SDL_WINDOWEVENT &&
						event.window.event == SDL_WINDOWEVENT_SHOWN) {
					is_running = true;
				}

				auto time_spent_waiting =
						std::chrono::duration_cast<std::chrono::milliseconds>(
								std::chrono::steady_clock::now() - start_time);

				SDL_Log(
						"time spent waiting for window to appear: %lld ms",
						time_spent_waiting.count());

			} while (!is_running);
		}

		SDL_Log("Device setup successful!");

		last_fps_print_time = std::chrono::steady_clock::now();

		return true;
	}

	void tearDown() {
		SDL_DestroyWindow(window);
		SDL_DestroyRenderer(renderer);
		SDL_DestroyTexture(texture);
		SDL_Quit();
	}

	void selfDestruct(char const* message, int line_number, const char* file_name) {
		printf("[%s:%d] %s\n", file_name, line_number, message);
		device::tearDown();
		exit(1);
	}

	bool running() {
		return is_running;
	}

	void clearScreen(int color) {
		zBufferReset();

		for (int y = 0; y < RES_Y; ++y) {
			for (int x = 0; x < RES_X; ++x) {
				setPixel(x, y, color);
			}
		}
	}

#define DEBUG_CHECK_OOB 0
#define DEBUG_FATAL_OOB 0
#define DEBUG_LOG_OOB 0

	void setPixel(int x, int y, int color) {
#if DEBUG_CHECK_OOB
		if (x < 0 || y < 0 || x >= RES_X || y >= RES_Y) {
			char message[1024];
			snprintf(
					message,
					sizeof(message),
					"trying to draw a pixel at x=%d and y=%d which is crazy illegal!!! resolution is %d x %d\n",
					x,
					y,
					RES_X,
					RES_Y);

	#if DEBUG_FATAL_OOB
			terminateFatal(message);
	#elif DEBUG_LOG_OOB
			printf("%s", message);
			return;
	#else
			return;
#endif
		}
#endif

		// invert y since it starts at the top
		y = RES_Y - y - 1;
		size_t index = y * RES_X + x;

		pixels.data[index] = color;
	}

	void updateScreen() {
		SDL_UpdateTexture(
				texture,
				nullptr,
				pixels.data,
				RES_X * sizeof(uint32_t));

		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);

		logFPS();

#if LOG_MEMORY_ALLOCATIONS
		// will print once per frame, instead of per second like FPS
		printMemoryAllocations();
#endif
	}

	// ***************************************************************************
	// IO
	// ***************************************************************************

	void processInput() {
		last_key = no_key;

		input_state.mouse.motion_x = 0;
		input_state.mouse.motion_y = 0;

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
							input_state.buttons.forward = true;
							break;

						case SDL_SCANCODE_DOWN:
						case SDL_SCANCODE_S:
							last_key = down;
							input_state.buttons.reverse = true;
							break;

						case SDL_SCANCODE_LEFT:
						case SDL_SCANCODE_A:
							last_key = left;
							input_state.buttons.left = true;
							break;

						case SDL_SCANCODE_RIGHT:
						case SDL_SCANCODE_D:
							last_key = right;
							input_state.buttons.right = true;
							break;

						case SDL_SCANCODE_Q:
							input_state.buttons.rise = true;
							break;

						case SDL_SCANCODE_E:
							input_state.buttons.descend = true;
							break;

						case SDL_SCANCODE_LSHIFT:
							input_state.buttons.sprint = true;
							break;

						case SDL_SCANCODE_T:
							break;

						case SDL_SCANCODE_X:
							last_key = x_key;
							break;

						case SDL_SCANCODE_Z:
							last_key = z_key;
							break;

						case SDL_SCANCODE_ESCAPE:
							spit("quitting");
							is_running = false;
							break;

						default:
							break;
					}

					break;

				case SDL_KEYUP:
					switch(event.key.keysym.scancode) {
						case SDL_SCANCODE_UP:
						case SDL_SCANCODE_W:
							input_state.buttons.forward = false;
							break;

						case SDL_SCANCODE_DOWN:
						case SDL_SCANCODE_S:
							input_state.buttons.reverse = false;
							break;

						case SDL_SCANCODE_LEFT:
						case SDL_SCANCODE_A:
							input_state.buttons.left = false;
							break;

						case SDL_SCANCODE_RIGHT:
						case SDL_SCANCODE_D:
							input_state.buttons.right = false;
							break;

						case SDL_SCANCODE_Q:
							input_state.buttons.rise = false;
							break;

						case SDL_SCANCODE_E:
							input_state.buttons.descend = false;
							break;

						case SDL_SCANCODE_LSHIFT:
							input_state.buttons.sprint = false;
							break;

						case SDL_SCANCODE_T:
							break;

						default:
							break;
					}

					break;

				case SDL_MOUSEBUTTONDOWN:
					last_key = mouse_1;
					input_state.buttons.action1 = true;
					break;

				case SDL_MOUSEBUTTONUP:
					input_state.buttons.action1 = false;
					break;

				case SDL_MOUSEMOTION:
					input_state.mouse.motion_x = (double)event.motion.xrel / MOUSE_SENSITIVITY_FACTOR;
					input_state.mouse.motion_y = (double)event.motion.yrel / MOUSE_SENSITIVITY_FACTOR;

					input_state.mouse.pos_x = event.motion.x;
					input_state.mouse.pos_y = event.motion.y;
					break;

				default:
				break;
			}
		}
	}

	InputState* getInputState() {
		return &input_state;
	}

	key_input getNextKey() {
		return last_key;
	}

	// ***************************************************************************
	// utility and other
	// ***************************************************************************

	uint32_t getColorValue(double red, double green, double blue) {
		#define clampToZero(value) (value >= 0.0 ? value : 0.0)

		unsigned char red_value = 255 * clampToZero(red);
		unsigned char green_value = 255 * clampToZero(green);
		unsigned char blue_value = 255 * clampToZero(blue);
		return (red_value << 24) + (green_value << 16) + (blue_value << 8);
	}

	double& zBufferAt(size_t x, size_t y) {
		size_t index = y * RES_X + x;

		return zbuffer[index];
	}

	unsigned int getXRes() {
		return RES_X;
	}

	unsigned int getYRes() {
		return RES_Y;
	}

	// this is here because I was using this with drawPoint and the mouse, and
	// drawPoint draws a few pixels around the cursor, causing issues when the
	// cursor is near the edge of the viewport
	#define VIEWPORT_BUFFER 5

	bool insideViewport(int x, int y) {
		return
				x > VIEWPORT_BUFFER &&
				y > VIEWPORT_BUFFER &&
				x < RES_X - VIEWPORT_BUFFER &&
				y < RES_Y - VIEWPORT_BUFFER;
	}

	void logFPS() {
		last_second_frame_count++;
		auto now = std::chrono::steady_clock::now();
		auto time_since_last_print = now - last_fps_print_time;

		if (time_since_last_print >= std::chrono::seconds(1)) {
			printf("FPS: %d\n", last_second_frame_count);

			last_fps_print_time = now;
			last_second_frame_count = 0;
		}
	}
	void logOncePerSecond(const char* fmt, ...) {
		// frame count is always reset to 0 when one second has elapsed
		if (last_second_frame_count == 0) {
			va_list args;
			va_start(args, fmt);
			vprintf(fmt, args);
			va_end(args);
		}
	}

}

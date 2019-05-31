#ifndef BUFFDOG_DEVICE
#define BUFFDOG_DEVICE

// This must be called first
int set_up_device();

// Should be called before exiting the program
void close_fbfd();

// white for 16 bit pixels: 0xFFFF
// white for 32 bit pixels: 0xFFFFFF
// red, green, and blue must be between 0.0 and 1.0
int color(double red, double green, double blue);


// TODO: document the layout of the arguments
// (0, 0) is bottom left of the screen
void draw_pixel(int x, int y, int color);

void clear_screen();

// Getters for screen info
unsigned int get_xres();
unsigned int get_yres();

void print_fb_info();

#endif

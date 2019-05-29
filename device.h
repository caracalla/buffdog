// This must be called first
int set_up_device();

// Should be called before exiting the program
void close_fbfd();

// TODO: document the layout of the arguments
void draw_pixel(int x, int y, int color);

void clear_screen();

// Getters for screen info
unsigned int get_xres();
unsigned int get_yres();

void print_fb_info();

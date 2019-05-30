#include <linux/fb.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <unistd.h> // close and usleep
#include "device.h"


const char *FB_NAME = "/dev/fb0";
struct fb_fix_screeninfo fixed_info;
struct fb_var_screeninfo variable_info;
int m_FBFD;
// pixels are 16 bits, so must be a short
// although I guess this isn't always true
// short int *fb_map_pointer;
int *fb_map_pointer;

int set_up_device() {
	// I got this from the internet, so I still need to figure out exactly what it's doing

	int iFrameBufferSize;
	/* Open the framebuffer device in read write */
	m_FBFD = open(FB_NAME, O_RDWR); // framebuffer file descriptor
	if (m_FBFD < 0) {
		printf("Unable to open %s.\n", FB_NAME);
		return 1;
	}

	/* Do Ioctl. Retrieve fixed screen info. */
	if (ioctl(m_FBFD, FBIOGET_FSCREENINFO, &fixed_info) < 0) {
		printf("get fixed screen info failed: %s\n", strerror(errno));
		close(m_FBFD);
		return 1;
	}

	/* Do Ioctl. Get the variable screen info. */
	if (ioctl(m_FBFD, FBIOGET_VSCREENINFO, &variable_info) < 0) {
		printf("Unable to retrieve variable screen info: %s\n", strerror(errno));
		close(m_FBFD);
		return 1;
	}

	/* Calculate the size to mmap */
	iFrameBufferSize = fixed_info.line_length * variable_info.yres;

	/* Now mmap the framebuffer. */
	void *fb_map = mmap(NULL, iFrameBufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_FBFD,0);
	if (fb_map == NULL) {
		printf("mmap failed:\n");
		close(m_FBFD);
		return 1;
	}

	// fb_map_pointer = (short int *) fb_map;
	fb_map_pointer = (int *) fb_map;

	return 0;
}

void close_fbfd() {
	close(m_FBFD);
}

int color(double red, double green, double blue) {
	unsigned char red_value = 255 * red;
	unsigned char green_value = 255 * green;
	unsigned char blue_value = 255 * blue;
	return (red_value << 16) + (green_value << 8) + blue_value;
}

void draw_pixel(int x, int y, int color) {
	// start the y coordinate axis at the bottom of the screen
	fb_map_pointer[x + ((variable_info.yres - 1) - y) * variable_info.xres] = color;
}

void clear_screen() {
	memset(fb_map_pointer, 0, (variable_info.yres * variable_info.xres) * 4);
}

unsigned int get_xres() {
	return variable_info.xres;
}

unsigned int get_yres() {
	return variable_info.yres;
}

void print_fb_info() {
	printf("Printing fixed info\n");
	printf("length of fb memory: %d\n", fixed_info.smem_len);
	printf("length of a line in bytes: %d\n", fixed_info.line_length);
	printf("FB Type: %d\n", fixed_info.type);
	printf("FB Visual: %d\n", fixed_info.visual);

	printf("Printing variable info\n");
	printf("xres: %d\n", variable_info.xres);
	printf("yres: %d\n", variable_info.yres);
	printf("xres_virtual: %d\n", variable_info.xres_virtual);
	printf("yres_virtual: %d\n", variable_info.yres_virtual);
	printf("xoffset: %d\n", variable_info.xoffset);
	printf("yoffset: %d\n", variable_info.yoffset);
	printf("bits_per_pixel: %d\n", variable_info.bits_per_pixel);
	printf("red bitfied length: %d\n", variable_info.red.length);
	printf("green bitfied length: %d\n", variable_info.green.length);
	printf("blue bitfied length: %d\n", variable_info.blue.length);
	printf("\n");
	printf("\n");
	printf("\n");
	printf("\n");
}

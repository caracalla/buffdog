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
void*   fb_map;
struct  fb_fix_screeninfo m_FixInfo;
struct  fb_var_screeninfo m_VarInfo;
int     m_FBFD;
// pixels are 16 bits, so must be a short
short int     *fb_map_pointer;

void set_pixel(int x, int y, int color) {
  // start the y coordinate axis at the bottom of the screen
  fb_map_pointer[x + ((m_VarInfo.yres - 1) - y) * m_VarInfo.xres] = color;
}

void clear_screen() {
  memset(fb_map_pointer, 0, (m_VarInfo.yres * m_VarInfo.xres) * 2);
}

int set_up_device() {
  int iFrameBufferSize;
  /* Open the framebuffer device in read write */
  m_FBFD = open(FB_NAME, O_RDWR); // framebuffer file descriptor
  if (m_FBFD < 0) {
  	printf("Unable to open %s.\n", FB_NAME);
  	return 1;
  }

  /* Do Ioctl. Retrieve fixed screen info. */
  if (ioctl(m_FBFD, FBIOGET_FSCREENINFO, &m_FixInfo) < 0) {
  	printf("get fixed screen info failed: %s\n", strerror(errno));
  	close(m_FBFD);
  	return 1;
  }

  /* Do Ioctl. Get the variable screen info. */
  if (ioctl(m_FBFD, FBIOGET_VSCREENINFO, &m_VarInfo) < 0) {
  	printf("Unable to retrieve variable screen info: %s\n", strerror(errno));
  	close(m_FBFD);
  	return 1;
  }

  /* Calculate the size to mmap */
  iFrameBufferSize = m_FixInfo.line_length * m_VarInfo.yres;

  /* Now mmap the framebuffer. */
  fb_map = mmap(NULL, iFrameBufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, m_FBFD,0);
  if (fb_map == NULL) {
  	printf("mmap failed:\n");
  	close(m_FBFD);
  	return 1;
  }

  fb_map_pointer = (short int *) fb_map;

  return 0;
}

void close_fbfd() {
  close(m_FBFD);
}

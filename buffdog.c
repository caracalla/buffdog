#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include "device.h"
// usleep
#include <unistd.h>

int main () {
  int setup_status = set_up_device();

  if (setup_status != 0) {
    return setup_status;
  }

  clear_screen();

  //draw a small cross in the center of the screen
  set_pixel(320, 240, 0xFFFF);
  set_pixel(320, 241, 0xFFFF);
  set_pixel(320, 242, 0xFFFF);
  set_pixel(320, 239, 0xFFFF);
  set_pixel(320, 238, 0xFFFF);
  set_pixel(321, 240, 0xFFFF);
  set_pixel(322, 240, 0xFFFF);
  set_pixel(319, 240, 0xFFFF);
  set_pixel(318, 240, 0xFFFF);

  // draw a diagonal line from the bottom left of the screen
  set_pixel(0, 0, 0xFFFF);
  set_pixel(1, 1, 0xFFFF);
  set_pixel(2, 2, 0xFFFF);
  set_pixel(3, 3, 0xFFFF);
  set_pixel(4, 4, 0xFFFF);
  set_pixel(5, 5, 0xFFFF);
  set_pixel(6, 6, 0xFFFF);
  set_pixel(7, 7, 0xFFFF);

  close_fbfd();

  return 0;
}

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include "device.h"
// usleep
#include <unistd.h>

#define ROUND(a) ((int)(a + 0.5))

void draw_line_DDA(int x1, int y1, int x2, int y2, int color) {
  int dx = x2 - x1;
  int dy = y2 - y1;
  int steps, i;
  float x_increment, y_increment;
  float x = x1;
  float y = y1;

  if (abs(dx) > abs(dy)) {
    steps = abs(dx);
  } else {
    steps = abs(dy);
  }

  x_increment = dx / ((float) steps);
  y_increment = dy / ((float) steps);

  set_pixel(ROUND(x), ROUND(y), color);

  for (i = 0; i < steps; i++) {
    x += x_increment;
    y += y_increment;
    set_pixel(ROUND(x), ROUND(y), color);
  }
}

int main () {
  int setup_status = set_up_device();
  float num = 0;
  int x, y;

  if (setup_status != 0) {
    return setup_status;
  }

  while (1) {
    usleep(1000);
    clear_screen();
    num += 0.01;
    x = ROUND(sin(num) * 50) + 320;
    y = ROUND(cos(num) * 50) + 240;
    draw_line_DDA(320, 240, x, y, 0xFFFF);
  }

  close_fbfd();

  return 0;
}

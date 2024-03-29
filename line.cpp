#include <cstdlib>
#include "line.h"
#include "device.h"


void drawVerticalLine(int x, int y1, int y2, int color) {
	int y;

	if (y2 < y1) {
		y = y1;
		y1 = y2;
		y2 = y;
	}

	for (y = y1; y <= y2; y++) {
		device::setPixel(x, y, color);
	}
}

void drawHorizontalLine(int y, int x1, int x2, int color) {
	int x;

	if (x2 < x1) {
		x = x1;
		x1 = x2;
		x2 = x;
	}

	for (x = x1; x <= x2; x++) {
		device::setPixel(x, y, color);
	}
}

// midpoint algorithm
void drawLineImpl(int x1, int y1, int x2, int y2, int color) {
	int dx = x2 - x1;
	int dy = y2 - y1;
	int decider;
	int increment_straight;
	int increment_diagonal;
	int x;
	int y;

	if (dx == 0) {
		drawVerticalLine(x1, y1, y2, color);
		return;
	}

	if (dy == 0) {
		drawHorizontalLine(y1, x1, x2, color);
		return;
	}

	if (dx < 0) {
		x = x1;
		x1 = x2;
		x2 = x;
		y = y1;
		y1 = y2;
		y2 = y;
		dx = -dx;
		dy = -dy;
	}

	x = x1;
	y = y1;

	device::setPixel(x, y, color);

	if (dy > 0) {
		if (dx > dy) {

			decider = 2 * dy - dx;
			increment_straight = 2 * dy;
			increment_diagonal = 2 * (dy - dx);

			while (x < x2) {
				if (decider <= 0) {
					decider += increment_straight;
					x++;
				} else {
					decider += increment_diagonal;
					x++;
					y++;
				}

				device::setPixel(x, y, color);
			}
		} else {
			decider = 2 * dx - dy;
			increment_straight = 2 * dx;
			increment_diagonal = 2 * (dx - dy);

			while (y < y2) {
				if (decider <= 0) {
					decider += increment_straight;
					y++;
				} else {
					decider += increment_diagonal;
					y++;
					x++;
				}

				device::setPixel(x, y, color);
			}
		}
	} else {
		dy = -dy;

		if (dx > dy) {
			decider = 2 * dy - dx;
			increment_straight = 2 * dy;
			increment_diagonal = 2 * (dy - dx);

			while (x < x2) {
				if (decider <= 0) {
					decider += increment_straight;
					x++;
				} else {
					decider += increment_diagonal;
					x++;
					y--;
				}

				device::setPixel(x, y, color);
			}
		} else {
			decider = 2 * dx - dy;
			increment_straight = 2 * dx;
			increment_diagonal = 2 * (dx - dy);

			while (y > y2) {
				if (decider <= 0) {
					decider += increment_straight;
					y--;
				} else {
					decider += increment_diagonal;
					y--;
					x++;
				}

				device::setPixel(x, y, color);
			}
		}
	}
}

void drawLine(Point start, Point end, int color) {
	drawLineImpl(start.x, start.y, end.x, end.y, color);
}

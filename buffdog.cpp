// usleep
#define _BSD_SOURCE
#include <unistd.h>

#include <cctype>
#include <chrono>
#define _USE_MATH_DEFINES // M_PI et al
#include <cmath>
#include <cstdlib>

#include "circle.h"
#include "device.h"
#include "line.h"
#include "matrix.h"
#include "vector.h"


int colorFromVector(Vector vec) {
	return device::getColorValue(vec.x, vec.y, vec.z);
}

void drawPointLine(Point start, Point end, Vector color) {
  int color_value = colorFromVector(color);

  drawLine(start.x, start.y, end.x, end.y, color_value);
}


struct Line {
  Point start;
  Point end;
  Vector color;

  void draw() {
    drawPointLine(start, end, color);
  }

  double length() {
    return this->start.distanceFrom(this->end);
  }
};


struct Container {
  Point bl;
  Point br;
  Point tl;
  Point tr;
  Vector color;

  void draw() {
    drawPointLine(bl, br, color);
    drawPointLine(br, tr, color);
    drawPointLine(tr, tl, color);
    drawPointLine(tl, bl, color);
  }
};


#define MAX_VELOCITY 10
#define VELOCITY_INCREMENT (MAX_VELOCITY * 0.01)

struct Entity {
  double radius;
  Point position;
  Vector accurate_position;
  Vector velocity;
  Vector color;

  Entity(Point position, Vector color) {
    this->radius = 5;
    this->position = position;
    this->accurate_position.x = position.x;
    this->accurate_position.y = position.y;
    this->velocity = Vector{0.707, -0.707};
    this->color = color;
  }

  void update(std::chrono::microseconds frame_duration) {
    key_states_t key_states = device::getKeyStates();

    if (key_states.forward) {
			this->velocity.y += VELOCITY_INCREMENT;
		}
		if (key_states.reverse) {
			this->velocity.y -= VELOCITY_INCREMENT;
		}
		if (key_states.left) {
			this->velocity.x -= VELOCITY_INCREMENT;
		}
		if (key_states.right) {
			this->velocity.x += VELOCITY_INCREMENT;
		}

    if (this->velocity.length() > MAX_VELOCITY) {
      this->velocity = this->velocity.unit().scalarMultiply(MAX_VELOCITY);
    }

    this->accurate_position.x += this->velocity.x;
    this->accurate_position.y += this->velocity.y;

    if (this->accurate_position.y - this->radius < 0) {
      this->velocity.y = -this->velocity.y;
      this->accurate_position.y = this->accurate_position.y + radius;
    }
    if (this->accurate_position.y + this->radius > device::getYRes()) {
      this->velocity.y = -this->velocity.y;
      this->accurate_position.y = this->accurate_position.y - radius;
    }
    if (this->accurate_position.x - this->radius < 0) {
      this->velocity.x = -this->velocity.x;
      this->accurate_position.x = this->accurate_position.x + radius;
    }
    if (this->accurate_position.x + this->radius > device::getXRes()) {
      this->velocity.x = -this->velocity.x;
      this->accurate_position.x = this->accurate_position.x - radius;
    }

    this->position = Point{(int)this->accurate_position.x, (int)this->accurate_position.y};
  }

  void draw() {
    Circle item{this->position, this->radius, color};
    item.draw();
  }
};


#define EPSILON 0.5

bool linePointCollision(Line line, Point point) {
  double line_length = line.length();
  double d1 = point.distanceFrom(line.start);
  double d2 = point.distanceFrom(line.end);

  return (d1 + d2 >= line_length - EPSILON && d1 + d2 <= line_length + EPSILON);
}


int main(int argc, char** argv) {
	if (!device::setUp()) {
		return 1;
	}

	auto last_frame_time = std::chrono::steady_clock::now();

  int buffer = 50;
  int max_x = device::getXRes() - buffer;
  int max_y = device::getYRes() - buffer;

  Container container{
    Point{buffer, buffer},
    Point{max_x, buffer},
    Point{buffer, max_y},
    Point{max_x, max_y},
    Vector{1.0, 0.0, 0.0}
  };

  Entity moving_circle = Entity(Point{100, 150}, Vector{0.0, 1.0, 0.0});

  Line line{
    // Point{buffer, buffer},
    // Point{max_x, max_y},
    Point{320, 240},
    Point{600, 400},
    Vector{1.0, 0.0, 0.0}
  };

	while (device::running()) {
    device::clearScreen(0);

		// draw the level and models
    moving_circle.draw();
    // container.draw();
    line.draw();

		// paint the screen
		device::updateScreen();

		// grab keyboard and mouse input
		device::processInput();

		// update player, models, and level by one time step
		auto now = std::chrono::steady_clock::now();
		auto frame_duration =
				std::chrono::duration_cast<std::chrono::microseconds>(now - last_frame_time);
		last_frame_time = now;

    if (linePointCollision(line, moving_circle.position)) {
      double dx = line.end.x - line.start.x;
      double dy = line.end.y - line.start.y;
      Vector circle_position{moving_circle.accurate_position.x, moving_circle.accurate_position.y};
      Vector line_normal = Vector{-dy, dx}.unit();

      if (line_normal.dotProduct(circle_position) < 0) {
        line_normal = Vector{-line_normal.x, -line_normal.y};
      }

      double factor = moving_circle.velocity.dotProduct(line_normal);
      Vector orthogonal_component = line_normal.scalarMultiply(factor);
      Vector new_velocity = moving_circle.velocity.subtract(orthogonal_component.scalarMultiply(2));
      moving_circle.velocity = new_velocity;
    }

    moving_circle.update(frame_duration);
	}

	device::tearDown();

	return 0;
}

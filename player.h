#ifndef BUFFDOG_PLAYER
#define BUFFDOG_PLAYER

// #include "model.h"
#include "vector.h"

// TODO: move this mouse logic to device
#define INCREMENT 0.005
#define MOUSE_SENSITIVITY_FACTOR 1000
// TODO: tie velocity to FPS
#define MAX_VELOCITY 0.0625


struct Player {
	double height = 1.7;
	double eye_height = 1.5; // distance from base
	Vector position; // of base
	Vector rotation;

	Vector weapon_position; // relative to base

	double velocity = 0;

	Model bullet;
	Model explosion;

	void move() {
		// handle mouse movement
		mouse_input mouse_motion = device::getMouseMotion();

		this->rotation.x += (double)mouse_motion.y / MOUSE_SENSITIVITY_FACTOR;
		this->rotation.y += (double)mouse_motion.x / MOUSE_SENSITIVITY_FACTOR;

		// handle key inputs
		key_states_t key_states = device::get_key_states();

		Vector translation = {0, 0, 0, 0};

		if (key_states.up ||
				key_states.down ||
				key_states.left ||
				key_states.right ||
				key_states.yup ||
				key_states.ydown) {
			double max_velocity = MAX_VELOCITY;

			if (key_states.sprint) {
				max_velocity *= 5;
			}

			if (this->velocity <= max_velocity) {
				this->velocity += INCREMENT;
			} else {
				this->velocity -= INCREMENT;
			}
		} else {
			this->velocity -= INCREMENT;

			if (this->velocity < 0) {
				this->velocity = 0;
			}
		}

		if (key_states.up) {
			translation.z -= 1;
		}
		if (key_states.down) {
			translation.z += 1;
		}
		if (key_states.left) {
			translation.x -= 1;
		}
		if (key_states.right) {
			translation.x += 1;
		}
		if (key_states.yup) {
			translation.y += 1;
		}
		if (key_states.ydown) {
			translation.y -= 1;
		}

		Vector movement = translation.unit().scalarMultiply(this->velocity);

		// the direction of motion is determined only by the rotation about the y axis
		Matrix rotationAboutY = Matrix::makeRotationMatrix(
				Vector::direction(0, this->rotation.y, 0));
		this->position = this->position.add(rotationAboutY.multiplyVector(movement));
		this->weapon_position = this->position.add(Vector::direction(0, 1.2, 0));
	}
};

#endif

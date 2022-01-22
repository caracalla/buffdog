#ifndef BUFFDOG_PLAYER
#define BUFFDOG_PLAYER

#include <chrono>

#include "../input.h"
#include "../vector.h"

#include "entity.h"
#include "model.h"


struct Weapon : public Entity {
	// the position of the weapon in the player model's local space
	// for example, a rocket launcher is shoulder mounted, but a rifle could be
	// shoulder fired, and a minigun might be hip fired
	Vector player_local_position;

	Model bullet;
	Model explosion;

	// technically these are half-lengths
	double x_len = 0.15;
	double y_len = 0.15;
	double z_len = 0.5;

	std::chrono::microseconds cooldown_remaining = std::chrono::microseconds(0);

	Vector bulletDirection();
	void fireBullet();

	Model buildModel() {
		Vector start = Vector::point(-this->x_len, -this->y_len, -this->z_len);
		Vector end = Vector::point(this->x_len, this->y_len, this->z_len);

		return Model::buildHexahedron(start, end);
	}
};


struct Player : public Entity {
	double height = 1.7;
	double width = 0.25; // distance from y axis along x or z in either direction
	double eye_height = 1.5; // distance from base of feet

	Weapon weapon;

	void move(std::chrono::microseconds frame_duration, InputState* input_state);

	Model buildModel() {
		// the center of the model's bottom plane is its "origin"
		Vector start = Vector::point(-this->width, 0.0, -this->width);
		Vector end = Vector::point(this->width, this->height, this->width);

		return Model::buildHexahedron(start, end);
	}
};

#endif

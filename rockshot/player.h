#ifndef BUFFDOG_PLAYER
#define BUFFDOG_PLAYER

#include <chrono>

#include "../input.h"
#include "../vector.h"

#include "entity.h"
#include "model.h"


struct Player : public Entity {
	double height = 1.7;
	double width = 0.25; // distance from y axis along x or z in either direction
	double eye_height = 1.5; // distance from base

	Vector weapon_position; // relative to base

	Model bullet_model;
	Model explosion_model;

	std::chrono::microseconds weapon_cooldown_remaining = std::chrono::microseconds(0);

	void move(std::chrono::microseconds frame_duration, InputState* input_state);

	Vector bulletDirection();

	void fireBullet();
	void fireSpewBullet();

	Entity makeDud(Model* model, Vector position) {
		Entity dud;
		dud.model = model;
		dud.scale = 0.5;
		dud.position = position;
		dud.mass = 0.0; // shouldn't be affected by gravity

		return dud;
	}

	Model buildModel() {
		Vector start = Vector::point(-this->width, 0, -this->width);
		Vector end = Vector::point(this->width, this->height, this->width);

		return buildHexahedron(start, end);
	}
};

#endif

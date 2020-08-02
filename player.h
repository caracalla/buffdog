#ifndef BUFFDOG_PLAYER
#define BUFFDOG_PLAYER

#include <chrono>

#include "device.h"
#include "entity.h"
#include "model.h"
#include "vector.h"


struct Player : public Entity {
	double height = 1.7;
	double eye_height = 1.5; // distance from base

	Vector weapon_position; // relative to base

	double velocity_value = 0;

	Model bullet_model;
	Model explosion_model;

	void move();

	Vector bulletDirection();

	void fireBullet(std::chrono::microseconds frame_duration);
	void fireSpewBullet(std::chrono::microseconds frame_duration);
};

#endif

#ifndef BUFFDOG_ENTITY
#define BUFFDOG_ENTITY

#include <chrono>
#include <functional>

#include "model.h"


struct Entity;
struct Scene;

typedef std::function<void(Entity*)> EntityAction;

// this is basically just a wrapper around models for now
struct Entity {
	Model* model;
	double scale;
	Vector position = Vector::point(0, 0, 0);
	Vector rotation = Vector::direction(0, 0, 0); // represented as radians around each axis

	// physics
	Vector velocity = Vector::direction(0, 0, 0);
	Vector force = Vector::direction(0, 0, 0);
	double mass = 0.0; // in kilograms?

	// action represents something that happens to the entity on each step
	EntityAction action;
	bool has_action = false;

	// if false, it's no longer part of the scene, it's not rendered and physics
	// are not applied
	bool active = true;
	Scene* scene;

	void applyPhysics(std::chrono::microseconds frame_duration) {
		auto dt = frame_duration.count();

		if (this->mass > 0) {
			this->applyForce(Vector::direction(0, -9.8 * this->mass, 0));

			// apply forces to velocity
			Vector delta_v = this->force.scalarMultiply(1 / this->mass).scalarMultiply(dt);
			this->velocity = this->velocity.add(delta_v);
		}

		// apply velocity to position
		Vector delta_p = this->velocity.scalarMultiply(dt);
		this->position = this->position.add(delta_p);

		// clear out forces
		this->force = Vector{0, 0, 0};
	}

	#define MICROSECONDS 1000000.0

	// parameter value should be in meters per second
	void applyForce(Vector new_force) {
		new_force = new_force.scalarMultiply(1 / (MICROSECONDS * MICROSECONDS));

		this->force = this->force.add(new_force);
	}
};

#endif

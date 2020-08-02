#ifndef BUFFDOG_ENTITY
#define BUFFDOG_ENTITY

#include <functional>

#include "model.h"


struct Entity;
struct Scene;

typedef std::function<void(Entity*)> EntityAction;

// this is basically just a wrapper around models for now
struct Entity {
	Model* model;
	double scale;
	Vector position;
	Vector rotation; // represented as radians around each axis

	// physics
	Vector velocity;
	Vector force;
	double mass; // in kilograms?

	// action represents something that happens to the entity on each step
	EntityAction action;
	bool has_action = false;
	bool visible = true;
	Scene* scene;

	void applyPhysics(std::chrono::microseconds frame_duration) {
		auto dt = frame_duration.count();

		// apply forces to velocity
		auto delta_v = this->force.scalarMultiply(1 / this->mass).scalarMultiply(dt);
		this->velocity = this->velocity.add(delta_v);

		// apply velocity to position
		auto delta_p = this->velocity.scalarMultiply(dt);
		this->position = this->position.add(delta_p);

		// clear out forces
		this->force = Vector{0, 0, 0};
	}
};

#endif

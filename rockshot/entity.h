#ifndef BUFFDOG_ENTITY
#define BUFFDOG_ENTITY

#include <chrono>
#include <functional>

#include "../vector.h"

#include "model.h"
#include "collision.h"


#define MICROSECONDS 1000000.0


struct Entity;
struct Scene;


typedef std::function<void(Entity*, std::chrono::microseconds frame_duration)> EntityAction;


struct Entity {
	Model* model;
	double scale = 1.0; // dear god this ruined an entire morning
	Vector start_pos = Vector::origin();
	Vector position = Vector::origin();
	Vector rotation = Vector::direction(0, 0, 0); // represented as radians around each axis

	double velocity_value = 0; // in meters per microsecond (really just speed)
	double y_speed = 0;

	// physics??
	Vector velocity = Vector::direction(0, 0, 0);
	Vector angular_velocity = Vector::direction(0, 0, 0);
	Vector force = Vector::direction(0, 0, 0);
	Vector torque = Vector::direction(0, 0, 0);
	double mass = 0.0; // in kilograms?

	// action represents something that happens to the entity on each step
	EntityAction action;
	bool has_action = false;

	// if false, it's no longer part of the scene, it's not rendered and physics
	// are not applied
	// is this still necessary?
	bool active = true;
	bool is_static = false; // non-moving items

	// newly created entities should always be added to the scene through
	// addEntity() or addEntityWithAction(), which will set scene and action
	// appropriately
	// is this still necessary?
	Scene* scene;

	// a copy of the model with transformations applied, used for physics
	Model model_in_world;
	// values > 1 make more translucent
	int translucency = 1;

	Collision collision;
	bool in_midair = false;

	char name[16];

	void setName(const char* name) {
		int i = 0;

		for (; i < 15; i++) {
			if (name[i] == '\0') {
				break;
			}

			this->name[i] = name[i];
		}

		this->name[i] = '\0';
	}

	Vector actualRotation() {
		return this->model->initial_rotation.add(this->rotation);
	}

	Vector centroid() {
		// technically, this will be a point, but the w value must be 0
		Vector centroid = Vector{0, 0, 0, 0};

		size_t vertex_count = this->model_in_world.vertices.size();

		for (int i = 0; i < vertex_count; i++) {
			centroid = centroid.add(this->model_in_world.vertices[i]);
		}

		return centroid.scalarMultiply(1.0 / vertex_count);
	}

	void buildWorldModel();

	// PHYSICS

	// parameter value should be in meters per second
	void applyForce(Vector applied_force, Vector point_of_application);

	void applyPhysics(std::chrono::microseconds frame_duration);

	// for each triangle,  find the plane that contains it. if the plane is in
	// front of the ray, determine if the point of intersection is within the
	// triangle itself this is probably slow and could be done better by
	// subdividing the world (BSP?)
	Vector collisionPoint(Vector ray_origin, Vector ray_direction, bool* did_collide);

	// player movement isn't handled like a generic Entity with applyPhysics
	// because that would be really hard to get feeling right
	void moveFromUserInputs(
			std::chrono::microseconds frame_duration,
			InputState* input_state);
};

#endif

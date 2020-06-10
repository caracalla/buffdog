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

	// action represents something that happens to the entity on each step
	EntityAction action;
	bool has_action = false;
	bool visible = true;
	Scene* scene;
};

#endif

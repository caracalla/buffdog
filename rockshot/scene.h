#ifndef BUFFDOG_SCENE
#define BUFFDOG_SCENE

#include <chrono>
#include <vector>

#include "../vector.h"

#include "entity.h"
#include "level.h"
#include "model.h"
#include "player.h"


// define viewport
struct Viewport {
	double width;
	double height;
	double distance; // position on z axis
	double near_plane_distance;
	double far_plane_distance; // position on z axis
};


// camera is always at final render origin
// position is distance from world origin
// rotation is about x and y world axes
struct Camera : public Entity{
	Viewport viewport;
};


struct Light {
	enum class Type {
		ambient,
		directional,
		point
	};

	Type type;
	Vector direction;
	double intensity;

	static Light ambient(double intensity) {
		return Light{Type::ambient, Vector::direction(0, 0, 0), intensity};
	}

	static Light directional(double intensity, Vector direction) {
		return Light{Type::directional, direction, intensity};
	}
};


struct Scene {
	Camera camera;
	Level level;
	Player player;
	std::vector<Entity> entities;
	std::vector<Light> lights;

	void init(Level level, Player player);

	// Entity handling
	void addEntity(Entity&& entity);
	void addEntityWithAction(Entity&& entity, EntityAction action);

	void step(std::chrono::microseconds frame_duration);
};

#endif

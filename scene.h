#ifndef BUFFDOG_SCENE
#define BUFFDOG_SCENE

#include <chrono>
#include <vector>

#include "entity.h"
#include "device.h"
#include "level.h"
#include "model.h"
#include "player.h"
#include "vector.h"


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
struct Camera {
	Viewport viewport;
	Vector position;
	Vector rotation; // only rotation about x and y axes matters
};


enum LightType {
	ambient,
	directional,
	point
};

struct Light {
	LightType type;
	Vector direction;
	double intensity;
};


struct Scene {
	Camera camera;
	Level level;
	Player player;
	std::vector<Entity> entities;
	std::vector<Light> lights;

	void init(Level level, Player player);

	// Entity handling
	void addEntity(Entity entity);
	void addEntityWithAction(Entity entity, EntityAction action);

	// TODO: make input handling a device responsibility
	//       the player and other entities should read input from device
	void readInput(std::chrono::microseconds frame_duration);

	void step(std::chrono::microseconds frame_duration);

	// int getBackgroundColor() {
	// 	return device::color(backgroundColor.x, backgroundColor.y, backgroundColor.z);
	// }
};

#endif

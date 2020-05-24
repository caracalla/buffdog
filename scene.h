#ifndef BUFFDOG_SCENE
#define BUFFDOG_SCENE

#include <vector>

#include "device.h"
#include "level.h"
#include "model.h"
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
// translation is distance from world origin
// rotation is about x and y world axes
struct Camera {
	Viewport viewport;
	Vector translation;
	Vector rotation; // z is ignored
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
	Level level;
	std::vector<Model> models;
	Camera camera;
	std::vector<Light> lights;


	static Scene create() {
		Scene scene;

		scene.camera.viewport.width = 4;
		scene.camera.viewport.height = 3;
		scene.camera.viewport.distance = -2;
		scene.camera.viewport.near_plane_distance = -0.1;
		scene.camera.viewport.far_plane_distance = -100;

		scene.camera.translation = Vector::point(0, 0, 0);
		scene.camera.rotation = Vector::direction(0, 0, 0);

		Light ambient = {LightType::ambient, Vector::direction(0, 0, 0), 0.2};
		Light directional = {LightType::directional, Vector::direction(-1, 1, 1).unit(), 0.8};

		scene.lights.push_back(ambient);
		scene.lights.push_back(directional);

		return scene;
	}

	void addModel(Model item) {
		this->models.push_back(item);
	}

	void setLevel(Level level) {
		this->level = level;
		this->level.init();

		this->camera.translation = this->level.player_start_position;
		this->camera.rotation = this->level.player_start_rotation;
	}

	// int getBackgroundColor() {
	// 	return device::color(backgroundColor.x, backgroundColor.y, backgroundColor.z);
	// }
};

#endif

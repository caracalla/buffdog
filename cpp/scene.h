#ifndef BUFFDOG_SCENE
#define BUFFDOG_SCENE

#include <vector>

#include "device.h"
#include "model.h"
#include "vector.h"


// define viewport
struct Viewport {
	double width;
	double height;
	double distance; // position on z axis
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


struct Scene {
	std::vector<Model> models;
	Camera camera;

	static Scene create() {
		Scene scene;

		scene.camera.viewport.width = 4;
		scene.camera.viewport.height = 3;
		scene.camera.viewport.distance = -2;
		scene.camera.viewport.far_plane_distance = -10;

		scene.camera.translation = Vector::point(0, 0, 0);
		scene.camera.rotation = Vector::direction(0, 0, 0);

		return scene;
	}

	void addModel(Model item) {
		models.push_back(item);
	}

	// int getBackgroundColor() {
	// 	return device::color(backgroundColor.x, backgroundColor.y, backgroundColor.z);
	// }
};

#endif

#ifndef BUFFDOG_SCENE
#define BUFFDOG_SCENE

// #include <functional>
#include <vector>

#include "device.h"
#include "entity.h"
#include "level.h"
#include "model.h"
#include "vector.h"

#define INCREMENT 0.005
#define MOUSE_SENSITIVITY_FACTOR 1000
// TODO: tie velocity to FPS
#define MAX_VELOCITY 0.0625


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

struct Player {
	double height = 1.7;
	double eye_height = 1.5; // distance from base
	Vector position; // of base
	Vector rotation;

	Vector weapon_position; // relative to base

	double velocity = 0;

	Model marker;
};


struct Scene {
	Camera camera;
	Level level;
	Player player;
	std::vector<Entity> entities;
	std::vector<Light> lights;


	static Scene create(Level level) {
		Scene scene;

		// set up level
		scene.level = level;
		scene.level.init();

		// set up player
		scene.player.position = scene.level.player_start_position;
		scene.player.rotation = scene.level.player_start_rotation;

		scene.player.marker = buildTetrahedron();

		// set up camera
		scene.camera.position = scene.player.position;
		scene.camera.position.y = scene.player.eye_height;
		scene.camera.rotation = scene.player.rotation;

		scene.camera.viewport.width = 4;
		scene.camera.viewport.height = 3;
		scene.camera.viewport.distance = -2;
		scene.camera.viewport.near_plane_distance = -0.1;
		scene.camera.viewport.far_plane_distance = -100;

		// set up lights
		Light ambient = {LightType::ambient, Vector::direction(0, 0, 0), 0.2};
		Light directional = {LightType::directional, Vector::direction(-1, 1, 1).unit(), 0.8};

		scene.lights.push_back(ambient);
		scene.lights.push_back(directional);

		return scene;
	}

	void addEntity(Entity entity) {
		this->entities.push_back(entity);
	}

	void addEntityWithAction(Entity entity, EntityAction action) {
		entity.action = action;
		entity.has_action = true;

		this->addEntity(entity);
	}

	void step() {
		for (auto& entity : this->entities) {
			if (entity.has_action) {
				entity.action(&entity);
			}
		}

		key_input next_key = device::get_next_key();

		if (next_key) {
			// the camera is always pointing down the z axis in the negative direction
			// by default
			Vector view_normal = Matrix::makeRotationMatrix(this->camera.rotation).
					multiplyVector(Vector::direction(0, 0, -1));
			Vector tetra_rotation = this->camera.rotation;
			tetra_rotation.x -= M_PI + M_PI_2; // align top to point at cameras

			Entity tetra_ent{
					&this->player.marker,
					0.5,
					this->level.collisionPoint(this->camera.position, view_normal),
					tetra_rotation};

			switch(next_key) {
				case x_key:
					this->addEntity(tetra_ent);
					break;

				default:
					break;
			}
		}

		mouse_input mouse_motion = device::getMouseMotion();

		this->camera.rotation.x += (double)mouse_motion.y / MOUSE_SENSITIVITY_FACTOR;
		this->camera.rotation.y += (double)mouse_motion.x / MOUSE_SENSITIVITY_FACTOR;

		key_states_t key_states = device::get_key_states();

		Vector translation = {0, 0, 0, 0};

		if (key_states.up || key_states.down || key_states.left || key_states.right || key_states.yup || key_states.ydown) {
			double max_velocity = MAX_VELOCITY;

			if (key_states.sprint) {
				max_velocity *= 5;
			}

			if (this->player.velocity <= max_velocity) {
				this->player.velocity += INCREMENT;
			} else {
				this->player.velocity -= INCREMENT;
			}
		} else {
			this->player.velocity -= INCREMENT;

			if (this->player.velocity < 0) {
				this->player.velocity = 0;
			}
		}

		if (key_states.up) {
			translation.z -= 1;
		}
		if (key_states.down) {
			translation.z += 1;
		}
		if (key_states.left) {
			translation.x -= 1;
		}
		if (key_states.right) {
			translation.x += 1;
		}
		if (key_states.yup) {
			translation.y += 1;
		}
		if (key_states.ydown) {
			translation.y -= 1;
		}

		Vector movement = translation.unit().scalarMultiply(this->player.velocity);

		// the direction of motion is determined only by the rotation about the y axis
		Matrix rotationAboutY = Matrix::makeRotationMatrix(
				Vector::direction(0, this->camera.rotation.y, 0));
		this->camera.position = this->camera.position.add(rotationAboutY.multiplyVector(movement));
	}

	// int getBackgroundColor() {
	// 	return device::color(backgroundColor.x, backgroundColor.y, backgroundColor.z);
	// }
};

#endif

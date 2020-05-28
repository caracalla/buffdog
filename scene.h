#ifndef BUFFDOG_SCENE
#define BUFFDOG_SCENE

#include <vector>

#include "device.h"
#include "entity.h"
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


	static Scene create(Level level) {
		Scene scene;

		// set up level
		scene.level = level;
		scene.level.init();

		// set up player
		scene.player.position = scene.level.player_start_position;
		scene.player.rotation = scene.level.player_start_rotation;

		// scene.player.weapon_position = Vector::direction(0.5, -0.7, 0);
		scene.player.bullet = buildTetrahedron();

		// set up camera
		// scene.camera.position = scene.player.position;
		// scene.camera.position.y = scene.player.eye_height;
		// scene.camera.rotation = scene.player.rotation;

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
			// to be able to fire this from player.weapon_position, we need to calculate
			// a new rotation based on uhhhh actually I'm not sure
			Vector view_normal = Matrix::makeRotationMatrix(this->camera.rotation).
					multiplyVector(Vector::direction(0, 0, -1));
			Vector tetra_rotation = this->camera.rotation;
			tetra_rotation.x -= M_PI + M_PI_2; // align top to point away from camera

			Entity tetra_ent{
					&this->player.bullet,
					0.5,
					this->player.weapon_position, // this->camera.position,
					tetra_rotation};

			Vector collision_point =
					this->level.collisionPoint(this->player.weapon_position, view_normal);
					// this->level.collisionPoint(this->camera.position, view_normal);

			switch(next_key) {
				case x_key:
					this->addEntityWithAction(
							tetra_ent,
							[collision_point, view_normal](Entity* self) {
								// double distance = collision.subtract(self->position).length();
								double distance =
										collision_point.subtract(self->position).dotProduct(view_normal);

								if (distance > 0) {
									self->position = self->position.add(view_normal);
								} else {
									if (distance < 0) {
										self->position = collision_point;
									}

									// stop processing this entity's action
									self->has_action = false;
								}
							});
					break;

				default:
					break;
			}
		}

		this->player.move();

		this->camera.position = this->player.position;
		this->camera.position.y += this->player.eye_height;
		this->camera.rotation = this->player.rotation;
	}

	// int getBackgroundColor() {
	// 	return device::color(backgroundColor.x, backgroundColor.y, backgroundColor.z);
	// }
};

#endif

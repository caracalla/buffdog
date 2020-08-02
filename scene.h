#ifndef BUFFDOG_SCENE
#define BUFFDOG_SCENE

#include <chrono>
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


std::vector<Entity> new_entity_buffer;


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
		scene.player.bullet_model = buildTetrahedron();
		scene.player.explosion_model = buildCube();

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

	// don't want to mess with the vector of entities before the frame step is
	// over, so we put them in new_entity_buffer and flush that to entities when
	// the step is over
	void addEntity(Entity entity) {
		entity.scene = this;
		new_entity_buffer.push_back(entity);
	}

	void addEntityWithAction(Entity entity, EntityAction action) {
		entity.action = action;
		entity.has_action = true;

		this->addEntity(entity);
	}

	void flushEntityBuffer() {
		std::move(
				new_entity_buffer.begin(),
				new_entity_buffer.end(),
				std::back_inserter(this->entities));

		new_entity_buffer.clear();
	}

	// TODO: make input handling a device responsibility
	//       the player and other entities should read input from device
	#define MICROSECONDS 1000000.0

	void readInput(std::chrono::microseconds frame_duration) {
		key_input next_key = device::get_next_key();

		// the camera is always pointing down the z axis in the negative direction
		// by default
		// to be able to fire this from player.weapon_position, we need to calculate
		// a new rotation based on uhhhh actually I'm not sure
		Vector view_normal = Matrix::makeRotationMatrix(this->player.rotation).
				multiplyVector(Vector::direction(0, 0, -1));

		// rudimentary "rocket" mechanics
		if (next_key) {
			Vector tetra_rotation = this->player.rotation;
			tetra_rotation.x -= M_PI + M_PI_2; // align top to point away from camera

			Vector collision_point =
					this->level.collisionPoint(this->player.weapon_position, view_normal);
			Vector initial_velocity = view_normal.scalarMultiply(20 / MICROSECONDS);

			switch(next_key) {
				case x_key:
					this->addEntityWithAction(
							Entity{
									&this->player.bullet_model,
									0.5,
									this->player.weapon_position,
									tetra_rotation,
									initial_velocity,
									Vector::direction(0, 0, 0),
									1.0},
							[collision_point, view_normal, frame_duration](Entity* self) {
								double distance =
										collision_point.subtract(self->position).dotProduct(view_normal);

								if (distance > 0) {
									self->applyPhysics(frame_duration);
								} else {
									if (distance < 0) {
										self->position = collision_point;
									}

									// stop processing this entity's action
									self->has_action = false;
									self->visible = false;
									spit("bullet done");

									// TODO:
									// after spawning the "explosion" entity,
									// remove self (the bullet) from entity vector

									self->scene->addEntityWithAction(
											Entity{
													&self->scene->player.explosion_model,
													0.1,
													collision_point,
													Vector::direction(0, 0, 0)},
											[](Entity* self) {
												self->scale += 0.05;

												if (self->scale > 1.0) {
													self->has_action = false;
													self->visible = false;
												}
											});
								}
							});
					break;

				default:
					break;
			}
		}

		// shoot a bunch of tetrahedrons out that will fall and disappear
		key_states_t key_states = device::get_key_states();

		if (key_states.spew) {
			// shoot an entity down the view normal with physics
			#define JITTER_AMOUNT 0.05
			Vector random_jitter = Vector{
					device::randomDouble(-JITTER_AMOUNT, JITTER_AMOUNT),
					device::randomDouble(-JITTER_AMOUNT, JITTER_AMOUNT),
					device::randomDouble(-JITTER_AMOUNT, JITTER_AMOUNT)};

			Vector initial_velocity = view_normal.add(random_jitter).scalarMultiply(35 / MICROSECONDS);

			this->addEntityWithAction(
					Entity{
							&this->player.bullet_model,
							0.5,
							this->player.weapon_position,
							Vector::direction(0, 0, 0),
							initial_velocity,
							Vector::direction(0, 0, 0),
							25.0},
					[frame_duration](Entity* self) {
						#define GRAVITY 9.8 / (MICROSECONDS * MICROSECONDS)

						self->force.y -= self->mass * GRAVITY;
						self->applyPhysics(frame_duration);

						if (self->position.y <= 0.5) {
							self->position.y = sqrt(3) / 2;
							self->velocity = Vector::direction(0, 0, 0);

							self->has_action = false;
						}
					});
		}
	}

	void step(std::chrono::microseconds frame_duration) {
		for (auto& entity : this->entities) {
			if (entity.has_action) {
				entity.action(&entity);
			}
		}

		this->readInput(frame_duration);

		this->player.move();

		this->camera.position = this->player.position;
		this->camera.position.y += this->player.eye_height;
		this->camera.rotation = this->player.rotation;

		this->flushEntityBuffer();
	}

	// int getBackgroundColor() {
	// 	return device::color(backgroundColor.x, backgroundColor.y, backgroundColor.z);
	// }
};

#endif

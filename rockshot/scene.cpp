#include "../device.h"

#include "scene.h"


void Scene::init(Level level, Player player) {
	this->level = level;
	this->player = player;
	this->player.scene = this;
	this->player.weapon.scene = this;

	// set up player in level
	this->player.position = this->level.player_start_position;
	this->player.rotation = this->level.player_start_rotation;

	// set up camera
	this->camera.position = this->player.position;
	this->camera.rotation = this->player.rotation;

	this->camera.viewport.width = 4;
	this->camera.viewport.height = 3;
	this->camera.viewport.distance = -2;
	this->camera.viewport.near_plane_distance = -0.1;
	this->camera.viewport.far_plane_distance = -100;

	// set up lights
	this->lights.push_back(Light::ambient(0.2));
	this->lights.push_back(
			Light::directional(0.8, Vector::direction(-1, 1, 1).unit()));
}

// we don't want to mess with the vector of entities before the frame step is
// over, so we put them in temp_entity_buffer and flush that to entities when
// the step is over
std::vector<Entity> temp_entity_buffer;

void Scene::addEntity(Entity&& entity) {
	entity.scene = this;
	temp_entity_buffer.push_back(entity);
}

void Scene::addEntityWithAction(Entity&& entity, EntityAction action) {
	entity.action = action;
	entity.has_action = true;

	this->addEntity(std::move(entity));
}

void flushEntityBuffer(std::vector<Entity>& entities) {
	std::move(
			temp_entity_buffer.begin(),
			temp_entity_buffer.end(),
			std::back_inserter(entities));

	temp_entity_buffer.clear();
}

void Scene::step(std::chrono::microseconds frame_duration) {
	for (auto& entity : this->entities) {
		if (entity.active) {
			entity.buildWorldModel();

			if (entity.has_action) {
				entity.action(&entity);
			}

			entity.applyPhysics(frame_duration);
		}
	}

	// We're reading from device input in both of these functions. Ideally
	// readInput() could convert the input into some kind of series of actions.
	// I think quake does this.  This way, player could avoid including all of
	// device.h
	InputState* input_state = device::getInputState();
	this->player.move(frame_duration, input_state);

	// if desired, we could move the camera separately (for now it moves with the player, in first person view)
	this->camera.moveFromUserInputs(frame_duration, input_state);

	flushEntityBuffer(this->entities);
}

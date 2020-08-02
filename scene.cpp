#include "scene.h"


void Scene::init(Level level, Player player) {
	this->level = level;
	this->player = player;
	this->player.scene = this;

	// set up player in level
	this->player.position = this->level.player_start_position;
	this->player.rotation = this->level.player_start_rotation;

	// set up camera
	this->camera.viewport.width = 4;
	this->camera.viewport.height = 3;
	this->camera.viewport.distance = -2;
	this->camera.viewport.near_plane_distance = -0.1;
	this->camera.viewport.far_plane_distance = -100;

	// set up lights
	Light ambient = {LightType::ambient, Vector::direction(0, 0, 0), 0.2};
	Light directional = {
			LightType::directional,
			Vector::direction(-1, 1, 1).unit(), 0.8};

	this->lights.push_back(ambient);
	this->lights.push_back(directional);
}

// we don't want to mess with the vector of entities before the frame step is
// over, so we put them in temp_entity_buffer and flush that to entities when
// the step is over
std::vector<Entity> temp_entity_buffer;

void Scene::addEntity(Entity entity) {
	entity.scene = this;
	temp_entity_buffer.push_back(entity);
}

void Scene::addEntityWithAction(Entity entity, EntityAction action) {
	entity.action = action;
	entity.has_action = true;

	this->addEntity(entity);
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
		if (entity.has_action) {
			entity.action(&entity);
		}
	}

	this->readInput(frame_duration);

	this->player.move();

	this->camera.position = this->player.position;
	this->camera.position.y += this->player.eye_height;
	this->camera.rotation = this->player.rotation;

	flushEntityBuffer(this->entities);
}

void Scene::readInput(std::chrono::microseconds frame_duration) {
	key_input next_key = device::get_next_key();

	if (next_key) {
		switch(next_key) {
			case x_key:
				// rudimentary "gun" mechanics
				this->player.fireBullet(frame_duration);
				break;

			default:
				break;
		}
	}

	key_states_t key_states = device::get_key_states();

	if (key_states.spew) {
		// continuously shoot bullets down the view normal with basic physics
		this->player.fireSpewBullet(frame_duration);
	}
}

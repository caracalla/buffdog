#include "../matrix.h"
#include "../util.h"

#include "player.h"
#include "scene.h"

constexpr std::chrono::microseconds kWeaponCooldown = std::chrono::microseconds(200000);

void Player::move(std::chrono::microseconds frame_duration, InputState* input_state) {
	this->moveFromUserInputs(frame_duration, input_state);
	Sphere& sphere = this->collision.sphere;
	sphere.center_pos = this->position;
	// y pos has to be adjusted to the center of the player, since player pos is foot bottom
	sphere.center_pos.y += sphere.radius;

	bool on_ground = false;

	for (auto& entity : this->scene->entities) {
		// only want static entities
		if (!entity.active || !entity.is_static) {
			continue;
		}

		Vector collision_point;
		bool did_collide = Collision::sphereVsAABB(
						sphere,
						entity.collision.box,
						collision_point);

		if (did_collide) {
			// find point closest to sphere center on AABB
			// direction = closest - AABB center
			// new_pos = direction.unit() * radius + old_pos
			Vector collision_direction = sphere.center_pos.subtract(collision_point).unit();

			// move the collision sphere (and the player sphere.radius away from the collision point)
			sphere.center_pos = collision_point.add(collision_direction.scalarMultiply(sphere.radius));
			this->position = sphere.center_pos;
			this->position.y -= sphere.radius;

			if (collision_direction.y > 0.5) {
				on_ground = true;
			}
		}
	}

	// we're now falling, if we weren't already
	if (!on_ground) {
		this->in_midair = true;
	} else {
		this->in_midair = false;
	}

	// want to prevent weirdness where the weapon rotates about the player's base
	// position, instead of about the player's eye position
	Vector player_eye_pos = this->weapon.player_local_position;
	player_eye_pos.y -= this->eye_height;
	this->weapon.position = Matrix::makeRotationMatrix(this->rotation).
			multiplyVector(player_eye_pos).add(this->position);
	this->weapon.position.y += this->eye_height;

	this->weapon.rotation = this->rotation;

	// the world model looks weird if the player looks up, which causes rotation
	// about the z axis
	Vector old_rotation = this->rotation;
	this->rotation = Vector::direction(0, this->rotation.y, 0);
	this->buildWorldModel();
	this->rotation = old_rotation;

	this->weapon.buildWorldModel();

	if (this->weapon.cooldown_remaining.count() > 0) {
		this->weapon.cooldown_remaining -= frame_duration;
	} else if (input_state->buttons.action1) {
		this->weapon.fireRocket();
		this->weapon.cooldown_remaining = kWeaponCooldown;
	}
}

Vector Weapon::rocketDirection() {
	// the camera is always pointing down the z axis in the negative direction
	// by default
	return Matrix::makeRotationMatrix(this->rotation).
			multiplyVector(Vector::direction(0, 0, -1)).unit();
}

Entity makeRocket(
		Model* model, Vector position, Vector direction, Vector rotation) {
	Entity rocket;
	rocket.model = model;
	rocket.scale = 0.2;
	rocket.position = position;
	rocket.rotation = rotation;
	// problem: rockets don't take into account the player's existing velocity,
	// so if the player is moving, the newly fired rocket moves strangely
	rocket.velocity = direction.scalarMultiply(20 / MICROSECONDS);
	rocket.mass = 0.0; // shouldn't be affected by gravity

	rocket.setName("rocket");

	return rocket;
}

Entity makeExplosion(Model* model, Vector position) {
	Entity explosion;
	explosion.model = model;
	explosion.scale = 0.1;
	explosion.position = position;

	explosion.setName("explosion");

	return explosion;
}

void Weapon::fireRocket() {
	Vector rocket_direction = this->rocketDirection();

	// align top to point away from player
	Vector rocket_rotation = this->rotation.add(
			Vector::direction(-(kPi + kHalfPi), 0, 0));

	// by default, the "collision point" is just some distance away, in midair
	constexpr float MAX_NO_COLLISION_DISTANCE = 50.0;
	Vector collision_point = this->position.add(
		rocket_direction.scalarMultiply(MAX_NO_COLLISION_DISTANCE));

	float min_tmin = FLT_MAX;

	for (auto& entity : this->scene->entities) {
		Vector possible_collision_point;
		float tmin;

		bool did_collide = Collision::rayVsAABB(
				this->position,
				rocket_direction,
				entity.collision.box,
				tmin,
				possible_collision_point);
		
		if (did_collide && tmin < min_tmin) {
			collision_point = possible_collision_point;
			min_tmin = tmin;
		}
	}

	this->scene->addEntityWithAction(
			makeRocket(
					&this->rocket,
					this->position,
					rocket_direction,
					rocket_rotation),
			[collision_point, rocket_direction](Entity* self, std::chrono::microseconds frame_duration) {
				double distance =
						collision_point.subtract(self->position).dotProduct(rocket_direction);

				if (distance <= 0) {
					// it's hit, stop processing this entity's action
					self->position = collision_point;
					self->has_action = false;
					self->active = false;

					// TODO:
					// after spawning the "explosion" entity,
					// remove self (the rocket) from entity vector

					self->scene->addEntityWithAction(
							makeExplosion(&self->scene->player.weapon.explosion, collision_point),
							[](Entity* self, std::chrono::microseconds frame_duration) {
								// self->scale += 0.05;
								self->scale += frame_duration.count() / MICROSECONDS * 5;

								if (self->scale > 1.5) {
									self->translucency = static_cast<int>((self->scale - 1.4) * 10);
								}

								// printf("scale: %f, translucency: %d\n", self->scale, self->translucency);

								if (self->scale > 2.0) {
									self->has_action = false;
									self->active = false;
								}
							});
				}
			});
}

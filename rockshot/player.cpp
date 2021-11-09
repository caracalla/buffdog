#include "../matrix.h"
#include "../util.h"

#include "player.h"
#include "scene.h"

constexpr std::chrono::microseconds kWeaponCooldown = std::chrono::microseconds(50000);

void Player::move(std::chrono::microseconds frame_duration, InputState* input_state) {
	this->moveFromUserInputs(frame_duration, input_state);

	// the world model looks weird if the player looks up, which causes rotation
	// about the z axis
	Vector old_rotation = this->rotation;
	this->rotation = Vector::direction(0, this->rotation.y, 0);
	this->buildWorldModel();
	this->rotation = old_rotation;

	// move the weapon down a bit, so it's not right in the middle of the field
	// of view (this really only works when looking straight ahead)
	this->weapon_position = this->position.add(Vector::direction(0, 1.2, 0));

	if (this->weapon_cooldown_remaining.count() > 0) {
		this->weapon_cooldown_remaining -= frame_duration;
	} else if (input_state->buttons.action1) {
		this->fireBullet();
		this->weapon_cooldown_remaining = kWeaponCooldown;
	}
}

Vector Player::bulletDirection() {
	// the camera is always pointing down the z axis in the negative direction
	// by default
	// to be able to get the normal from player.weapon_position, we need to
	// calculate a new rotation based on...
	// uhhhh actually I'm not sure
	return Matrix::makeRotationMatrix(this->rotation).
			multiplyVector(Vector::direction(0, 0, -1)).unit();
}

Entity makeBullet(
		Model* model, Vector position, Vector direction, Vector rotation) {
	Entity bullet;
	bullet.model = model;
	bullet.scale = 0.5;
	bullet.position = position;
	bullet.rotation = rotation;
	bullet.velocity = direction.scalarMultiply(20 / MICROSECONDS);
	bullet.mass = 0.0; // shouldn't be affected by gravity

	return bullet;
}

Entity makeExplosion(Model* model, Vector position) {
	Entity explosion;
	explosion.model = model;
	explosion.scale = 0.1;
	explosion.position = position;

	return explosion;
}

void Player::fireBullet() {
	Vector view_normal = this->bulletDirection();
	bool did_collide = false;
	Vector collision_point =
			this->scene->level.collisionPoint(
					this->weapon_position, view_normal, &did_collide);

	// align top to point away from player
	Vector bullet_rotation = this->rotation.add(
			Vector::direction(-(kPi + kHalfPi), 0, 0));

	this->scene->addEntityWithAction(
			makeBullet(
					&this->bullet_model,
					this->weapon_position,
					this->bulletDirection(),
					bullet_rotation),
			[collision_point, view_normal](Entity* self) {
				double distance =
						collision_point.subtract(self->position).dotProduct(view_normal);

				if (distance <= 0) {
					// it's hit, stop processing this entity's action
					self->position = collision_point;
					self->has_action = false;
					self->active = false;

					// TODO:
					// after spawning the "explosion" entity,
					// remove self (the bullet) from entity vector

					self->scene->addEntityWithAction(
							makeExplosion(&self->scene->player.explosion_model, collision_point),
							[](Entity* self) {
								self->scale += 0.05;

								if (self->scale > 1.0) {
									self->has_action = false;
									self->active = false;
								}
							});
				}
			});
}

#define SPEW_JITTER_AMOUNT 0.05

Entity makeSpewBullet(Model* model, Vector position, Vector direction) {
	// fire at random within a circle
	Vector random_jitter = Vector{
			util::randomDouble(-1, 1),
			util::randomDouble(-1, 1),
			util::randomDouble(-1, 1)}.unit().scalarMultiply(SPEW_JITTER_AMOUNT);

	Vector initial_velocity = direction.add(random_jitter).
			scalarMultiply(35 / MICROSECONDS);

	Entity spewBullet;
	spewBullet.model = model;
	spewBullet.scale = 0.5;
	spewBullet.position = position;
	spewBullet.rotation = Vector::direction(0, 0, 0);
	spewBullet.velocity = initial_velocity;
	spewBullet.mass = 25.0;

	return spewBullet;
}

void Player::fireSpewBullet() {
	this->scene->addEntityWithAction(
			makeSpewBullet(
					&this->bullet_model, this->weapon_position, this->bulletDirection()),
			[](Entity* self) {
				double tetra_height = sqrt(3) / 2;

				if (self->position.y <= tetra_height) {
					self->position.y = tetra_height;
					self->velocity = Vector::direction(0, 0, 0);
					self->mass = 0.0; // stop gravity from applying

					self->has_action = false;
				}
			});
}

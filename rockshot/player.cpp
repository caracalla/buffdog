#include "../matrix.h"
#include "../util.h"

#include "player.h"
#include "scene.h"

constexpr std::chrono::microseconds kWeaponCooldown = std::chrono::microseconds(200000);

void Player::move(std::chrono::microseconds frame_duration, InputState* input_state) {
	this->moveFromUserInputs(frame_duration, input_state);

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
		this->weapon.fireBullet();
		this->weapon.cooldown_remaining = kWeaponCooldown;
	}
}

Vector Weapon::bulletDirection() {
	// the camera is always pointing down the z axis in the negative direction
	// by default
	return Matrix::makeRotationMatrix(this->rotation).
			multiplyVector(Vector::direction(0, 0, -1)).unit();
}

Entity makeBullet(
		Model* model, Vector position, Vector direction, Vector rotation) {
	Entity bullet;
	bullet.model = model;
	bullet.scale = 0.2;
	bullet.position = position;
	bullet.rotation = rotation;
	// problem: bullets don't take into account the player's existing velocity,
	// so if the player is moving, the newly fired bullet moves strangely
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

void Weapon::fireBullet() {
	Vector view_normal = this->bulletDirection();
	bool did_collide = false;
	Vector collision_point =
			this->scene->level.collisionPoint(
					this->position, view_normal, &did_collide);

	// align top to point away from player
	Vector bullet_rotation = this->rotation.add(
			Vector::direction(-(kPi + kHalfPi), 0, 0));

	this->scene->addEntityWithAction(
			makeBullet(
					&this->bullet,
					this->position,
					this->bulletDirection(),
					bullet_rotation),
			[collision_point, view_normal](Entity* self, std::chrono::microseconds frame_duration) {
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

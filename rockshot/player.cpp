#include "../device.h"
#include "../matrix.h"
#include "../util.h"

#include "player.h"
#include "scene.h"


#define MICROSECONDS 1000000.0

// walking speed of 2 meters per second
#define MAX_WALKING_VELOCITY (2 / MICROSECONDS)
#define WALKING_VELOCITY_INCREMENT (MAX_WALKING_VELOCITY * 0.1)
#define SPRINTING_VELOCITY_FACTOR 5


// player movement isn't handled like a generic Entity with applyPhysics
// because that would be really hard to get feeling right
void Player::move(std::chrono::microseconds frame_duration) {
	// handle mouse movement
	mouse_input mouse_motion = device::getMouseMotion();

	this->rotation.x += mouse_motion.y;
	this->rotation.y += mouse_motion.x;

	if (this->rotation.x < -M_PI_2) {
		this->rotation.x = -M_PI_2;
	} else if (this->rotation.x > M_PI_2) {
		this->rotation.x = M_PI_2;
	}

	// handle key inputs
	key_states_t key_states = device::getKeyStates();

	Vector translation = {0, 0, 0, 0};

	if (key_states.forward ||
			key_states.reverse ||
			key_states.left ||
			key_states.right ||
			key_states.yup ||
			key_states.ydown) {
		double max_velocity = MAX_WALKING_VELOCITY;

		if (key_states.sprint) {
			max_velocity *= SPRINTING_VELOCITY_FACTOR;
		}

		if (this->velocity_value > max_velocity) {
			this->velocity_value = max_velocity;
		}

		if (this->velocity_value <= max_velocity) {
			this->velocity_value += WALKING_VELOCITY_INCREMENT;
		} else {
			this->velocity_value -= WALKING_VELOCITY_INCREMENT;
		}

		if (key_states.forward) {
			translation.z -= 1;
		}
		if (key_states.reverse) {
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
	} else {
		this->velocity_value -= WALKING_VELOCITY_INCREMENT;

		if (this->velocity_value < 0) {
			this->velocity_value = 0;
		}
	}

	double meters_moved = this->velocity_value * frame_duration.count();

	Vector movement = translation.unit().scalarMultiply(meters_moved);

	// the direction of motion is determined only by the rotation about the y axis
	Matrix rotationAboutY = Matrix::makeRotationMatrix(
			Vector::direction(0, this->rotation.y, 0));
	this->position = this->position.add(rotationAboutY.multiplyVector(movement));
	this->weapon_position = this->position.add(Vector::direction(0, 1.2, 0));
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
			Vector::direction(-(M_PI + M_PI_2), 0, 0));

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

#include "matrix.h"
#include "player.h"
#include "scene.h"


// TODO: move this mouse logic to device
#define INCREMENT 0.005
#define MOUSE_SENSITIVITY_FACTOR 1000
// TODO: tie velocity to FPS
#define MAX_VELOCITY 0.0625


void Player::move(std::chrono::microseconds frame_duration) {
	// handle mouse movement
	mouse_input mouse_motion = device::getMouseMotion();

	this->rotation.x += (double)mouse_motion.y / MOUSE_SENSITIVITY_FACTOR;
	this->rotation.y += (double)mouse_motion.x / MOUSE_SENSITIVITY_FACTOR;
	if (this->rotation.x < -M_PI_2) {
		this->rotation.x = -M_PI_2;
	} else if (this->rotation.x > M_PI_2) {
		this->rotation.x = M_PI_2;
	}

	// handle key inputs
	key_states_t key_states = device::get_key_states();

	Vector translation = {0, 0, 0, 0};

	if (key_states.forward ||
			key_states.reverse ||
			key_states.left ||
			key_states.right ||
			key_states.yup ||
			key_states.ydown) {
		double max_velocity = MAX_VELOCITY;

		if (key_states.sprint) {
			max_velocity *= 5;
		}

		if (this->velocity_value <= max_velocity) {
			this->velocity_value += INCREMENT;
		} else {
			this->velocity_value -= INCREMENT;
		}
	} else {
		this->velocity_value -= INCREMENT;

		if (this->velocity_value < 0) {
			this->velocity_value = 0;
		}
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

	Vector movement = translation.unit().scalarMultiply(this->velocity_value);

	// the direction of motion is determined only by the rotation about the y axis
	Matrix rotationAboutY = Matrix::makeRotationMatrix(
			Vector::direction(0, this->rotation.y, 0));
	this->position = this->position.add(rotationAboutY.multiplyVector(movement));
	this->weapon_position = this->position.add(Vector::direction(0, 1.2, 0));
}

Vector Player::bulletDirection() {
	// the camera is always pointing down the z axis in the negative direction
	// by default
	// to be able to get the nroaml from player.weapon_position, we need to
	// calculate a new rotation based on...
	// uhhhh actually I'm not sure
	return Matrix::makeRotationMatrix(this->rotation).
			multiplyVector(Vector::direction(0, 0, -1));
}

#define MICROSECONDS 1000000.0

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
	Vector collision_point =
			this->scene->level.collisionPoint(
					this->weapon_position, view_normal);

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
					spit("bullet done");

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
	Vector random_jitter = Vector{
			device::randomDouble(-SPEW_JITTER_AMOUNT, SPEW_JITTER_AMOUNT),
			device::randomDouble(-SPEW_JITTER_AMOUNT, SPEW_JITTER_AMOUNT),
			device::randomDouble(-SPEW_JITTER_AMOUNT, SPEW_JITTER_AMOUNT)};

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
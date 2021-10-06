#ifndef BUFFDOG_ENTITY
#define BUFFDOG_ENTITY

#include <chrono>
#include <functional>

#include "../matrix.h"
#include "../vector.h"

#include "model.h"


#define MICROSECONDS 1000000.0


struct Entity;
struct Scene;

typedef std::function<void(Entity*)> EntityAction;

// this is basically just a wrapper around models for now
struct Entity {
	Model* model;
	double scale = 1.0; // dear god this ruined an entire morning
	Vector position = Vector::point(0, 0, 0);
	Vector rotation = Vector::direction(0, 0, 0); // represented as radians around each axis

	double velocity_value = 0; // in meters per microsecond (really just speed)

	// physics??
	Vector velocity = Vector::direction(0, 0, 0);
	Vector angular_velocity = Vector::direction(0, 0, 0);
	Vector force = Vector::direction(0, 0, 0);
	Vector torque = Vector::direction(0, 0, 0);
	double mass = 0.0; // in kilograms?

	// action represents something that happens to the entity on each step
	EntityAction action;
	bool has_action = false;

	// if false, it's no longer part of the scene, it's not rendered and physics
	// are not applied
	bool active = true;

	// newly created entities should always be added to the scene through
	// addEntity() or addEntityWithAction(), which will set scene and action
	// appropriately
	Scene* scene;

	// a copy of the model with transformations applied, used for physics
	Model model_in_world;

	Vector actualRotation() {
		return this->model->initial_rotation.add(this->rotation);
	}

	// in other words... the vertex shader??
	void buildWorldModel() {
		Matrix worldMatrix = Matrix::makeWorldMatrix(
				this->scale, this->rotation, this->position);

		this->model_in_world = *(this->model);

		for (auto& vertex : model_in_world.vertices) {
			// transform to world space
			vertex = worldMatrix.multiplyVector(vertex);
		}

		Matrix normalTransformationMatrix =
				Matrix::makeRotationMatrix(this->rotation);

		for (auto& normal : model_in_world.normals) {
			normal = normalTransformationMatrix.multiplyVector(normal);
		}

		for (auto& triangle : model_in_world.triangles) {
			triangle.normal =
					normalTransformationMatrix.multiplyVector(triangle.normal);
		}
	}

	Vector centroid() {
		// technically, this will be a point, but the w value must be 0
		Vector centroid = Vector{0, 0, 0, 0};

		size_t vertex_count = this->model_in_world.vertices.size();

		for (int i = 0; i < vertex_count; i++) {
			centroid = centroid.add(this->model_in_world.vertices[i]);
		}

		return centroid.scalarMultiply(1.0 / vertex_count);
	}

	// PHYSICS

	// parameter value should be in meters per second
	void applyForce(Vector applied_force, Vector point_of_application) {
		applied_force =
				applied_force.scalarMultiply(1 / (MICROSECONDS * MICROSECONDS));

		// move force to centroid
		//  (need to determine centroid??)
		// apply torque to angular velocity

		Matrix rotmat = Matrix::makeRotationMatrix(this->rotation);
		// this is wrong
		// point_of_application = rotmat.multiplyVector(point_of_application);

		Vector r = point_of_application.subtract(this->centroid());
		// printf("\nr: "); r.log();
		Vector new_torque = applied_force.crossProduct(r);

		// shouldn't it be this? but this goes backwards?
		// Vector new_torque = r.crossProduct(applied_force);

		// printf("torque: "); new_torque.log();
		// TODO: rotate each torque axis somehow?
		Vector rotated_torque = rotmat.multiplyVector(new_torque);
		// printf("rotated torque: "); rotated_torque.log();
		this->torque = this->torque.add(rotated_torque);

		this->force = this->force.add(applied_force);
	}

	void applyPhysics(std::chrono::microseconds frame_duration) {
		// force and torque from applyForce are already in terms of microseconds
		auto dt = frame_duration.count();

		if (this->mass > 0) {
			// this->applyForce(Vector::direction(0, -9.8 * this->mass, 0));

			// apply forces to velocity
			// v2 = v1 + (F / m) * dt
			// Vector delta_v = this->force.scalarMultiply(1 / this->mass).scalarMultiply(dt);
			// this->velocity = this->velocity.add(delta_v);

			// assume scale is roughly "radius"
			double MoI = this->mass * this->scale * this->scale * 2 / 3;
			Vector delta_av = this->torque.scalarMultiply(MoI).scalarMultiply(dt);
			// printf("delta_av: "); delta_av.log();

			this->angular_velocity = this->angular_velocity.add(delta_av);
		}

		// apply velocity to position
		Vector delta_p = this->velocity.scalarMultiply(dt);
		this->position = this->position.add(delta_p);

		// apply angular velocity to rotation
		Vector delta_r = this->angular_velocity.scalarMultiply(dt);
		// Matrix rotmat = Matrix::makeRotationMatrix(this->rotation);
		// delta_r = rotmat.multiplyVector(delta_r);
		// printf("delta_r: "); delta_r.log();
		this->rotation = this->rotation.add(delta_r);

		#define TWO_PI (M_PI * 2)

		for (int i = 0; i < 3; i++) {
			if (this->rotation.at(i) > TWO_PI) {
				this->rotation.at(i) -= TWO_PI;
			} else if (this->rotation.at(i) < -TWO_PI) {
				this->rotation.at(i) += TWO_PI;
			}
		}

		this->angular_velocity = this->angular_velocity.scalarMultiply(0.95);

		// clear out forces
		this->force = Vector::direction(0, 0, 0);
		this->torque = Vector::direction(0, 0, 0);
	}



	#define MAX_NO_COLLISION_DISTANCE 50

	// for each triangle,  find the plane that contains it. if the plane is in
	// front of the ray, determine if the point of intersection is within the
	// triangle itself this is probably slow and could be done better by
	// subdividing the world (BSP?)
	Vector collisionPoint(Vector ray_origin, Vector ray_direction, bool* did_collide) {
		*did_collide = false;

		// start with a default result representing a "collision" an arbitrary
		// distance from the start
		Vector result = ray_origin.add(
				ray_direction.scalarMultiply(MAX_NO_COLLISION_DISTANCE));
		double min_t = INFINITY;

		for (auto& triangle : this->model_in_world.triangles) {
			// TODO: remove
			if (triangle.ignore_texture) {
				triangle.ignore_texture = false;
			}

			Vector triangle_plane = Vector::plane(
					triangle.normal,
					this->model_in_world.vertices[triangle.v0.index]);

			double plane_dot_direction = triangle_plane.dotProduct(ray_direction);

			if (plane_dot_direction == 0) {
				continue;
			}

			double t = -(triangle_plane.dotProduct(ray_origin)) / plane_dot_direction;

			if (t > 0) {
				// check if point is in triangle
				Vector plane_intersect = ray_origin.add(ray_direction.scalarMultiply(t));
				Vector r = plane_intersect.subtract(this->model_in_world.vertices[triangle.v0.index]);
				Vector q1 = this->model_in_world.vertices[triangle.v1.index].subtract(this->model_in_world.vertices[triangle.v0.index]);
				Vector q2 = this->model_in_world.vertices[triangle.v2.index].subtract(this->model_in_world.vertices[triangle.v0.index]);

				double q1_sq = q1.squaredLength();
				double q2_sq = q2.squaredLength();
				double q1_dot_q2 = q1.dotProduct(q2);
				double inv = 1 / (q1_sq * q2_sq - q1_dot_q2 * q1_dot_q2);

				double w1 = (q2_sq * r.dotProduct(q1) - q1_dot_q2 * r.dotProduct(q2)) * inv;
				double w2 = (q1_sq * r.dotProduct(q2) - q1_dot_q2 * r.dotProduct(q1)) * inv;

				if (w1 > 0 && w2 > 0 && (1 - w1 - w2) > 0) {
					if (t < min_t) {
						min_t = t;
						result = plane_intersect;

						// this will draw the triangle with the collision point red
						// triangle.ignore_texture = true;
						// triangle.color = Vector::direction(1.0, 0.0, 0.0);

						*did_collide = true;
					}
				}
			}
		}

		return result;
	}


	// walking speed of 2 meters per second
	#define MAX_WALKING_VELOCITY (2 / MICROSECONDS)
	#define WALKING_VELOCITY_INCREMENT (MAX_WALKING_VELOCITY * 0.1)
	#define SPRINTING_VELOCITY_FACTOR 5


	// player movement isn't handled like a generic Entity with applyPhysics
	// because that would be really hard to get feeling right
	void moveFromUserInputs(std::chrono::microseconds frame_duration) {
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
	}
};

#endif

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include <sys/time.h>

#include "bmp.h"
#include "device.h"
#include "matrix.h"
#include "model.h"
#include "obj.h"
#include "ppm.h"
#include "renderer.h"
#include "scene.h"
#include "texture.h"
#include "triangle.h"
#include "vector.h"


#define DELAY_US 10000
#define INCREMENT 0.005
#define MOUSE_SENSITIVITY_FACTOR 1000
// TODO: tie velocity to FPS
#define MAX_VELOCITY 0.0625


int fps = 0;
struct timespec lastPrintTimeSpec;

void logFPS() {
	fps++;

	struct timespec now;
	clock_gettime(CLOCK_REALTIME, &now);

	if (now.tv_sec - lastPrintTimeSpec.tv_sec >= 1) {
		printf("FPS: %d\n", fps);
		lastPrintTimeSpec = now;
		fps = 0;
	}
}

Vector getPlane(Vector normal, Vector point) {
	Vector result = normal;
	result.w = normal.scalarMultiply(-1).dotProduct(point);
	return result;
}

struct Level {
	Model model; // needs to be a copy because we will be modifying this in init
	Vector player_start_position;
	Vector player_start_rotation;

	void init() {
		for (auto& vertex : model.vertices) {
			// transform to world space
			Matrix worldMatrix = Matrix::makeWorldMatrix(
					model.scale, model.rotation, model.translation);
			vertex = worldMatrix.multiplyVector(vertex);
		}

		Matrix normalTransformationMatrix = Matrix::makeRotationMatrix(model.rotation);

		for (auto& normal : model.normals) {
			normal = normalTransformationMatrix.multiplyVector(normal);
		}

		for (auto& triangle : model.triangles) {
			triangle.normal = normalTransformationMatrix.multiplyVector(triangle.normal);
		}
	}

	Vector collisionPoint(Vector ray_origin, Vector ray_direction) {
		// old shitty way
		Vector result = Vector::point(0, 0, 0);
		double min_t = INFINITY;

		for (auto& triangle : this->model.triangles) {
			Vector triangle_plane = getPlane(
					triangle.normal,
					this->model.vertices[triangle.v0.index]);

			double plane_dot_direction = triangle_plane.dotProduct(ray_direction);

			if (plane_dot_direction == 0) {
				continue;
			}

			double t = -(triangle_plane.dotProduct(ray_origin)) / plane_dot_direction;

			if (t > 0) {
				// check if point is in triangle
				Vector plane_intersect = ray_origin.add(ray_direction.scalarMultiply(t));
				Vector r = plane_intersect.subtract(this->model.vertices[triangle.v0.index]);
				Vector q1 = this->model.vertices[triangle.v1.index].subtract(this->model.vertices[triangle.v0.index]);
				Vector q2 = this->model.vertices[triangle.v2.index].subtract(this->model.vertices[triangle.v0.index]);

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
					}
				}
			}
		}

		return result;
	}
};

int main(int argc, char** argv) {
	if (!device::setUp()) {
		return 1;
	}

	// for FPS determination
	clock_gettime(CLOCK_REALTIME, &lastPrintTimeSpec);

	Scene scene = Scene::create();
	Renderer renderer = Renderer::create(scene.camera.viewport);

	Model cube = buildCube(
			1.0, Vector::direction(48, 2, 40), Vector::direction(0, 0, 0));
	BMPTexture crate_texture = BMPTexture::load("textures/crate.bmp");
	cube.addTexture(&crate_texture);
	scene.addModel(&cube);

	Model city = parseOBJFile("models/city.obj");
	PPMTexture city_texture = PPMTexture::load("textures/city.ppm");
	city.addTexture(&city_texture);
	city.scale = 12.0;
	city.translation = Vector::direction(0, 0, 0);
	city.rotation = Vector::direction(0, 0, 0);
	city.setTriangleNormals();
	scene.addModel(&city);

	Level level = {
		city,
		Vector::direction(38, 0, 38),
		Vector::direction(0, M_PI_2, 0)
	};

	level.init();

	scene.camera.translation = level.player_start_position;
	scene.camera.rotation = level.player_start_rotation;
	scene.camera.translation.y = 1.7; // player height

	double velocity = 0;

	Model tetra = buildTetrahedron(0.5, Vector::direction(0, 0, 0), Vector::direction(0, 0, 0));

	while (device::running()) {
		cube.rotation.x += 0.005;
		cube.rotation.y += 0.007;
		cube.rotation.z += 0.009;

		renderer.drawScene(scene);

		Vector view_normal = Matrix::makeRotationMatrix(scene.camera.rotation).
				multiplyVector(Vector::direction(0, 0, -1));
		Vector camera_pos = scene.camera.translation;
		camera_pos.w = 1;
		tetra.translation = level.collisionPoint(camera_pos, view_normal);

		std::vector<Light> lights;

		renderer.drawModel(renderer.transformModel(tetra), scene.camera.viewport, lights);

		device::updateScreen();
		device::processInput();

		// key_input next_key = device::get_next_key();
		//
		// if (next_key) {
		// 	switch(next_key) {
		// 		case x_key:
		// 			cameraMatrix.log();
		// 			break;
		//
		// 		default:
		// 			break;
		// 	}
		// }

		mouse_input mouse_motion = device::getMouseMotion();

		scene.camera.rotation.x += (double)mouse_motion.y / MOUSE_SENSITIVITY_FACTOR;
		scene.camera.rotation.y += (double)mouse_motion.x / MOUSE_SENSITIVITY_FACTOR;

		key_states_t key_states = device::get_key_states();

		Vector translation = {0, 0, 0, 0};

		if (key_states.up || key_states.down || key_states.left || key_states.right || key_states.yup || key_states.ydown) {
			double max_velocity = MAX_VELOCITY;

			if (key_states.sprint) {
				max_velocity *= 5;
			}

			if (velocity <= max_velocity) {
				velocity += INCREMENT;
			} else {
				velocity -= INCREMENT;
			}
		} else {
			velocity -= INCREMENT;

			if (velocity < 0) {
				velocity = 0;
			}
		}

		if (key_states.up) {
			translation.z -= 1;
		}
		if (key_states.down) {
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

		Vector movement = translation.unit().scalarMultiply(velocity);

		// the direction of motion is determined only by the rotation about the y axis
		Matrix rotationAboutY = Matrix::makeRotationMatrix((Vector){0, scene.camera.rotation.y, 0, 0});
		scene.camera.translation = scene.camera.translation.add(rotationAboutY.multiplyVector(movement));

		logFPS();
	}

	device::tearDown();

	return 0;
}

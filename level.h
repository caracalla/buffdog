#ifndef BUFFDOG_LEVEL
#define BUFFDOG_LEVEL

#include "matrix.h"
#include "model.h"
#include "vector.h"

struct Level {
	Model model;
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

	// for each triangle,  find the plane that contains it. if the plane is in front of the ray, determine if the point of intersection is within the triangle itself this is probably slow and could be done better by subdividing the world (BSP?)
	Vector collisionPoint(Vector ray_origin, Vector ray_direction, Triangle3D** collided_triangle) {
		Vector result = Vector::point(0, 0, 0);
		double min_t = INFINITY;

		for (auto& triangle : this->model.triangles) {
			Vector triangle_plane = Vector::plane(
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
						*collided_triangle = &triangle;
					}
				}
			}
		}

		return result;
	}
};

#endif

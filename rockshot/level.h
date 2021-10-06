#ifndef BUFFDOG_LEVEL
#define BUFFDOG_LEVEL

#include "../matrix.h"
#include "../vector.h"

#include "entity.h"
#include "model.h"


struct Level : public Entity {
	Model level_model;

	Vector player_start_position;
	Vector player_start_rotation;

	void init() {
		// shitty hack because don't want this to double render or anything
		this->active = false;

		// Matrix worldMatrix = Matrix::makeWorldMatrix(
		// 		this->scale, this->rotation, this->position);
		//
		// for (auto& vertex : this->level_model.vertices) {
		// 	// transform to world space
		// 	vertex = worldMatrix.multiplyVector(vertex);
		// }
		//
		// Matrix normalTransformationMatrix = Matrix::makeRotationMatrix(this->rotation);
		//
		// for (auto& normal : this->level_model.normals) {
		// 	normal = normalTransformationMatrix.multiplyVector(normal);
		// }
		//
		// for (auto& triangle : this->level_model.triangles) {
		// 	triangle.normal = normalTransformationMatrix.multiplyVector(triangle.normal);
		// }

		this->buildWorldModel();
	}
};

#endif

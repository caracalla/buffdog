#ifndef BUFFDOG_MAT4
#define BUFFDOG_MAT4

#include "vec4.h"
#include "util.h"


typedef enum {x_axis, y_axis, z_axis, w_axis} axis;


struct mat4 {
	// [row][column]
	double data[4][4];

	double& at(size_t row, size_t col) {
		return data[row][col];
	}

	mat4 transpose() {
		mat4 result;

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
			 	result.at(i, j) = this->at(j, i);
			}
		}

		return result;
	}

	mat4 multiplyMat4(mat4 other) {
		mat4 result;

		double calc;

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
			 	calc = 0;

				for (int k = 0; k < 4; k++) {
					calc += this->at(i, k) * other.at(k, j);
				}

				result.at(i, j) = calc;
			}
		}

		return result;
	}

	vec4 multiplyVec4(vec4 vec) {
		vec4 result;

		for (int i = 0; i < 4; i++) {
			result.at(i) = 0;

			for (int j = 0; j < 4; j++) {
				result.at(i) += this->at(i, j) * vec.at(j);
			}
		}

		return result;
	}

	static mat4 makeScaleMatrix(double scale) {
		mat4 result;

		result.at(0, 0) = scale;
		result.at(0, 1) = 0;
		result.at(0, 2) = 0;
		result.at(0, 3) = 0;

		result.at(1, 0) = 0;
		result.at(1, 1) = scale;
		result.at(1, 2) = 0;
		result.at(1, 3) = 0;

		result.at(2, 0) = 0;
		result.at(2, 1) = 0;
		result.at(2, 2) = scale;
		result.at(2, 3) = 0;

		result.at(3, 0) = 0;
		result.at(3, 1) = 0;
		result.at(3, 2) = 0;
		result.at(3, 3) = 1;

		return result;
	}

	static mat4 makeAxisRotationMatrix(double angle, axis about_axis) {
		double s = sin(angle);
		double c = cos(angle);

		mat4 r;

		if (about_axis == x_axis) {
			r.at(0, 0) = 1;  r.at(0, 1) = 0;  r.at(0, 2) = 0;  r.at(0, 3) = 0;
			r.at(1, 0) = 0;  r.at(1, 1) = c;  r.at(1, 2) = -s; r.at(1, 3) = 0;
			r.at(2, 0) = 0;  r.at(2, 1) = s;  r.at(2, 2) = c;  r.at(2, 3) = 0;
			r.at(3, 0) = 0;  r.at(3, 1) = 0;  r.at(3, 2) = 0;  r.at(3, 3) = 1;
		} else if (about_axis == y_axis) {
			r.at(0, 0) = c;  r.at(0, 1) = 0;  r.at(0, 2) = s;  r.at(0, 3) = 0;
			r.at(1, 0) = 0;  r.at(1, 1) = 1;  r.at(1, 2) = 0;  r.at(1, 3) = 0;
			r.at(2, 0) = -s; r.at(2, 1) = 0;  r.at(2, 2) = c;  r.at(2, 3) = 0;
			r.at(3, 0) = 0;  r.at(3, 1) = 0;  r.at(3, 2) = 0;  r.at(3, 3) = 1;
		} else if (about_axis == z_axis) {
			r.at(0, 0) = c;  r.at(0, 1) = -s; r.at(0, 2) = 0;  r.at(0, 3) = 0;
			r.at(1, 0) = s;  r.at(1, 1) = c;  r.at(1, 2) = 0;  r.at(1, 3) = 0;
			r.at(2, 0) = 0;  r.at(2, 1) = 0;  r.at(2, 2) = 1;  r.at(2, 3) = 0;
			r.at(3, 0) = 0;  r.at(3, 1) = 0;  r.at(3, 2) = 0;  r.at(3, 3) = 1;
		} else {
			terminate("invalid rotation axis");
		}

		return r;
	}

	static mat4 makeRotationMatrix(vec4 rotation) {
#if 1
		mat4 xRotation = makeAxisRotationMatrix(rotation.x * -1, x_axis);
		mat4 yRotation = makeAxisRotationMatrix(rotation.y * -1, y_axis);
		mat4 zRotation = makeAxisRotationMatrix(rotation.z * -1, z_axis);

		return zRotation.multiplyMat4(yRotation.multiplyMat4(xRotation));
#else
		double alpha = rotation.z * -1;
		double beta = rotation.y * -1;
		double gamma = rotation.x * -1;

		double sa = sin(alpha), ca = cos(alpha);
		double sb = sin(beta),  cb = cos(beta);
		double sg = sin(gamma), cg = cos(gamma);

		mat4 result;

		result.at(0, 0) = ca * cb;
		result.at(0, 1) = ca * sb * sg - (sa * cg);
		result.at(0, 2) = ca * sb * cg + sa * sg;
		result.at(0, 3) = 0;

		result.at(1, 0) = sa * cb;
		result.at(1, 1) = sa * sb * sg + ca * cg;
		result.at(1, 2) = sa * sb * cg - (ca * sg);
		result.at(1, 3) = 0;

		result.at(2, 0) = sb * -1;
		result.at(2, 1) = cb * sg;
		result.at(2, 2) = cb * cg;
		result.at(2, 3) = 0;

		result.at(3, 0) = 0;
		result.at(3, 1) = 0;
		result.at(3, 2) = 0;
		result.at(3, 3) = 1;

		return result;
#endif
	}

	static mat4 makeTranslationMatrix(vec4 translation) {
		mat4 result;

		result.at(0, 0) = 1;
		result.at(0, 1) = 0;
		result.at(0, 2) = 0;
		result.at(0, 3) = translation.x;

		result.at(1, 0) = 0;
		result.at(1, 1) = 1;
		result.at(1, 2) = 0;
		result.at(1, 3) = translation.y;

		result.at(2, 0) = 0;
		result.at(2, 1) = 0;
		result.at(2, 2) = 1;
		result.at(2, 3) = translation.z;

		result.at(3, 0) = 0;
		result.at(3, 1) = 0;
		result.at(3, 2) = 0;
		result.at(3, 3) = 1;

		return result;
	}

	static mat4 makeWorldMatrix(double scale, vec4 rotation, vec4 translation) {
		mat4 scaleMat = makeScaleMatrix(scale);
		mat4 rotationMat = makeRotationMatrix(rotation);
		mat4 translationMat = makeTranslationMatrix(translation);

		return translationMat.multiplyMat4(scaleMat).multiplyMat4(rotationMat);
	}

	static mat4 makeCameraMatrix(vec4 rotation, vec4 translation) {
		vec4 safeRotation = vec4::direction(rotation.x, rotation.y, 0);
		vec4 invertedTranslation = vec4::direction(
				-translation.x,
				-translation.y,
				-translation.z);

		mat4 rotationMat = makeRotationMatrix(safeRotation).transpose();
		mat4 translationMat = makeTranslationMatrix(invertedTranslation);

		return rotationMat.multiplyMat4(translationMat);
	}

	void log() {
		printf("{%f, %f, %f, %f}\n", this->at(0, 0), this->at(0, 1), this->at(0, 2), this->at(0, 3));
		printf("{%f, %f, %f, %f}\n", this->at(1, 0), this->at(1, 1), this->at(1, 2), this->at(1, 3));
		printf("{%f, %f, %f, %f}\n", this->at(2, 0), this->at(2, 1), this->at(2, 2), this->at(2, 3));
		printf("{%f, %f, %f, %f}\n", this->at(3, 0), this->at(3, 1), this->at(3, 2), this->at(3, 3));
		printf("\n");
	}

	static mat4 makeIdentity() {
		mat4 result;

		result.at(0, 0) = 1;
		result.at(0, 1) = 0;
		result.at(0, 2) = 0;
		result.at(0, 3) = 0;

		result.at(1, 0) = 0;
		result.at(1, 1) = 1;
		result.at(1, 2) = 0;
		result.at(1, 3) = 0;

		result.at(2, 0) = 0;
		result.at(2, 1) = 0;
		result.at(2, 2) = 1;
		result.at(2, 3) = 0;

		result.at(3, 0) = 0;
		result.at(3, 1) = 0;
		result.at(3, 2) = 0;
		result.at(3, 3) = 1;

		return result;
	}
};

#endif

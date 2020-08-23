#ifndef BUFFDOG_MAT4
#define BUFFDOG_MAT4

#include "vector.h"


typedef enum {x_axis, y_axis, z_axis, w_axis} axis;


struct Matrix {
	// [row][column]
	double data[4][4];

	double& at(size_t row, size_t col) {
		return data[row][col];
	}

	Matrix transpose() {
		Matrix result;

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
			 	result.at(i, j) = this->at(j, i);
			}
		}

		return result;
	}

	Matrix multiplyMatrix(Matrix other) {
		Matrix result;

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

	Vector multiplyVector(Vector vec) {
		Vector result;

		for (int i = 0; i < 4; i++) {
			result.at(i) = 0;

			for (int j = 0; j < 4; j++) {
				result.at(i) += this->at(i, j) * vec.at(j);
			}
		}

		return result;
	}

	static Matrix makeScaleMatrix(double scale) {
		Matrix result;

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

	#define ROTATION_EPSILON 0.00001 * 0.00001 * 0.00001

	static Matrix makeAxisRotationMatrix(double angle, axis about_axis) {
		double s = sin(angle);
		double c = cos(angle);

		if (double abs_s = fabs(s); abs_s > 0 && abs_s < ROTATION_EPSILON) {
			s = 0;
		}

		if (double abs_c = fabs(c); abs_c > 0 && abs_c < ROTATION_EPSILON) {
			c = 0;
		}

		Matrix r;

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
			terminateFatal("invalid rotation axis");
		}

		return r;
	}

	static Matrix makeRotationMatrix(Vector rotation) {
#if 1
		Matrix xRotation = makeAxisRotationMatrix(rotation.x * -1, x_axis);
		Matrix yRotation = makeAxisRotationMatrix(rotation.y * -1, y_axis);
		Matrix zRotation = makeAxisRotationMatrix(rotation.z * -1, z_axis);

		return zRotation.multiplyMatrix(yRotation.multiplyMatrix(xRotation));
#else
		double alpha = rotation.z * -1;
		double beta = rotation.y * -1;
		double gamma = rotation.x * -1;

		double sa = sin(alpha), ca = cos(alpha);
		double sb = sin(beta),  cb = cos(beta);
		double sg = sin(gamma), cg = cos(gamma);

		Matrix result;

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

	static Matrix makeTranslationMatrix(Vector translation) {
		Matrix result;

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

	static Matrix makeWorldMatrix(double scale, Vector rotation, Vector translation) {
		Matrix scaleMat = makeScaleMatrix(scale);
		Matrix rotationMat = makeRotationMatrix(rotation);
		Matrix translationMat = makeTranslationMatrix(translation);

		return translationMat.multiplyMatrix(scaleMat).multiplyMatrix(rotationMat);
	}

	static Matrix makeCameraMatrix(Vector rotation, Vector translation) {
		Vector safeRotation = Vector::direction(rotation.x, rotation.y, 0);
		Vector invertedTranslation = Vector::direction(
				-translation.x,
				-translation.y,
				-translation.z);

		Matrix rotationMat = makeRotationMatrix(safeRotation).transpose();
		Matrix translationMat = makeTranslationMatrix(invertedTranslation);

		return rotationMat.multiplyMatrix(translationMat);
	}

	void log() {
		printf("{%f, %f, %f, %f}\n", this->at(0, 0), this->at(0, 1), this->at(0, 2), this->at(0, 3));
		printf("{%f, %f, %f, %f}\n", this->at(1, 0), this->at(1, 1), this->at(1, 2), this->at(1, 3));
		printf("{%f, %f, %f, %f}\n", this->at(2, 0), this->at(2, 1), this->at(2, 2), this->at(2, 3));
		printf("{%f, %f, %f, %f}\n", this->at(3, 0), this->at(3, 1), this->at(3, 2), this->at(3, 3));
		printf("\n");
	}

	static Matrix makeIdentity() {
		Matrix result;

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

#ifndef BUFFDOG_VEC4
#define BUFFDOG_VEC4

#include "device.h"

struct Vector {
	double x;
	double y;
	double z;
	double w;

	// convenience method for looping
	// treat as single column matrix
	double& at(size_t row) {
		switch(row) {
			case 0:
				return this->x;
				break;

			case 1:
				return this->y;
				break;

			case 2:
				return this->z;
				break;

			case 3:
				return this->w;
				break;

			default:
				terminateFatal("invalid Vector index");
				return this->w;  // unreachable dummy
				break;
		}
	}

	static Vector point(double x, double y, double z) {
		return (Vector){x, y, z, 1};
	}

	static Vector direction(double x, double y, double z) {
		return (Vector){x, y, z, 0};
	}

	double dotProduct(Vector other) {
		double result = 0;

		for (int i = 0; i < 4; i++) {
			result += this->at(i) * other.at(i);
		}

		return result;
	}

	// only valid in three (or seven) dimensions!
	Vector crossProduct(Vector other) {
		Vector result;

		result.x = (this->y * other.z) - (this->z * other.y);
		result.y = (this->z * other.x) - (this->x * other.z);
		result.z = (this->x * other.y) - (this->y * other.x);
		result.w = 0;

		return result;
	}

	double squaredLength() {
		return (this->x * this->x) + (this->y * this->y) + (this->z * this->z);
	}

	double length() {
		return sqrt(this->squaredLength());
	}

	Vector scalarMultiply(double scalar) {
		Vector result;

		result.x = this->x * scalar;
		result.y = this->y * scalar;
		result.z = this->z * scalar;
		result.w = this->w * scalar;

		return result;
	}

	Vector unit() {
		double length = this->length();

		if (length == 0) {
			return (Vector){0, 0, 0, 0};
		}

		return scalarMultiply(1 / length);
	}

	Vector add(Vector other) {
		Vector result;

		result.x = this->x + other.x;
		result.y = this->y + other.y;
		result.z = this->z + other.z;
		result.w = this->w + other.w;

		return result;
	}

	Vector subtract(Vector other) {
		Vector result;

		result.x = this->x - other.x;
		result.y = this->y - other.y;
		result.z = this->z - other.z;
		result.w = this->w - other.w;

		return result;
	}

	void log() {
		printf("{%f, %f, %f, %f}\n", this->x, this->y, this->z, this->w);
	}
};

#endif

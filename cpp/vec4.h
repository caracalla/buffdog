#ifndef BUFFDOG_VEC4
#define BUFFDOG_VEC4

#include <stdexcept>

struct vec4 {
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
				// why is it that when I try to replace this with terminate() the compiler
				// yells at me
				throw std::runtime_error("invalid vec4 index");
				break;
		}
	}

	static vec4 point(double x, double y, double z) {
		return (vec4){x, y, z, 1};
	}

	static vec4 direction(double x, double y, double z) {
		return (vec4){x, y, z, 0};
	}

	double dotProduct(vec4 other) {
		double result = 0;

		for (int i = 0; i < 4; i++) {
			result += this->at(i) * other.at(i);
		}

		return result;
	}

	// only valid in three (or seven) dimensions!
	vec4 crossProduct(vec4 other) {
		vec4 result;

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

	vec4 scalarMultiply(double scalar) {
		vec4 result;

		result.x = this->x * scalar;
		result.y = this->y * scalar;
		result.z = this->z * scalar;
		result.w = this->w * scalar;

		return result;
	}

	vec4 unit() {
		double length = this->length();

		if (length == 0) {
			return (vec4){0, 0, 0, 0};
		}

		return scalarMultiply(1 / length);
	}

	vec4 add(vec4 other) {
		vec4 result;

		result.x = this->x + other.x;
		result.y = this->y + other.y;
		result.z = this->z + other.z;
		result.w = this->w + other.w;

		return result;
	}

	vec4 subtract(vec4 other) {
		vec4 result;

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

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

	double squared_length() {
		return (this->x * this->x) + (this->y * this->y) + (this->z * this->z);
	}

	double length() {
		return sqrt(this->squared_length());
	}

	vec4 scalar_multiply(double scalar) {
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

		return scalar_multiply(1 / length);
	}

	void add(vec4 other) {
		this->x += other.x;
		this->y += other.y;
		this->z += other.z;
		this->w += other.w;
	}
};

#endif

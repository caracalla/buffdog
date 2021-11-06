#ifndef BUFFDOG_VEC4
#define BUFFDOG_VEC4

#include <cmath>

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
		return Vector{x, y, z, 1.0};
	}

	static Vector direction(double x, double y, double z) {
		return Vector{x, y, z, 0.0};
	}

	static Vector color(double r, double g, double b) {
		// should we validate the ranges? clamp to 0-1?
		return direction(r, g, b);
	}

	// find the plane for a given normal and point, in vector format
	static Vector plane(Vector normal, Vector point) {
		return Vector{
			normal.x,
			normal.y,
			normal.z,
			normal.scalarMultiply(-1).dotProduct(point)
		};
	}

	// can't be marked const because at() returns a reference
	double dotProduct(Vector other) {
		double result = 0;

		for (int i = 0; i < 4; i++) {
			result += this->at(i) * other.at(i);
		}

		return result;
	}

	// only valid in three (or seven) dimensions!
	Vector crossProduct(Vector other) const {
		Vector result;

		result.x = (this->y * other.z) - (this->z * other.y);
		result.y = (this->z * other.x) - (this->x * other.z);
		result.z = (this->x * other.y) - (this->y * other.x);
		result.w = 0;

		return result;
	}

	double squaredLength() const {
		return (this->x * this->x) + (this->y * this->y) + (this->z * this->z);
	}

	// square roots are expensive apparently
	double length() const {
		return sqrt(this->squaredLength());
	}

	Vector scalarMultiply(double scalar) const {
		Vector result;

		result.x = this->x * scalar;
		result.y = this->y * scalar;
		result.z = this->z * scalar;
		result.w = this->w * scalar;

		return result;
	}

	Vector unit() const {
		double length = this->length();

		// preserve w
		double w = this->w;

		if (length == 0) {
			return Vector{0, 0, 0, w};
		}

		Vector result = this->scalarMultiply(1 / length);
		result.w = w;

		return result;
	}

	// direction + direction = direction
	// direction - direction = direction
	// point + direction = point
	// point - point = direction
	// point + point is INVALID
	Vector add(Vector other) const {
		Vector result;

		result.x = this->x + other.x;
		result.y = this->y + other.y;
		result.z = this->z + other.z;
		result.w = this->w + other.w;

		return result;
	}

	Vector subtract(Vector other) const {
		Vector result;

		result.x = this->x - other.x;
		result.y = this->y - other.y;
		result.z = this->z - other.z;
		result.w = this->w - other.w;

		return result;
	}

	void log(const char* identifier = "") const {
		device::logOncePerSecond("vector %s: {%f, %f, %f, %f}\n", identifier, this->x, this->y, this->z, this->w);
	}
};

#endif

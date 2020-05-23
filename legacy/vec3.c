#include <math.h>

#include "vec3.h"

vec3 add_vec3(vec3 first, vec3 second) {
	vec3 result;

	result.x = first.x + second.x;
	result.y = first.y + second.y;
	result.z = first.z + second.z;

	return result;
}

vec3 subtract_vec3(vec3 first, vec3 second) {
	vec3 result;

	result.x = first.x - second.x;
	result.y = first.y - second.y;
	result.z = first.z - second.z;

	return result;
}

vec3 multiply_vec3(vec3 first, vec3 second) {
	vec3 result;

	result.x = first.x * second.x;
	result.y = first.y * second.y;
	result.z = first.z * second.z;

	return result;
}

vec3 divide_vec3(vec3 first, vec3 second) {
	vec3 result;

	result.x = first.x / second.x;
	result.y = first.y / second.y;
	result.z = first.z / second.z;

	return result;
}

vec3 scalar_multiply(vec3 vec, double scalar) {
	vec3 result;

	result.x = vec.x * scalar;
	result.y = vec.y * scalar;
	result.z = vec.z * scalar;

	return result;
}

vec3 scalar_divide(vec3 vec, double scalar) {
	vec3 result;

	result.x = vec.x / scalar;
	result.y = vec.y / scalar;
	result.z = vec.z / scalar;

	return result;
}

double dot_product(vec3 first, vec3 second) {
	return (first.x * second.x) + (first.y * second.y) + (first.z * second.z);
}

vec3 cross_product(vec3 first, vec3 second) {
	vec3 result;

	result.x = (first.y * second.z) - (first.z * second.y);
	result.y = (first.z * second.x) - (first.x * second.z);
	result.z = (first.x * second.y) - (first.y - second.x);

	return result;
}

vec3 unit_vector(vec3 vec) {
	return scalar_divide(vec, length(vec));
}

double squared_length(vec3 vec) {
	return (vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z);
}

double length(vec3 vec) {
	return sqrt(squared_length(vec));
}

#ifndef BUFFDOG_SPHERE
#define BUFFDOG_SPHERE

#include <stddef.h>

#include "vec3.h"

typedef struct {
	double radius;
	vec3 center;
	vec3 color;
} sphere;

typedef struct {
	sphere *data;
	size_t length;
	size_t max_length;
} sphere_list;

sphere make_sphere(double radius, vec3 center, vec3 color);

sphere_list make_sphere_list(size_t initial_size);

void append_sphere(sphere_list *list, sphere item);

void free_sphere_list(sphere_list *list);

#endif

#ifndef BUFFDOG_light
#define BUFFDOG_light

#include <stddef.h>

#include "vec3.h"

enum light_type {
	ambient,
	point,
	directional
};

typedef struct {
	enum light_type type;
	double intensity;
	vec3 position; // only applies to point sources
	vec3 direction; // only applies to directional sources
} light;

typedef struct {
	light *data;
	size_t length;
	size_t max_length;
} light_list;

light make_light(enum light_type type, double intesity, vec3 position, vec3 direction);

light_list make_light_list(size_t initial_size);

void append_light(light_list *list, light item);

void free_light_list(light_list *list);

#endif

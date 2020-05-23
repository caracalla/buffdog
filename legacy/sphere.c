#include "sphere.h"

#include <stdlib.h>

#include "util.h"

sphere make_sphere(double radius, vec3 center, vec3 color) {
	sphere result = {radius, center, color};
	return result;
}

sphere_list make_sphere_list(size_t initial_size) {
	sphere_list list;

	list.data = malloc(initial_size * sizeof(sphere));

	if (!list.data) {
		self_destruct("failed to initialize sphere_list", __LINE__, __FILE__);
	}

	list.max_length = initial_size;
	list.length = 0;

	return list;
}

void resize_sphere_list(sphere_list *list, size_t new_size) {
	sphere *new_data = malloc(new_size * sizeof(sphere));

	if (!new_data) {
		self_destruct("failed to resize sphere_list", __LINE__, __FILE__);
	}

	size_t i;

	for (i = 0; i < list->length && i < new_size; i++) {
		new_data[i] = list->data[i];
	}

	free(list->data);

	list->data = new_data;
	list->max_length = new_size;
	list->length = i;
}

void append_sphere(sphere_list *list, sphere item) {
	if (list->length == list->max_length) {
		resize_sphere_list(list, list->max_length * 2);
	}

	list->data[list->length] = item;
	list->length += 1;
}

void free_sphere_list(sphere_list *list) {
	free(list->data);
}

#include "light.h"

#include <stdlib.h>

#include "util.h"

light make_light(enum light_type type, double intensity, vec3 position, vec3 direction) {
	light result = {type, intensity, position, direction};
	return result;
}

light_list make_light_list(size_t initial_size) {
	light_list list;

	list.data = malloc(initial_size * sizeof(light));

	if (!list.data) {
		self_destruct("failed to initialize light_list", __LINE__, __FILE__);
	}

	list.max_length = initial_size;
	list.length = 0;

	return list;
}

void resize_light_list(light_list *list, size_t new_size) {
	light *new_data = malloc(new_size * sizeof(light));

	if (!new_data) {
		self_destruct("failed to resize light_list", __LINE__, __FILE__);
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

void append_light(light_list *list, light item) {
	if (list->length == list->max_length) {
		resize_light_list(list, list->max_length * 2);
	}

	list->data[list->length] = item;
	list->length += 1;
}

void free_light_list(light_list *list) {
	free(list->data);
}

#ifndef BUFFDOG_VEC3
#define BUFFDOG_VEC3

struct vec3 {
	double x;
	double y;
	double z;
};

typedef struct vec3 vec3;

vec3 add_vec3(vec3 first, vec3 second);
vec3 subtract_vec3(vec3 first, vec3 second);

// I don't think I understand these
vec3 multiply_vec3(vec3 first, vec3 second);
vec3 divide_vec3(vec3 first, vec3 second);

vec3 scalar_multiply(vec3 vec, double scalar);
vec3 scalar_divide(vec3 vec, double scalar);

double dot_product(vec3 first, vec3 second);
vec3 cross_product(vec3 first, vec3 second);

vec3 unit_vector(vec3 vec);
double squared_length(vec3 vec);
double length(vec3 vec);

#endif

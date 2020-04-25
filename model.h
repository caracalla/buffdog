#ifndef BUFFDOG_MODEL
#define BUFFDOG_MODEL

#include <math.h>

#include "vec3.h"
#include "triangle.h"

// Logic for drawing models (currently, just cubes)

#define CUBE_V_COUNT 8
#define CUBE_T_COUNT 12

// 3D triangles are represented as indices in the model's vertex list
typedef struct {
  size_t v0;
  size_t v1;
  size_t v2;
  int color;
} tri3d;

typedef struct {
  vec3 vertices[8];
  tri3d triangles[12];
  double scale;
  vec3 translation;
  vec3 rotation; // represented as radians around each axis
} cube;

cube buildCube(double scale, vec3 translation, vec3 rotation) {
  int red = color(1, 0, 0);
  int blue = color(0, 0, 1);
  int green = color(0, 1, 0);
  int yellow = color(0, 0, 0); // color(1, 1, 0);
  int purple = color(1, 0, 1);
  int cyan = color(0, 1, 1);
  cube item = {
    { // vertices
      { 1,  1,  1},
      {-1,  1,  1},
      {-1, -1,  1},
      { 1, -1,  1},
      { 1,  1, -1},
      {-1,  1, -1},
      {-1, -1, -1},
      { 1, -1, -1},
    },
    { // triangles
      {0, 1, 2, red},
      {0, 2, 3, red},
      {4, 0, 3, green},
      {4, 3, 7, green},
      {5, 4, 7, blue},
      {5, 7, 6, blue},
      {1, 5, 6, yellow},
      {1, 6, 2, yellow},
      {4, 5, 1, purple},
      {4, 1, 0, purple},
      {2, 6, 7, cyan},
      {2, 7, 3, cyan},
    },
    scale,
    translation,
    rotation
  };

  return item;
}

// naive transformation, to be replaced with matrix
cube applyTransform(cube item) {
  // first, scale
  double scale = item.scale > 0 ? item.scale : 0;

  for (int i = 0; i < CUBE_V_COUNT; i++) {
    item.vertices[i].x *= scale;
    item.vertices[i].y *= scale;
    item.vertices[i].z *= scale;
  }

  // second, rotation
  double alpha = item.rotation.z * -1;
  double beta = item.rotation.y * -1;
  double gamma = item.rotation.x * -1;

  double sa = sin(alpha), ca = cos(alpha);
  double sb = sin(beta),  cb = cos(beta);
  double sg = sin(gamma), cg = cos(gamma);

  vec3 xrow = {
      ca * cb,
      ca * sb * sg - (sa * cg),
      ca * sb * cg + sa * sg};

  vec3 yrow = {
      sa * cb,
      sa * sb * sg + ca * cg,
      sa * sb * cg - (ca * sg)};

  vec3 zrow = {
      sb * -1,
      cb * sg,
      cb * cg};

  for (int i = 0; i < CUBE_V_COUNT; i++) {
    vec3 original = item.vertices[i];
    item.vertices[i].x = dot_product(xrow, original);
    item.vertices[i].y = dot_product(yrow, original);
    item.vertices[i].z = dot_product(zrow, original);
  }

  // third, translation
  for (int i = 0; i < CUBE_V_COUNT; i++) {
    item.vertices[i] = add_vec3(item.vertices[i], item.translation);
  }

  return item;
}

#endif

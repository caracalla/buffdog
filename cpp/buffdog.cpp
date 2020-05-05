// usleep
#define _BSD_SOURCE
#include <unistd.h>

#include <cstdlib>
#include <cmath>
#include <ctime>

#include "device.h"
#include "line.h"
#include "matrix.h"
#include "model.h"
#include "triangle.h"
#include "vector.h"
#include "util.h"


#define DELAY_US 10000
#define INCREMENT 0.005
#define MOUSE_SENSITIVITY_FACTOR 1000
// TODO: tie velocity to FPS
#define MAX_VELOCITY 0.03125

#define NUM_FRUSTUM_PLANES 6


int fps = 0;
clock_t nextPrintTime;

void printFPS() {
	fps++;

	clock_t nowTime = clock();

	double timeDiff = ((double) (nowTime - nextPrintTime)) / CLOCKS_PER_SEC;

	if (timeDiff >= 1.0) {
		nextPrintTime = nowTime;
		printf("FPS: %d\n", fps);
		fps = 0;
	}
}


// define viewport
struct viewport_t {
	double width;
	double height;
	double distance; // position on z axis
	double far_plane_distance; // position on z axis
};

// camera is always at final render origin
// translation is distance from world origin
// rotation is about x and y world axes
struct camera_t {
	viewport_t viewport;
	Vector translation;
	Vector rotation; // z is ignored
};

// camera starts out at the world origin
viewport_t viewport = {4, 3, -2, -10};
camera_t camera = {viewport, {0, 0, 0, 1}, {0, 0, 0, 0}};

// TODO: determine these dynamically from viewport
// the x value for the right plane and the y value for the high plane are a
// little off because otherwise the clipAndDraw function would try to draw out
// of bounds
Vector frustum_planes[NUM_FRUSTUM_PLANES] = {
	{0, 0, -1, viewport.distance}, // near plane
	{0.7071, 0, -0.7071, 0}, // left plane
	{-0.70712, 0, -0.7071, 0}, // right plane
	{0, -0.801, -0.6, 0}, // high plane
	{0, 0.8, -0.6, 0}, // low plane
	{0, 0, 1, -viewport.far_plane_distance} // far plane
};

// void setUpFrustumPlanes() {
// 	// near plane
// 	frustum_planes[0] = {0, 0, 1, -viewport.distance};
// 	// far plane
// 	frustum_planes[1] = {0, 0, 1, viewport.far_plane_distance};
// }

double limit_value = 0.0;

bool insidePlane(Vector vertex, Vector plane) {
	return vertex.dotProduct(plane) > limit_value;
}

bool insideFrustum(Vector vertex) {
	for (int i = 0; i < NUM_FRUSTUM_PLANES; i++) {
		if (!insidePlane(vertex, frustum_planes[i])) {
			return false;
		}
	}

	return true;
}

// the max potential vertices for a triangle is 9
//   clipping a convex polygon of n vertices against each plane can yield n+1 vertices
//   clipping against six planes yields n + 6 vertices
#define MAX_VERTICES 9
Vector vertices1[MAX_VERTICES];
Vector vertices2[MAX_VERTICES];

Vector linePlaneIntersection(Vector v1, Vector v2, Vector plane) {
	double t = plane.dotProduct(v1) / plane.dotProduct(v1.subtract(v2));

	return v1.add(v2.subtract(v1).scalarMultiply(t));
}

Point viewportToCanvas(double x, double y) {
	Point result;
	result.x = x * device::getXRes() / camera.viewport.width + device::getXRes() / 2;
	result.y = y * device::getYRes() / camera.viewport.height + device::getYRes() / 2;
	return result;
}

Point projectVertex(Vector vertex) {
	double x = vertex.x * camera.viewport.distance / vertex.z;
	double y = vertex.y * camera.viewport.distance / vertex.z;
	return viewportToCanvas(x, y);
}

// Sutherland-Hodgman algorithm
// TODO: Take into account triangle winding.  This *should* already do so but
// I'm not certain
void clipAndDraw(Vector v0, Vector v1, Vector v2, int color) {
	Vector* original_vertices = vertices1;
	Vector* new_vertices = vertices2;
	int original_count = 3;
	int new_count = 0;

	original_vertices[0] = v0;
	original_vertices[1] = v1;
	original_vertices[2] = v2;

	// clip against each frustum plane
	for (int i = 0; i < NUM_FRUSTUM_PLANES; i++) {
		new_count = 0;
		int previous = original_count - 1;
		int current = 0;

		// step through each pair of vertices, deciding what to do based on whether
		// each is inside or outside the given plane
		while (current < original_count) {
			if (insidePlane(original_vertices[previous], frustum_planes[i])) {
				if (insidePlane(original_vertices[current], frustum_planes[i])) {
					// just add current
					new_vertices[new_count] = original_vertices[current];
					new_count++;
				} else {
					// add the intersect
					new_vertices[new_count] = linePlaneIntersection(
							original_vertices[current],
							original_vertices[previous],
							frustum_planes[i]);
					new_count++;
				}
			} else {
				if (insidePlane(original_vertices[current], frustum_planes[i])) {
					// add the intersect
					new_vertices[new_count] = linePlaneIntersection(
							original_vertices[current],
							original_vertices[previous],
							frustum_planes[i]);
					new_count++;

					// then add current
					new_vertices[new_count] = original_vertices[current];
					new_count++;
				} else {
					// both previous and current are outside the plane, do nothing
				}
			}

			previous += 1;
			current += 1;
			if (previous == original_count) {
				previous = 0;
			}
		}

		if (i < NUM_FRUSTUM_PLANES - 1) {
			// swap for the next plane
			Vector* temp = original_vertices;
			original_vertices = new_vertices;
			new_vertices = temp;
			original_count = new_count;
		}
	}

	Point projectedVertices[new_count];

	for (int i = 0; i < new_count; i++) {
		projectedVertices[i] = projectVertex(new_vertices[i]);
	}

	// triangulate the resulting polygon, with all triangles starting at v0
	for (int i = 1; i < new_count - 1; i++) {
		Triangle2D triangle = {
				projectedVertices[0],
				projectedVertices[i],
				projectedVertices[i + 1],
				color};

		triangle.fill();
	}
}

// position of the camera after all transforms is always at the origin
Vector cameraOrigin = {0, 0, 0, 1};

bool isBackFace(Vector triangle_normal, Vector vertex) {
	Vector vectorToCamera = cameraOrigin.subtract(vertex);

	return vectorToCamera.dotProduct(triangle_normal) <= 0;
}

void drawModel(Model item) {
	bool isVertexVisible[item.vertices.size()];
	Point projectedVertices[item.vertices.size()];

	for (int i = 0; i < item.vertices.size(); i++) {
		isVertexVisible[i] = insideFrustum(item.vertices[i]);

		if (isVertexVisible[i]) {
			projectedVertices[i] = projectVertex(item.vertices[i]);
		}
	}

	for (auto& triangle : item.triangles) {
		if (isBackFace(triangle.normal, item.vertices[triangle.v0])) {
			// this is a back face, don't draw
			continue;
		}

		if (isVertexVisible[triangle.v0] &&
				isVertexVisible[triangle.v1] &&
				isVertexVisible[triangle.v2]) {
			// all vertices are visible
			Triangle2D tri = {
					projectedVertices[triangle.v0],
					projectedVertices[triangle.v1],
					projectedVertices[triangle.v2],
					triangle.color};

			tri.fill();
		} else if (!isVertexVisible[triangle.v0] &&
				!isVertexVisible[triangle.v1] &&
				!isVertexVisible[triangle.v2]) {
			// no vertices are visible, do nothing
			// NOTE: this is technically incorrect.  For instance, the triangle could
			// be so large that it covers the entire screen, and each vertex is outside
			// of a different plane.  Technically, this should fall into the final
			// condition below.
		} else {
			// some vertices are visible
			// it's clipping time
			clipAndDraw(
					item.vertices[triangle.v0],
					item.vertices[triangle.v1],
					item.vertices[triangle.v2],
					triangle.color);
		}
	}
}

Matrix cameraMatrix;

Model applyTransform(Model item) {
	Matrix worldMatrix = Matrix::makeWorldMatrix(item.scale, item.rotation, item.translation);
	Matrix finalMatrix = cameraMatrix.multiplyMatrix(worldMatrix);

	// transform vertices
	for (auto& vertex : item.vertices) {
		vertex = finalMatrix.multiplyVector(vertex);
	}

	// transform triangle normals (is using finalMatrix here really okay?)
	for (auto& triangle : item.triangles) {
		triangle.normal = finalMatrix.multiplyVector(triangle.normal);
	}

	return item;
}

int main() {
	if (!device::setUp()) {
		return 1;
	}

	// for FPS determination
	nextPrintTime = clock();

	double cube1Scale = 1;
	Vector cube1translation = Vector::direction(-1.5, 0, -7);
	Vector cube1rotation = Vector::direction(0, 0, 0);
	Model cube1 = buildCube(cube1Scale, cube1translation, cube1rotation);

	double cube2Scale = 0.5;
	Vector cube2translation = Vector::direction(2, -0.5, -6);
	Vector cube2rotation = Vector::direction(0, 0, 0);
	Model cube2 = buildCube(cube2Scale, cube2translation, cube2rotation);

	double velocity = 0;

	while (device::running()) {
		usleep(DELAY_US);

		cube1.rotation.x += 0.005;
		cube1.rotation.y += 0.007;
		cube1.rotation.z += 0.009;

		device::clearScreen(device::color(1, 1, 1));

		cameraMatrix = Matrix::makeCameraMatrix(camera.rotation, camera.translation);

		drawModel(applyTransform(cube1));
		drawModel(applyTransform(cube2));

		device::updateScreen();
		device::processInput();

		// key_input next_key = get_next_key();
		//
		// if (next_key) {
		// 	switch(next_key) {
		// 		case x_key:
		// 			cameraMatrix.log();
		// 			break;
		//
		// 		default:
		// 			break;
		// 	}
		// }

		mouse_input mouse_motion = device::getMouseMotion();

		camera.rotation.x += (double)mouse_motion.y / MOUSE_SENSITIVITY_FACTOR;
		camera.rotation.y += (double)mouse_motion.x / MOUSE_SENSITIVITY_FACTOR;

		key_states_t key_states = device::get_key_states();

		Vector translation = {0, 0, 0, 0};

		if (key_states.up || key_states.down || key_states.left || key_states.right || key_states.yup || key_states.ydown) {
			if (velocity < MAX_VELOCITY) {
				velocity += INCREMENT;
			}
		} else {
			velocity -= INCREMENT;

			if (velocity < 0) {
				velocity = 0;
			}
		}

		if (key_states.up) {
			translation.z -= 1;
		}
		if (key_states.down) {
			translation.z += 1;
		}
		if (key_states.left) {
			translation.x -= 1;
		}
		if (key_states.right) {
			translation.x += 1;
		}
		if (key_states.yup) {
			translation.y += 1;
		}
		if (key_states.ydown) {
			translation.y -= 1;
		}

		Vector movement = translation.unit().scalarMultiply(velocity);

		// the direction of motion is determined only by the rotation about the y axis
		Matrix rotationAboutY = Matrix::makeRotationMatrix((Vector){0, camera.rotation.y, 0, 0});
		camera.translation = camera.translation.add(rotationAboutY.multiplyVector(movement));

		printFPS();
	}

	device::tearDown();

	return 0;
}

#ifndef BUFFDOG_RENDERER
#define BUFFDOG_RENDERER

#include <vector>

#include "device.h"
#include "matrix.h"
#include "model.h"
#include "scene.h"
#include "triangle.h"
#include "vector.h"

#define NUM_FRUSTUM_PLANES 6
// the max potential vertices for a triangle clipped against six planes is 9
#define MAX_CLIPPED_POLYGON_VERTICES 9


// these buffers are used for clipping triangles
Vector vertices1[MAX_CLIPPED_POLYGON_VERTICES];
Vector vertices2[MAX_CLIPPED_POLYGON_VERTICES];

// position of the camera after all transforms is always at the origin
const Vector kOrigin = {0, 0, 0, 1};


struct Renderer {
	Vector frustum_planes[NUM_FRUSTUM_PLANES];
	Matrix cameraMatrix;

	static Renderer create(Viewport& viewport) {
		Renderer renderer;

		// TODO: determine these dynamically from viewport
		// the x value for the right plane and the y value for the high plane are a
		// little off because otherwise the clipTriangle function would try to draw out
		// of bounds
		renderer.frustum_planes[0] = {0, 0, -1, viewport.distance}; // near plane
		renderer.frustum_planes[1] = {0.7071, 0, -0.7071, 0}; // left plane
		renderer.frustum_planes[2] = {-0.70712, 0, -0.7071, 0}; // right plane
		renderer.frustum_planes[3] = {0, -0.801, -0.6, 0}; // high plane
		renderer.frustum_planes[4] = {0, 0.8, -0.6, 0}; // low plane
		renderer.frustum_planes[5] = {0, 0, 1, -viewport.far_plane_distance}; // far plane

		return renderer;
	}

	// void setUpFrustumPlanes() {
	// 	// near plane
	// 	frustum_planes[0] = {0, 0, 1, -viewport.distance};
	// 	// far plane
	// 	frustum_planes[1] = {0, 0, 1, viewport.far_plane_distance};
	// }

	bool insidePlane(Vector vertex, Vector plane) {
		return vertex.dotProduct(plane) > 0.0;
	}

	bool insideFrustum(Vector vertex) {
		for (int i = 0; i < NUM_FRUSTUM_PLANES; i++) {
			if (!insidePlane(vertex, frustum_planes[i])) {
				return false;
			}
		}

		return true;
	}

	Vector linePlaneIntersection(Vector v1, Vector v2, Vector plane) {
		double t = plane.dotProduct(v1) / plane.dotProduct(v1.subtract(v2));

		return v1.add(v2.subtract(v1).scalarMultiply(t));
	}


	// projection to canvas

	Point viewportToCanvas(double x, double y, double viewport_width, double viewport_height) {
		Point result;
		result.x = x * device::getXRes() / viewport_width + device::getXRes() / 2;
		result.y = y * device::getYRes() / viewport_height + device::getYRes() / 2;
		return result;
	}

	Point projectVertexToScreen(Vector& vertex, Viewport& viewport) {
		double x = vertex.x * viewport.distance / vertex.z;
		double y = vertex.y * viewport.distance / vertex.z;
		return viewportToCanvas(x, y, viewport.width, viewport.height);
	}

	// drawing

	// Sutherland-Hodgman algorithm
	// fills the result array with a new set of vertices representing the clipped
	// polygon
	// the return value is the length of that array
	// I don't like this but I'll figure out something better
	size_t clipTriangle(Vector v0, Vector v1, Vector v2, Vector* result) {
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

		for (int i = 0; i < new_count; i++) {
			result[i] = new_vertices[i];
		}

		return new_count;
	}

	bool isBackFace(Vector triangle_normal, Vector vertex) {
		Vector vectorToCamera = kOrigin.subtract(vertex);

		return vectorToCamera.dotProduct(triangle_normal) <= 0;
	}

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

	void drawModel(Model item, Viewport& viewport) {
		bool isVertexVisible[item.vertices.size()];
		Point projectedVertices[item.vertices.size()];

		for (int i = 0; i < item.vertices.size(); i++) {
			isVertexVisible[i] = insideFrustum(item.vertices[i]);

			if (isVertexVisible[i]) {
				projectedVertices[i] = projectVertexToScreen(item.vertices[i], viewport);
			}
		}

		for (auto& triangle : item.triangles) {
			if (isBackFace(triangle.normal, item.vertices[triangle.v0])) {
				// this is a back face, don't draw
				// triangle.color = device::color(0, 0, 0);
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
				Vector new_vertices[MAX_CLIPPED_POLYGON_VERTICES];

				int new_vertex_count = clipTriangle(
						item.vertices[triangle.v0],
						item.vertices[triangle.v1],
						item.vertices[triangle.v2],
						new_vertices);

				Point projectedVertices[new_vertex_count];

				for (int i = 0; i < new_vertex_count; i++) {
					projectedVertices[i] = projectVertexToScreen(new_vertices[i], viewport);
				}

				// triangulate the resulting polygon, with all triangles starting at v0
				for (int i = 1; i < new_vertex_count - 1; i++) {
					Triangle2D new_triangle = {
							projectedVertices[0],
							projectedVertices[i],
							projectedVertices[i + 1],
							triangle.color};

					new_triangle.fill();
				}
			}
		}
	}

	void drawScene(Scene& scene) {
		// update camera matrix (should we check if it has changed first?)
		cameraMatrix = Matrix::makeCameraMatrix(scene.camera.rotation, scene.camera.translation);

		// draw the background
		// TODO: make this more interesting/dynamic
		device::clearScreen(device::color(1, 1, 1));

		for (const auto& model : scene.models) {
			drawModel(applyTransform(model), scene.camera.viewport);
		}
	}
};

#endif

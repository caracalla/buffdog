#ifndef BUFFDOG_RENDERER
#define BUFFDOG_RENDERER

#include <array>
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


// position of the camera after all transforms is always at the origin
const Vector kOrigin = {0, 0, 0, 1};


// a container to represent the vertices of a clipped triangle
struct ClippedPolygon {
	std::array<Vector, MAX_CLIPPED_POLYGON_VERTICES> vertices;
	std::array<double, MAX_CLIPPED_POLYGON_VERTICES> shades;
	std::array<double, MAX_CLIPPED_POLYGON_VERTICES> u_values;
	std::array<double, MAX_CLIPPED_POLYGON_VERTICES> v_values;
	size_t vertex_count;
};


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
	// 	this->frustum_planes[0] = {0, 0, 1, -viewport.distance};
	// 	// far plane
	// 	this->frustum_planes[1] = {0, 0, 1, viewport.far_plane_distance};
	// }

	bool insidePlane(Vector vertex, Vector plane) {
		return vertex.dotProduct(plane) > 0.0;
	}

	bool insideFrustum(Vector vertex) {
		for (int i = 0; i < NUM_FRUSTUM_PLANES; i++) {
			if (!insidePlane(vertex, this->frustum_planes[i])) {
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
	ClippedPolygon clipTriangle(ClippedPolygon triangle) {
		ClippedPolygon final_poly = {{}, {}, {}, {}, 0};

		ClippedPolygon* original_poly = &triangle;
		ClippedPolygon* new_poly = &final_poly;

		int new_poly_vertex_count = 0;

		// clip against each frustum plane
		for (int i = 0; i < NUM_FRUSTUM_PLANES; i++) {
			new_poly_vertex_count = 0;
			int previous = original_poly->vertex_count - 1;
			int current = 0;

			// step through each pair of vertices, deciding what to do based on whether
			// each is inside or outside the given plane
			while (current < original_poly->vertex_count) {
				Vector& v1 = original_poly->vertices[previous];
				Vector& v2 = original_poly->vertices[current];
				Vector& plane = this->frustum_planes[i];

				double& s1 = original_poly->shades[previous];
				double& s2 = original_poly->shades[current];

				double tex_u1 = original_poly->u_values[previous];
				double tex_v1 = original_poly->v_values[previous];

				double tex_u2 = original_poly->u_values[current];
				double tex_v2 = original_poly->v_values[current];

				if (insidePlane(v1, plane)) {
					if (insidePlane(v2, plane)) {
						// just add current
						new_poly->vertices[new_poly_vertex_count] = v2;
						new_poly->shades[new_poly_vertex_count] = original_poly->shades[current];
						new_poly->u_values[new_poly_vertex_count] = tex_u2;
						new_poly->v_values[new_poly_vertex_count] = tex_v2;
						new_poly_vertex_count++;
					} else {
						// add the intersect
						double t = plane.dotProduct(v1) / plane.dotProduct(v1.subtract(v2));
						new_poly->vertices[new_poly_vertex_count] = v1.add(v2.subtract(v1).scalarMultiply(t));
						new_poly->shades[new_poly_vertex_count] = s1 + (s2 - s1) * t;
						new_poly->u_values[new_poly_vertex_count] =
								(tex_u1 + (tex_u2 - tex_u1) * t);
						new_poly->v_values[new_poly_vertex_count] =
								(tex_v1 + (tex_v2 - tex_v1) * t);
						new_poly_vertex_count++;
					}
				} else {
					if (insidePlane(v2, plane)) {
						// add the intersect
						double t = plane.dotProduct(v1) / plane.dotProduct(v1.subtract(v2));
						new_poly->vertices[new_poly_vertex_count] = v1.add(v2.subtract(v1).scalarMultiply(t));
						new_poly->shades[new_poly_vertex_count] = s1 + (s2 - s1) * t;
						new_poly->u_values[new_poly_vertex_count] =
								(tex_u1 + (tex_u2 - tex_u1) * t);
						new_poly->v_values[new_poly_vertex_count] =
								(tex_v1 + (tex_v2 - tex_v1) * t);
						new_poly_vertex_count++;

						// then add current
						new_poly->vertices[new_poly_vertex_count] = v2;
						new_poly->shades[new_poly_vertex_count] = original_poly->shades[current];
						new_poly->u_values[new_poly_vertex_count] = tex_u2;
						new_poly->v_values[new_poly_vertex_count] = tex_v2;
						new_poly_vertex_count++;
					} else {
						// both previous and current are outside the plane, do nothing
					}
				}

				previous += 1;
				current += 1;
				if (previous == original_poly->vertex_count) {
					previous = 0;
				}
			}

			if (i < NUM_FRUSTUM_PLANES - 1) {
				// swap for the next plane
				ClippedPolygon* temp = original_poly;
				original_poly = new_poly;
				new_poly = temp;

				original_poly->vertex_count = new_poly_vertex_count;
				new_poly->vertex_count = 0;
			}
		}

		return final_poly;
	}

	bool isBackFace(Vector triangle_normal, Vector vertex) {
		Vector vectorToCamera = kOrigin.subtract(vertex);

		return vectorToCamera.dotProduct(triangle_normal) <= 0;
	}

	Model applyTransform(Model item) {
		Matrix worldMatrix = Matrix::makeWorldMatrix(item.scale, item.rotation, item.translation);
		Matrix finalMatrix = this->cameraMatrix.multiplyMatrix(worldMatrix);

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
						triangle.color,
						item.shades[triangle.v0],
						item.shades[triangle.v1],
						item.shades[triangle.v2],
						1 / item.vertices[triangle.v0].z,
						1 / item.vertices[triangle.v1].z,
						1 / item.vertices[triangle.v2].z,
						triangle.tex_u0,
						triangle.tex_v0,
						triangle.tex_u1,
						triangle.tex_v1,
						triangle.tex_u2,
						triangle.tex_v2,
						item.texture};

				tri.fillShaded();
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
				ClippedPolygon triangle_poly = (ClippedPolygon){
						{
							item.vertices[triangle.v0],
							item.vertices[triangle.v1],
							item.vertices[triangle.v2]
						},
						{
							item.shades[triangle.v0],
							item.shades[triangle.v1],
							item.shades[triangle.v2]
						},
						{
							triangle.tex_u0,
							triangle.tex_u1,
							triangle.tex_u2
						},
						{
							triangle.tex_v0,
							triangle.tex_v1,
							triangle.tex_v2
						},
						3};

				ClippedPolygon poly = clipTriangle(triangle_poly);

				Point projectedVertices[poly.vertex_count];

				for (int i = 0; i < poly.vertex_count; i++) {
					projectedVertices[i] = projectVertexToScreen(poly.vertices[i], viewport);
				}

				// triangulate the resulting polygon, with all triangles starting at v0
				for (int i = 1; i < poly.vertex_count - 1; i++) {
					Triangle2D new_triangle = {
							projectedVertices[0],
							projectedVertices[i],
							projectedVertices[i + 1],
							triangle.color,
							poly.shades[0],
							poly.shades[i],
							poly.shades[i + 1],
							1 / poly.vertices[0].z,
							1 / poly.vertices[i].z,
							1 / poly.vertices[i + 1].z,
							poly.u_values[0],
							poly.v_values[0],
							poly.u_values[i],
							poly.v_values[i],
							poly.u_values[i + 1],
							poly.v_values[i + 1],
							item.texture};

					new_triangle.fillShaded();
				}
			}
		}
	}

	void drawScene(Scene& scene) {
		// update camera matrix (should we check if it has changed first?)
		this->cameraMatrix = Matrix::makeCameraMatrix(scene.camera.rotation, scene.camera.translation);

		// draw the background
		// TODO: make this more interesting/dynamic
		device::clearScreen(device::color(1, 1, 1));

		for (const auto& model : scene.models) {
			drawModel(applyTransform(model), scene.camera.viewport);
		}
	}
};

#endif

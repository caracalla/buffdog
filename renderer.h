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
		renderer.frustum_planes[0] = {0, 0, -1, viewport.near_plane_distance}; // near plane
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

				double vt_u1 = original_poly->u_values[previous];
				double vt_v1 = original_poly->v_values[previous];

				double vt_u2 = original_poly->u_values[current];
				double vt_v2 = original_poly->v_values[current];

				#define addCurrentVertex(vertex, vt_u, vt_v) \
						({ \
							new_poly->vertices[new_poly_vertex_count] = vertex; \
							new_poly->shades[new_poly_vertex_count] = \
									original_poly->shades[current]; \
							new_poly->u_values[new_poly_vertex_count] = vt_u; \
							new_poly->v_values[new_poly_vertex_count] = vt_v; \
							new_poly_vertex_count++; \
						})

				if (insidePlane(v1, plane)) {
					if (insidePlane(v2, plane)) {
						// just add current
						addCurrentVertex(v2, vt_u2, vt_v2);
					} else {
						// add the intersect
						double nDotV = plane.dotProduct(v1.subtract(v2));

						// nDotV is 0 if v1->v2 is parallel to the plane.  This shouldn't be
						// possible if one is in and one is out, but who knows.  As a fallback,
						// we'll just add the current vertex since it's basically inside anyways
						// ... right?
						if (nDotV != 0) {
							double t = plane.dotProduct(v1) / nDotV;
							new_poly->vertices[new_poly_vertex_count] =
									v1.add(v2.subtract(v1).scalarMultiply(t));
							new_poly->shades[new_poly_vertex_count] = s1 + (s2 - s1) * t;
							new_poly->u_values[new_poly_vertex_count] =
									(vt_u1 + (vt_u2 - vt_u1) * t);
							new_poly->v_values[new_poly_vertex_count] =
									(vt_v1 + (vt_v2 - vt_v1) * t);
							new_poly_vertex_count++;
						} else {
							addCurrentVertex(v2, vt_u2, vt_v2);
						}
					}
				} else {
					if (insidePlane(v2, plane)) {
						// add the intersect
						double nDotV = plane.dotProduct(v1.subtract(v2));

						if (nDotV != 0) {
							double t = plane.dotProduct(v1) / nDotV;
							new_poly->vertices[new_poly_vertex_count] =
									v1.add(v2.subtract(v1).scalarMultiply(t));
							new_poly->shades[new_poly_vertex_count] = s1 + (s2 - s1) * t;
							new_poly->u_values[new_poly_vertex_count] =
									(vt_u1 + (vt_u2 - vt_u1) * t);
							new_poly->v_values[new_poly_vertex_count] =
									(vt_v1 + (vt_v2 - vt_v1) * t);
							new_poly_vertex_count++;
						}

						// then add current
						addCurrentVertex(v2, vt_u2, vt_v2);
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

			if (new_poly_vertex_count == 0) {
				// clipped out of existence
				return *new_poly;
			}

			new_poly->vertex_count = new_poly_vertex_count;

			if (i < NUM_FRUSTUM_PLANES - 1) {
				// swap for the next plane
				ClippedPolygon* temp = original_poly;
				original_poly = new_poly;
				new_poly = temp;

				new_poly->vertex_count = 0;
			}
		}

		return *new_poly;
	}

	bool isBackFace(Vector triangle_normal, Vector vertex) {
		Vector vectorToCamera = kOrigin.subtract(vertex);

		return vectorToCamera.dotProduct(triangle_normal) <= 0;
	}

	double applyLighting(Vector& normal, std::vector<Light>& lights) {
		double result = 0;

		for (auto& light : lights) {
			if (light.type == LightType::directional) {
				double directional_light = normal.dotProduct(light.direction);

				if (directional_light > 0) {
					result += directional_light * light.intensity;
				}
			} else {
				result += light.intensity;
			}
		}

		return result;
	}

	void drawModel(Model item, Viewport& viewport, std::vector<Light>& lights) {
		bool isVertexVisible[item.vertices.size()];
		Point projectedVertices[item.vertices.size()];

		for (int i = 0; i < item.vertices.size(); i++) {
			isVertexVisible[i] = insideFrustum(item.vertices[i]);

			if (isVertexVisible[i]) {
				projectedVertices[i] = projectVertexToScreen(item.vertices[i], viewport);
			}
		}

		for (auto& triangle : item.triangles) {
			if (isBackFace(triangle.normal, item.vertices[triangle.v0.index])) {
				// this is a back face, don't draw
				continue;
			}

			if (item.normals.size() > 0) {
				triangle.v0.light_intensity = applyLighting(
						item.normals[triangle.v0.normal], lights);
				triangle.v1.light_intensity = applyLighting(
						item.normals[triangle.v1.normal], lights);
				triangle.v2.light_intensity = applyLighting(
						item.normals[triangle.v2.normal], lights);
			} else {
				triangle.v0.light_intensity = applyLighting(triangle.normal, lights);
				triangle.v1.light_intensity = applyLighting(triangle.normal, lights);
				triangle.v2.light_intensity = applyLighting(triangle.normal, lights);
			}

			if (isVertexVisible[triangle.v0.index] &&
					isVertexVisible[triangle.v1.index] &&
					isVertexVisible[triangle.v2.index]) {
				// all vertices are visible
				Triangle2D tri = {
						projectedVertices[triangle.v0.index],
						projectedVertices[triangle.v1.index],
						projectedVertices[triangle.v2.index],
						triangle.color,
						triangle.v0.light_intensity,
						triangle.v1.light_intensity,
						triangle.v2.light_intensity,
						1 / item.vertices[triangle.v0.index].z,
						1 / item.vertices[triangle.v1.index].z,
						1 / item.vertices[triangle.v2.index].z,
						item.uvs[triangle.v0.uv].first,
						item.uvs[triangle.v0.uv].second,
						item.uvs[triangle.v1.uv].first,
						item.uvs[triangle.v1.uv].second,
						item.uvs[triangle.v2.uv].first,
						item.uvs[triangle.v2.uv].second,
						item.has_texture && !triangle.special ? item.texture : nullptr};

				// tri.draw();
				tri.fillShaded();
				// tri.fillBarycentric();
			} else {
				// not all vertices are visible
				// it's clipping time
				ClippedPolygon triangle_poly = ClippedPolygon{
						{
							item.vertices[triangle.v0.index],
							item.vertices[triangle.v1.index],
							item.vertices[triangle.v2.index]
						},
						{
							triangle.v0.light_intensity,
							triangle.v1.light_intensity,
							triangle.v2.light_intensity,
						},
						{
							item.uvs[triangle.v0.uv].first,
							item.uvs[triangle.v1.uv].first,
							item.uvs[triangle.v2.uv].first,
						},
						{
							item.uvs[triangle.v0.uv].second,
							item.uvs[triangle.v1.uv].second,
							item.uvs[triangle.v2.uv].second,
						},
						3};

				ClippedPolygon poly = clipTriangle(triangle_poly);

				if (poly.vertex_count == 0) {
					// clipped out of existence, move on
					continue;
				}

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
							item.has_texture && !triangle.special ? item.texture : nullptr};

					// new_triangle.draw();
					new_triangle.fillShaded();
					// new_triangle.fillBarycentric();
				}
			}
		}
	}

	Model transformModel(Model item) {
		Matrix worldMatrix = Matrix::makeWorldMatrix(
				item.scale, item.rotation, item.position);
		Matrix finalMatrix = this->cameraMatrix.multiplyMatrix(worldMatrix);

		// transform vertices into camera space
		for (auto& vertex : item.vertices) {
			vertex = finalMatrix.multiplyVector(vertex);
		}

		// transform normals
		Matrix normalTransformationMatrix =
				this->cameraMatrix.multiplyMatrix(Matrix::makeRotationMatrix(item.rotation));

		for (auto& normal : item.normals) {
			normal = normalTransformationMatrix.multiplyVector(normal);
		}

		for (auto& triangle : item.triangles) {
			triangle.normal = normalTransformationMatrix.multiplyVector(triangle.normal);
		}

		return item;
	}

	Model transformLevel(Model level) {
		// levels are already in world space, so just do a camera transformations

		// transform vertices into camera space
		for (auto& vertex : level.vertices) {
			vertex = this->cameraMatrix.multiplyVector(vertex);
		}

		for (auto& normal : level.normals) {
			normal = this->cameraMatrix.multiplyVector(normal);
		}

		for (auto& triangle : level.triangles) {
			triangle.normal = this->cameraMatrix.multiplyVector(triangle.normal);
		}

		return level;
	}

	void drawScene(Scene& scene) {
		// update camera matrix (should we check if it has changed first?)
		this->cameraMatrix = Matrix::makeCameraMatrix(
				scene.camera.rotation, scene.camera.position);

		// draw the background
		// TODO: make this more interesting/dynamic
		device::clearScreen(device::color(1, 1, 1));

		std::vector<Light> lights = scene.lights;

		for (auto& light : lights) {
			if (light.type == LightType::directional) {
				light.direction = this->cameraMatrix.multiplyVector(light.direction);
			}
		}

		// the level geometry is handled differently from the models
		drawModel(transformLevel(scene.level.model), scene.camera.viewport, lights);

		for (const auto& model : scene.models) {
			drawModel(transformModel(model), scene.camera.viewport, lights);
		}
	}
};

#endif

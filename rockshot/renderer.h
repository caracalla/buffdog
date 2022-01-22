#ifndef BUFFDOG_RENDERER
#define BUFFDOG_RENDERER

#include <array>
#include <vector>

#include "../device.h"
#include "../matrix.h"
#include "../vector.h"

#include "entity.h"
#include "model.h"
#include "scene.h"
#include "triangle.h"

#define NUM_FRUSTUM_PLANES 6
// the max potential vertices for a triangle clipped against six planes is 9
#define MAX_CLIPPED_POLYGON_VERTICES 9

// Visual C++ is unhappy with dynamic automatic array sizes, so I'll just do
// this for now.  This is effectively the upper limit on the number of vertices
// that can be displayed
#define TEMP_ARRAY_SIZE 4096

// set this to 1 to draw triangles with the fillBarycentric method
#define USE_BARYCENTRIC 0



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
	Matrix camera_matrix;

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
		ClippedPolygon temp_poly = {{}, {}, {}, {}, 0};

		ClippedPolygon* original_poly = &triangle;
		ClippedPolygon* new_poly = &temp_poly;

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
						new_poly->vertices[new_poly_vertex_count] = vertex; \
						new_poly->shades[new_poly_vertex_count] = \
								original_poly->shades[current]; \
						new_poly->u_values[new_poly_vertex_count] = vt_u; \
						new_poly->v_values[new_poly_vertex_count] = vt_v; \
						new_poly_vertex_count++; \

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
		// position of the camera after all transforms is always at the origin
		Vector vertex_to_camera = Vector::origin().subtract(vertex);

		return vertex_to_camera.dotProduct(triangle_normal) <= 0;
	}

	double applyLighting(Vector& normal, std::vector<Light>& lights) {
		double result = 0;

		for (auto& light : lights) {
			if (light.type == Light::Type::directional) {
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

	// draw the axes as a helpful diagram in front of the player
	void drawPointers(Camera& camera) {
		Vector cameraDirection = Matrix::makeRotationMatrix(camera.rotation).
				multiplyVector(Vector::direction(0, 0, -1)).unit();

		Vector offset = cameraDirection.scalarMultiply(2);
		Vector position = camera.position.add(offset);

		Vector x = position.add(Vector::direction(0.5, 0, 0));
		Vector x1 = position.add(Vector::direction(0.45, 0.05, 0));
		Vector x2 = position.add(Vector::direction(0.45, -0.05, 0));
		Vector y = position.add(Vector::direction(0, 0.5, 0));
		Vector y1 = position.add(Vector::direction(0, 0.45, 0.05));
		Vector y2 = position.add(Vector::direction(0, 0.45, -0.05));
		Vector z = position.add(Vector::direction(0, 0, 0.5));
		Vector z1 = position.add(Vector::direction(0.05, 0, 0.45));
		Vector z2 = position.add(Vector::direction(-0.05, 0, 0.45));

		position = this->camera_matrix.multiplyVector(position);
		x = this->camera_matrix.multiplyVector(x);
		x1 = this->camera_matrix.multiplyVector(x1);
		x2 = this->camera_matrix.multiplyVector(x2);
		y = this->camera_matrix.multiplyVector(y);
		y1 = this->camera_matrix.multiplyVector(y1);
		y2 = this->camera_matrix.multiplyVector(y2);
		z = this->camera_matrix.multiplyVector(z);
		z1 = this->camera_matrix.multiplyVector(z1);
		z2 = this->camera_matrix.multiplyVector(z2);

		Point po = projectVertexToScreen(position, camera.viewport);

		// draw line from the "origin" along the x axis
		Point px = projectVertexToScreen(x, camera.viewport);
		drawLine(po, px, device::getColorValue(1.0, 0.0, 0.0));
		// draw two little lines from the end of the line, to make an arrow
		Point px1 = projectVertexToScreen(x1, camera.viewport);
		Point px2 = projectVertexToScreen(x2, camera.viewport);
		drawLine(px, px1, device::getColorValue(1.0, 0.0, 0.0));
		drawLine(px, px2, device::getColorValue(1.0, 0.0, 0.0));
		drawLine(px1, px2, device::getColorValue(1.0, 0.0, 0.0));

		Point py = projectVertexToScreen(y, camera.viewport);
		drawLine(po, py, device::getColorValue(0.0, 1.0, 0.0));
		Point py1 = projectVertexToScreen(y1, camera.viewport);
		Point py2 = projectVertexToScreen(y2, camera.viewport);
		drawLine(py, py1, device::getColorValue(0.0, 1.0, 0.0));
		drawLine(py, py2, device::getColorValue(0.0, 1.0, 0.0));
		drawLine(py1, py2, device::getColorValue(0.0, 1.0, 0.0));

		Point pz = projectVertexToScreen(z, camera.viewport);
		drawLine(po, pz, device::getColorValue(0.0, 0.0, 1.0));
		Point pz1 = projectVertexToScreen(z1, camera.viewport);
		Point pz2 = projectVertexToScreen(z2, camera.viewport);
		drawLine(pz, pz1, device::getColorValue(0.0, 0.0, 1.0));
		drawLine(pz, pz2, device::getColorValue(0.0, 0.0, 1.0));
		drawLine(pz1, pz2, device::getColorValue(0.0, 0.0, 1.0));
	}

	void drawModel(
			Model item,
			Viewport& viewport,
			std::vector<Light>& lights,
			int translucency) {
		// bool is_vertex_visible[item.vertices.size()];
		// Point projected_vertices[item.vertices.size()];
		bool is_vertex_visible[TEMP_ARRAY_SIZE];
		Point projected_vertices[TEMP_ARRAY_SIZE];

		for (int i = 0; i < item.vertices.size(); i++) {
			is_vertex_visible[i] = insideFrustum(item.vertices[i]);

			if (is_vertex_visible[i]) {
				projected_vertices[i] = projectVertexToScreen(item.vertices[i], viewport);
			}
		}

		for (auto& triangle : item.triangles) {
			Vector triangleNormal = triangle.getNormal();

			if (isBackFace(triangleNormal, item.vertices[triangle.v0.index])) {
				// this is a back face, don't draw
				continue;
			}

			if (item.compute_lighting) {
				if (item.normals.size() > 0) {
					triangle.v0.light_intensity = applyLighting(
							item.normals[triangle.v0.normal], lights);
					triangle.v1.light_intensity = applyLighting(
							item.normals[triangle.v1.normal], lights);
					triangle.v2.light_intensity = applyLighting(
							item.normals[triangle.v2.normal], lights);
				} else {
					triangle.v0.light_intensity = applyLighting(triangleNormal, lights);
					triangle.v1.light_intensity = applyLighting(triangleNormal, lights);
					triangle.v2.light_intensity = applyLighting(triangleNormal, lights);
				}
			}

			Texture* texture = nullptr;

			if (item.has_texture && !triangle.ignore_texture) {
				texture = item.texture;
			}

			if (is_vertex_visible[triangle.v0.index] &&
					is_vertex_visible[triangle.v1.index] &&
					is_vertex_visible[triangle.v2.index]) {
				// all vertices are visible
				Triangle2D tri = {
						projected_vertices[triangle.v0.index],
						projected_vertices[triangle.v1.index],
						projected_vertices[triangle.v2.index],
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
						texture,
						translucency};

				// tri.draw();
#if USE_BARYCENTRIC
				tri.fillBarycentric();
#else
				tri.fillShaded();
#endif
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

				// Point projected_vertices[poly.vertex_count];
				Point projected_vertices[TEMP_ARRAY_SIZE];

				for (int i = 0; i < poly.vertex_count; i++) {
					projected_vertices[i] = projectVertexToScreen(poly.vertices[i], viewport);
				}

				// triangulate the resulting polygon, with all triangles starting at v0
				for (int i = 1; i < poly.vertex_count - 1; i++) {
					Triangle2D new_triangle = {
							projected_vertices[0],
							projected_vertices[i],
							projected_vertices[i + 1],
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
							texture,
							translucency};

					// new_triangle.draw();
					#if USE_BARYCENTRIC
									new_triangle.fillBarycentric();
					#else
									new_triangle.fillShaded();
					#endif
				}
			}
		}
	}

	Model buildCameraModel(Entity& item) {
		Model result = *item.model;

		Matrix worldMatrix = Matrix::makeWorldMatrix(
				item.scale, item.actualRotation(), item.position);
		Matrix finalMatrix = this->camera_matrix.multiplyMatrix(worldMatrix);

		// transform vertices into camera space
		for (auto& vertex : result.vertices) {
			vertex = finalMatrix.multiplyVector(vertex);
		}

		// transform normals
		Matrix normalTransformationMatrix =
				this->camera_matrix.multiplyMatrix(Matrix::makeRotationMatrix(item.rotation));

		for (auto& normal : result.normals) {
			normal = normalTransformationMatrix.multiplyVector(normal);
		}

		for (auto& triangle : result.triangles) {
			triangle.normal = normalTransformationMatrix.multiplyVector(triangle.normal);
		}

		return result;
	}

	Model buildCameraModelFromWorldModel(Model model) {
		// levels are already in world space, so just do a camera transformations

		// transform vertices into camera space
		for (auto& vertex : model.vertices) {
			vertex = this->camera_matrix.multiplyVector(vertex);
		}

		for (auto& normal : model.normals) {
			normal = this->camera_matrix.multiplyVector(normal);
		}

		for (auto& triangle : model.triangles) {
			triangle.normal = this->camera_matrix.multiplyVector(triangle.normal);
		}

		return model;
	}

	void drawScene(Scene& scene) {
		// update camera matrix (should we check if it has changed first?)
		this->camera_matrix = Matrix::makeCameraMatrix(
				scene.camera.rotation, scene.camera.position);

		// draw the background
		// TODO: make this more interesting/dynamic
		device::clearScreen(device::getColorValue(1.0, 1.0, 1.0));

		// move the lights into camera space
		std::vector<Light> lights = scene.lights;

		for (auto& light : lights) {
			if (light.type == Light::Type::directional) {
				light.direction = this->camera_matrix.multiplyVector(light.direction);
			}
		}

		// draw level
		drawModel(
				buildCameraModelFromWorldModel(scene.level.model_in_world),
						scene.camera.viewport,
						lights,
						scene.level.translucency);

		// draw player
		drawModel(
				buildCameraModelFromWorldModel(scene.player.model_in_world),
						scene.camera.viewport,
						lights,
						scene.player.translucency);

		// draw player's weapon
		drawModel(
				buildCameraModelFromWorldModel(scene.player.weapon.model_in_world),
						scene.camera.viewport,
						lights,
						scene.player.weapon.translucency);

		// draw entities
		for (auto& entity : scene.entities) {
			if (entity.active) {
				drawModel(
						buildCameraModelFromWorldModel(entity.model_in_world),
								scene.camera.viewport,
								lights,
								entity.translucency);
			}
		}

		drawPointers(scene.camera);
	}
};

#endif

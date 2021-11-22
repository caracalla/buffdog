// usleep
#define _BSD_SOURCE
#include <unistd.h>

#include <cstdlib>
#include <cmath>
#include <vector>

#include "libdt.h"
#include "line.h"


#define DELAY_US 10000
#define DEBUG_ILLEGAL_TRIANGLES 0

int yres = device::getYRes();


int main() {
	if (!device::setUp()) {
		return 1;
	}

	initDT();

#if DEBUG_ILLEGAL_TRIANGLES

	// This version used to show the process of stepping through illegal
	// triangles in a little more detail
	while (device::running()) {
		usleep(DELAY_US);

		// device::clearScreen(device::getColorValue(0.1, 0.1, 0.1));

		key_input key = device::getNextKey();

		if (key == z_key) {
			initDT();
		}

		mouse_input mouse = device::getMouseMotion();

		device::clearScreen(device::getColorValue(0.1, 0.1, 0.1));
		drawDT();

		if (device::insideViewport(mouse.pos_x, mouse.pos_y)) {
			Point mouse_pos = Point{mouse.pos_x, yres - mouse.pos_y};

			TriangleDT* leaf_triangle =
					dt_bounding_triangle.findLeafContainingPoint(mouse_pos);

			if (leaf_triangle) { // && !leaf_triangle->touchesBoundingPoint()) {
				Vector tri_color{0.0, 0.8, 0.0};
				Vector illegal_tri_color{0.8, 0.0, 0.8};

				for (auto edge : edges_to_legalize) {
					edge.second->parent_triangle->drawImpl(illegal_tri_color, true);

					if (leaf_triangle && leaf_triangle->containsHalfEdge(edge.second)) {
						tri_color = Vector{0.0, 0.8, 0.8};
					}
				}

				leaf_triangle->drawImpl(tri_color, true);
			}

			Vector mouse_color{1.0, 0.0, 1.0};
			drawPoint(mouse_pos, colorFromVector(mouse_color));

			if (key == mouse_1) {
				if (edges_to_legalize.empty()) {
					addPointToDT(mouse_pos);
					printf("number of triangles: %lu\n", dt_triangles.size());
					printf("new number of half edges to legalize: %lu\n", edges_to_legalize.size());
				} else {
					// auto edge_to_legalize = edges_to_legalize[leaf_triangle];
					auto it = edges_to_legalize.find(leaf_triangle);

					if (it != edges_to_legalize.end()) {
						auto edge_to_legalize = it->second;
						legalizeEdgeImpl(edge_to_legalize);
						edges_to_legalize.erase(leaf_triangle);

						printf("number of half edges left to legalize: %lu\n", edges_to_legalize.size());
					}
				}
			}
		}

		Point circumcenter = dt_bounding_triangle.circumcenter();
		if (device::insideViewport(circumcenter.x, circumcenter.y)) {
			drawPoint(circumcenter, device::getColorValue(1.0, 0.0, 1.0));
		}

		device::updateScreen();
		device::processInput();
	}

#else

	while (device::running()) {
		usleep(DELAY_US);

		// press Z to reset the triangulation to a fresh state
		key_input key = device::getNextKey();
		if (key == z_key) {
			initDT();
		}

		mouse_state& mouse = device::getInputState()->mouse;

		device::clearScreen(device::getColorValue(0.1, 0.1, 0.1));
		drawDT();

		if (device::insideViewport(mouse.pos_x, mouse.pos_y)) {
			// convert mouse coordinates to origin at bottom left
			Point mouse_pos = Point{mouse.pos_x, yres - mouse.pos_y};

			// if the cursor is inside of an inner triangle, draw it filled green
			TriangleDT* leaf_triangle = dt_bounding_triangle.findLeafContainingPoint(mouse_pos);
			if (leaf_triangle && !leaf_triangle->touchesBoundingPoint()) {
				leaf_triangle->drawImpl(Vector::color(0.0, 0.8, 0.0), true);
			}

			drawPoint(mouse_pos, colorFromVector(Vector::color(1.0, 0.0, 1.0)));

			if (key == mouse_1) {
				addPointToDT(mouse_pos);
			}

			std::vector<TriangleDT*> leaves = leafTriangles();
			device::logOncePerSecond("number of leaf triangles: %d\n", leaves.size());
		}

		device::updateScreen();
		device::processInput();
	}

#endif

	device::tearDown();

	return 0;
}

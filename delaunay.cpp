// usleep
#define _BSD_SOURCE
#include <unistd.h>

#include <cstdlib>
#include <cmath>
#include <vector>

#include "libdt.h"
#include "line.h"
#include "spinner.h"


#define DELAY_US 10000


int main() {
	if (!device::setUp()) {
		return 1;
	}

	initDT();

	while (device::running()) {
		usleep(DELAY_US);

		// device::clearScreen(device::color(0.1, 0.1, 0.1));s

		key_input key = device::get_next_key();

		if (key == z_key) {
			initDT();
		}

		mouse_input mouse = device::getMouseMotion();
		int yres = device::getYRes();

		device::clearScreen(device::color(0.1, 0.1, 0.1));

		dt_bounding_triangle.draw();

		if (device::insideViewport(mouse.pos_x, mouse.pos_y)) {
			Point mouse_pos = Point{mouse.pos_x, yres - mouse.pos_y};

			TriangleDT* leaf_triangle = dt_bounding_triangle.findLeafContainingPoint(mouse_pos);

			if (leaf_triangle && !leaf_triangle->touchesBoundingPoint()) {
				Vector tri_color{0.0, 0.8, 0.0};

				leaf_triangle->drawImpl(tri_color, true);
			}

			Vector mouse_color{1.0, 0.0, 1.0};
			drawPoint(mouse_pos, colorFromVector(mouse_color));

			if (key == x_key) {
				addPointToDT(mouse_pos);
			}
		}

		device::updateScreen();
		device::processInput();
	}

#if 0

	while (device::running()) {
		usleep(DELAY_US);

		// device::clearScreen(device::color(0.1, 0.1, 0.1));s

		key_input key = device::get_next_key();

		if (key == z_key) {
			initDT();
		}

		mouse_input mouse = device::getMouseMotion();
		int yres = device::getYRes();

		device::clearScreen(device::color(0.1, 0.1, 0.1));
		drawDT();

		if (device::insideViewport(mouse.pos_x, mouse.pos_y)) {
			Point mouse_pos = Point{mouse.pos_x, yres - mouse.pos_y};

			TriangleDT* leaf_triangle = dt_bounding_triangle.findLeafContainingPoint(mouse_pos);

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

			if (key == x_key) {
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
			drawPoint(circumcenter, device::color(1.0, 0.0, 1.0));
		}

		device::updateScreen();
		device::processInput();
	}

#endif

	device::tearDown();

	return 0;
}
#ifndef BUFFDOG_LIBDT
#define BUFFDOG_LIBDT

#include <array>
#include <cstdlib>
#include <list>
#include <map>
#include <unordered_set>

#include "../circle.h"
#include "../device.h"
#include "../point.h"
#include "../vector.h"

#include "triangle.h"


// The sentinel value, for example for half-edges belonging to the bounding
// triangle that have no opposite half-edge
#define NULL_INDEX INT_MAX

// this is supposed to be useful for preventing really skinny triangles, but I
// had to set it super low because the barycentric logic gets funky with values
// like 0.01
#define EPSILON 0.00001


double distanceBetweenPoints(Point p1, Point p2) {
	int x_diff = p1.x - p2.x;
	int y_diff = p1.y - p2.y;
	return sqrt(x_diff * x_diff + y_diff * y_diff);
}


struct TriangleDT;

// Half-edges are directional lines pointing from start_point to end_point
// For all half-edges except those for the bounding triangle, there must be a
// corresponding edge going in the opposite direction
struct HalfEdge {
	TriangleDT* parent_triangle;
	size_t start_point = NULL_INDEX;
	size_t end_point = NULL_INDEX;

	// the point in the half-edge's triangle opposite this half-edge
	size_t opposite_point = NULL_INDEX;

	// the half edge for the same points, but going in the opposite direction
	// will be on a different triangle
	HalfEdge* opposite_half_edge = nullptr;
	HalfEdge* next_half_edge = nullptr;

	bool is_bounding = false;

	void log() {
		// printf(
		// 		"    start_point: %d\n"
		// 		"    end_point: %d\n"
		// 		"    opposite_point: %d\n",
		// 		this->start_point,
		// 		this->end_point,
		// 		this->opposite_point);
	}
};


// Triangles are made up of three half edges, and form a DAG:
// * The root of the DAG is the bounding triangle
// * Leaves are valid triangles
// * Intermediate triangles (between the root and the leaves) have been
//   "destroyed" by the addition of a point or as a result of flipping
struct TriangleDT {
	HalfEdge* half_edge_1 = nullptr;
	HalfEdge* half_edge_2 = nullptr;
	HalfEdge* half_edge_3 = nullptr;

	// the DAG
	std::vector<TriangleDT*> child_triangles;

	void log() {
		printf("half_edge_1:\n");
		this->half_edge_1->log();
		printf("half_edge_2:\n");
		this->half_edge_2->log();
		printf("half_edge_3:\n");
		this->half_edge_3->log();

		spit("\n");
	}

	// implemented below, since points are stored in a global vector
	std::array<Point, 3> points();

	std::array<HalfEdge*, 3> halfEdges() {
		return std::array<HalfEdge*, 3>{
				this->half_edge_1,
				this->half_edge_2,
				this->half_edge_3};
	}

	bool contains(Point point) {
		// define the bounding box containing the triangle
		auto triangle_points = this->points();

		Vector weights = getBarycentricWeights(
				triangle_points[0],
				triangle_points[1],
				triangle_points[2],
				point.x,
				point.y);

		return weights.x > EPSILON && weights.y > EPSILON && weights.z > EPSILON;
	}

	bool containsHalfEdge(HalfEdge* edge) {
		return edge == this->half_edge_1 || edge == this->half_edge_2 || edge == this->half_edge_3;
	}

	bool hasBoundingEdge() {
		return
				this->half_edge_1->is_bounding ||
				this->half_edge_2->is_bounding ||
				this->half_edge_3->is_bounding;
	}

	bool touchesBoundingPoint() {
		if (this->hasBoundingEdge()) {
			// bounding edges touch bounding points
			return true;
		}

		for (int i = 0; i < 3; i++) {
			if (this->half_edge_1->start_point == i ||
					this->half_edge_2->start_point == i ||
					this->half_edge_3->start_point == i) {
				return true;
			}
		}

		return false;
	}

	Point circumcenter() {
		auto points = this->points();
		Point p1 = points[0];
		Point p2 = points[1];
		Point p3 = points[2];

		// | a b | * | x | = | g |
		// | c d |   | y | = | h |

		int a = (p2.x * 2) - (p1.x * 2);
		int b = (p2.y * 2) - (p1.y * 2);
		int c = (p3.x * 2) - (p1.x * 2);
		int d = (p3.y * 2) - (p1.y * 2);

		int g = (p2.x * p2.x) + (p2.y * p2.y) - (p1.x * p1.x) - (p1.y * p1.y);
		int h = (p3.x * p3.x) + (p3.y * p3.y) - (p1.x * p1.x) - (p1.y * p1.y);

		double determinant = 1.0 / (a * d - b * c);
		double inverse_a = determinant * d;
		double inverse_b = determinant * -b;
		double inverse_c = determinant * -c;
		double inverse_d = determinant * a;

		// | x | = | i_a i_b | * | g |
		// | y | = | i_c i_d |   | h |

		int x = inverse_a * g + inverse_b * h;
		int y = inverse_c * g + inverse_d * h;

		return Point{x, y};
	}

	Circle circumcircle() {
		Circle result;

		result.center = this->circumcenter();

		auto points = this->points();
		// all points are equidistant from circumcenter
		result.radius = distanceBetweenPoints(result.center, points[0]);

		result.color = Vector{0.0, 1.0, 0.0};

		return result;
	}

	bool inCircumcircle(Point point) {
		Circle circumcircle = this->circumcircle();

		return distanceBetweenPoints(circumcircle.center, point) < circumcircle.radius;
	}

	TriangleDT* findLeafContainingPoint(Point point) {
		if (!this->contains(point)) {
			return nullptr;
		}

		// this triangle contains the point, and it's a leaf, so it's our winner
		if (this->child_triangles.size() == 0) {
			return this;
		}

		for (auto child_triangle : this->child_triangles) {
			TriangleDT* result = child_triangle->findLeafContainingPoint(point);

			if (result) {
				return result;
			}
		}

		// a parent triangle that contains a point must have a child that also
		// contains that point, so we should never get here;

		// spit("got to the weird case");

		// however, since we want to avoid points that are right on the edge between
		// two triangles, TriangleDT::contains() isn't exact, so it might be possible
		// to pass for one triangle and not for another (I think), so returning
		// nullptr here should just tell us to give up on this point
		return nullptr;
	}

	void insertPoint(size_t point_index);

	std::unordered_set<TriangleDT*> leafTriangles() {
		std::unordered_set<TriangleDT*> leaf_triangles;

		if (this->child_triangles.size() == 0) {
			leaf_triangles.emplace(this);
		} else {
			for (auto child_triangle : this->child_triangles) {
				auto child_leaves = child_triangle->leafTriangles();
				leaf_triangles.merge(child_leaves);
			}
		}

		return leaf_triangles;
	}

	void draw() {
		// only draw leaf triangles
		if (this->child_triangles.size() == 0) {
			if (!this->touchesBoundingPoint()) {
				this->drawImpl(Vector{0.0, 0.0, 0.5}, false); // true);
				this->drawImpl(Vector{0.8, 0.0, 0.0}, false);
			} else {
				// don't draw the bounding triangle or any of its children that touch
				// bounding points, as they will be outside of the viewport
				// (assuming BUILD_TRIANGLE_DEBUG is 0)
				// this->drawImpl(Vector{0.0, 0.8, 0.0}, false);
			}


		} else {
			for (auto child_triangle : this->child_triangles) {
				child_triangle->draw();
			}
		}
	}

	void drawImpl(Vector color, bool filled) {
		auto points = this->points();
		auto circumcenter = this->circumcenter();

		if (device::insideViewport(circumcenter.x, circumcenter.y)) {
			drawPoint(circumcenter, device::getColorValue(1.0, 1.0, 1.0));
		}

		Triangle2D tri{
				points[0],
				points[1],
				points[2],
				color};

		filled ? tri.fill() : tri.draw();
	}

	void drawCircumcircle() {
		this->circumcircle().draw();
	}
};


std::vector<Point> dt_points;
std::list<HalfEdge> dt_half_edges;

TriangleDT dt_bounding_triangle;

// container for children of dt_bounding_triangle
std::list<TriangleDT> dt_triangles;


// must run first to generate bounding triangle points
std::vector<Point> generateDTPoints(int count) {
	std::vector<Point> result;

	int xres = device::getXRes();
	int yres = device::getYRes();

	// the first three points will always be the bounding triangle
#define BUILD_TRIANGLE_DEBUG 0

#if BUILD_TRIANGLE_DEBUG
	int min_x = xres / 16;
	int min_y = yres / 16;
	int max_x = xres - min_x;
	int max_y = yres - min_y;

	Point bounding1{max_x, min_y};
	Point bounding2{min_x, max_y};
	Point bounding3{min_x, min_y};
#else
	double bounding_scaling_factor = 9.0;
	int bounding_x = xres * bounding_scaling_factor;
	int bounding_y = yres * bounding_scaling_factor;

	Point bounding1{bounding_x, 0};
	Point bounding2{0, bounding_y};
	Point bounding3{-bounding_x, -bounding_y};
#endif

	result.push_back(bounding1);
	result.push_back(bounding2);
	result.push_back(bounding3);

	// uncomment to just make a triangulation right away
	// for (int i = 0; i < count; i++) {
	// 	Point point{
	// 			device::randomInt(min_x, max_x),
	// 			device::randomInt(min_y, max_y)};
	//
	// 	result.push_back(point);
	// }

	return result;
}

// must be run after populating dt_points but before anything else
TriangleDT buildBoundingTriangle() {
	TriangleDT result;

	size_t point_A = 0;
	size_t point_B = 1;
	size_t point_C = 2;

	dt_half_edges.push_back(HalfEdge{});
	result.half_edge_1 = &(dt_half_edges.back());
	dt_half_edges.push_back(HalfEdge{});
	result.half_edge_2 = &(dt_half_edges.back());
	dt_half_edges.push_back(HalfEdge{});
	result.half_edge_3 = &(dt_half_edges.back());

	result.half_edge_1->parent_triangle = &result;
	result.half_edge_1->start_point = point_A;
	result.half_edge_1->end_point = point_B;
	result.half_edge_1->opposite_point = point_C;
	result.half_edge_1->opposite_half_edge = nullptr;
	result.half_edge_1->next_half_edge = result.half_edge_2;
	result.half_edge_1->is_bounding = true;

	result.half_edge_2->parent_triangle = &result;
	result.half_edge_2->start_point = point_B;
	result.half_edge_2->end_point = point_C;
	result.half_edge_2->opposite_point = point_A;
	result.half_edge_2->opposite_half_edge = nullptr;
	result.half_edge_2->next_half_edge = result.half_edge_3;
	result.half_edge_2->is_bounding = true;

	result.half_edge_3->parent_triangle = &result;
	result.half_edge_3->start_point = point_C;
	result.half_edge_3->end_point = point_A;
	result.half_edge_3->opposite_point = point_B;
	result.half_edge_3->opposite_half_edge = nullptr;
	result.half_edge_3->next_half_edge = result.half_edge_1;
	result.half_edge_3->is_bounding = true;

	return result;
}

std::map<TriangleDT*, HalfEdge*> edges_to_legalize;

void legalizeEdgeImpl(HalfEdge* half_edge);

bool linesIntersect(Point start1, Point end1, Point start2, Point end2) {
	double x_s1 = start1.x;
	double y_s1 = start1.y;

	double x_e1 = end1.x;
	double y_e1 = end1.y;

	double x_s2 = start2.x;
	double y_s2 = start2.y;

	double x_e2 = end2.x;
	double y_e2 = end2.y;

	// we only care if the intersection point is between start1 and end1
	double t_numerator = (x_s1 - x_s2) * (y_s2 - y_e2) - (y_s1 - y_s2) * (x_s2 - x_e2);
	double t_denominator = (x_s1 - x_e1) * (y_s2 - y_e2) - (y_s1 - y_e1) * (x_s2 - x_e2);

	double t = t_numerator / t_denominator;

	return t >= 0.0 && t <= 1.0;
}

void legalizeEdge(HalfEdge* half_edge) {
	HalfEdge* opposite_edge = half_edge->opposite_half_edge;

	// special cases for triangles touching bounding points:
	// 1. half_edge, opposite_edge, or any edges for either of their trianges are bounding edges -> always legal
	// 2. neither point of half_edge is a bounding point, but one of its opposite points is -> always legal?
	// 3. one of the two points of half_edge is a bounding point:
	//   * hypothetical flipped edge intersects with half_edge -> never legal
	//   * hypothetical flipped edge does not intersect with half_edge -> always legal

	// 1. if any of the half edges for this triangle or its opposite are bounding
	// edges, it's legal
	if (
			half_edge->parent_triangle->hasBoundingEdge() ||
			opposite_edge->parent_triangle->hasBoundingEdge()
	) {
		// spit("  should not flip SPECIAL 1");
		return;
	}

	bool definitely_flip = false;

	for (int i = 0; i < 3; i++) {
		// 2. if neither point of the half edge is a bounding point, but one of its
		// opposite points is, it's legal
		if (half_edge->opposite_point == i || opposite_edge->opposite_point == i) {
			// I don't think it's actually possible for half_edge->opposite_point to be
			// a bounding point, but whatever
			// spit("  should not flip SPECIAL 2");
			return;
		}

		// 3. one of the two points of half_edge is a bounding point:
		//   * hypothetical flipped edge intersects with half_edge -> never legal
		//   * hypothetical flipped edge does not intersect with half_edge -> always legal
		if (half_edge->start_point == i || half_edge->end_point == i) {
			bool lines_intersect = linesIntersect(
					dt_points[half_edge->start_point],
					dt_points[half_edge->end_point],
					dt_points[half_edge->opposite_point],
					dt_points[opposite_edge->opposite_point]);

			// check if the lines intersect between half_edge->start_point and
			// half_edge->end_point
			//   * if they do, the edge must be legalized
			//   * else, the edge must be legal
			if (lines_intersect) {
				definitely_flip = true;
			} else {
				// spit("  should not flip SPECIAL 3");
				return;
			}
		}
	}

	// actual legality check
	Point point_to_test = dt_points[half_edge->opposite_point];
	if (definitely_flip || opposite_edge->parent_triangle->inCircumcircle(point_to_test)) {
		legalizeEdgeImpl(half_edge);

		// edges_to_legalize.emplace(half_edge->parent_triangle, half_edge);
	}
}

// for interactive DT generation
void addPointToDT(Point point) {
	TriangleDT* leaf_triangle = dt_bounding_triangle.findLeafContainingPoint(point);

	if (leaf_triangle) {
		size_t point_index = dt_points.size();
		dt_points.push_back(point);

		leaf_triangle->insertPoint(point_index);
	} else {
		spit("no leaf triangle found");
	}
}

void initDT() {
	dt_points.clear();
	dt_half_edges.clear();
	dt_triangles.clear();

	edges_to_legalize.clear();

	int point_count = 25;
	dt_points = generateDTPoints(point_count);
	dt_bounding_triangle = buildBoundingTriangle();
}

void drawDT() {
	dt_bounding_triangle.draw();

	for (int i = 3; i < dt_points.size(); i++) {
		drawPoint(dt_points[i], device::getColorValue(0.0, 1.0, 1.0));
	}
}


std::array<Point, 3> TriangleDT::points() {
	return std::array<Point, 3>{
		dt_points[half_edge_1->start_point],
		dt_points[half_edge_2->start_point],
		dt_points[half_edge_3->start_point]};
}



// putting these at the bottom because they're huge and tedious

void TriangleDT::insertPoint(size_t point_index) {
	// inserting a point creates three new triangles (assume all points end up
	// inside triangles and not on edges)
	dt_triangles.push_back(TriangleDT{});
	TriangleDT* t1 = &(dt_triangles.back());

	dt_triangles.push_back(TriangleDT{});
	TriangleDT* t2 = &(dt_triangles.back());

	dt_triangles.push_back(TriangleDT{});
	TriangleDT* t3 = &(dt_triangles.back());

	this->child_triangles.push_back(t1);
	this->child_triangles.push_back(t2);
	this->child_triangles.push_back(t3);

	// each triangle has two new half edges
	t1->half_edge_1 = this->half_edge_1;
	dt_half_edges.push_back(HalfEdge{});
	t1->half_edge_2 = &(dt_half_edges.back());
	dt_half_edges.push_back(HalfEdge{});
	t1->half_edge_3 = &(dt_half_edges.back());

	t2->half_edge_1 = this->half_edge_2;
	dt_half_edges.push_back(HalfEdge{});
	t2->half_edge_2 = &(dt_half_edges.back());
	dt_half_edges.push_back(HalfEdge{});
	t2->half_edge_3 = &(dt_half_edges.back());

	t3->half_edge_1 = this->half_edge_3;
	dt_half_edges.push_back(HalfEdge{});
	t3->half_edge_2 = &(dt_half_edges.back());
	dt_half_edges.push_back(HalfEdge{});
	t3->half_edge_3 = &(dt_half_edges.back());

	// set up first triangle
	t1->half_edge_1->parent_triangle = t1;
	t1->half_edge_1->opposite_point = point_index;
	t1->half_edge_1->next_half_edge = t1->half_edge_2;

	t1->half_edge_2->start_point = t1->half_edge_1->end_point;
	t1->half_edge_2->end_point = point_index;
	t1->half_edge_2->parent_triangle = t1;
	t1->half_edge_2->opposite_point = t1->half_edge_1->start_point;
	t1->half_edge_2->next_half_edge = t1->half_edge_3;
	t1->half_edge_2->opposite_half_edge = t2->half_edge_3;

	t1->half_edge_3->start_point = point_index;
	t1->half_edge_3->end_point = t1->half_edge_1->start_point;
	t1->half_edge_3->parent_triangle = t1;
	t1->half_edge_3->opposite_point = t1->half_edge_1->end_point;
	t1->half_edge_3->next_half_edge = t1->half_edge_1;
	t1->half_edge_3->opposite_half_edge = t3->half_edge_2;

	// set up second triangle
	t2->half_edge_1->parent_triangle = t2;
	t2->half_edge_1->opposite_point = point_index;
	t2->half_edge_1->next_half_edge = t2->half_edge_2;

	t2->half_edge_2->start_point = t2->half_edge_1->end_point;
	t2->half_edge_2->end_point = point_index;
	t2->half_edge_2->parent_triangle = t2;
	t2->half_edge_2->opposite_point = t2->half_edge_1->start_point;
	t2->half_edge_2->next_half_edge = t2->half_edge_3;
	t2->half_edge_2->opposite_half_edge = t3->half_edge_3;

	t2->half_edge_3->start_point = point_index;
	t2->half_edge_3->end_point = t2->half_edge_1->start_point;
	t2->half_edge_3->parent_triangle = t2;
	t2->half_edge_3->opposite_point = t2->half_edge_1->end_point;
	t2->half_edge_3->next_half_edge = t2->half_edge_1;
	t2->half_edge_3->opposite_half_edge = t1->half_edge_2;

	// set up third triangle
	t3->half_edge_1->parent_triangle = t3;
	t3->half_edge_1->opposite_point = point_index;
	t3->half_edge_1->next_half_edge = t3->half_edge_2;

	t3->half_edge_2->start_point = t3->half_edge_1->end_point;
	t3->half_edge_2->end_point = point_index;
	t3->half_edge_2->parent_triangle = t3;
	t3->half_edge_2->opposite_point = t3->half_edge_1->start_point;
	t3->half_edge_2->next_half_edge = t3->half_edge_3;
	t3->half_edge_2->opposite_half_edge = t1->half_edge_3;

	t3->half_edge_3->start_point = point_index;
	t3->half_edge_3->end_point = t3->half_edge_1->start_point;
	t3->half_edge_3->parent_triangle = t3;
	t3->half_edge_3->opposite_point = t3->half_edge_1->end_point;
	t3->half_edge_3->next_half_edge = t3->half_edge_1;
	t3->half_edge_3->opposite_half_edge = t2->half_edge_2;

	// call legalize edges on original three edges (in new triangles)
	legalizeEdge(t1->half_edge_1);
	legalizeEdge(t2->half_edge_1);
	legalizeEdge(t3->half_edge_1);

	// spit("");
}

void legalizeEdgeImpl(HalfEdge* half_edge) {
	// edge must be flipped
	HalfEdge* opposite_edge = half_edge->opposite_half_edge;

	dt_half_edges.push_back(HalfEdge{});
	HalfEdge* new_half_edge_1 = &(dt_half_edges.back());

	dt_half_edges.push_back(HalfEdge{});
	HalfEdge* new_half_edge_2 = &(dt_half_edges.back());

	// make new triangles from those half-edges
	dt_triangles.push_back(TriangleDT{});
	TriangleDT* t1 = &(dt_triangles.back());

	dt_triangles.push_back(TriangleDT{});
	TriangleDT* t2 = &(dt_triangles.back());

	half_edge->parent_triangle->child_triangles.push_back(t1);
	half_edge->parent_triangle->child_triangles.push_back(t2);
	opposite_edge->parent_triangle->child_triangles.push_back(t1);
	opposite_edge->parent_triangle->child_triangles.push_back(t2);

	// shorthands so I don't go insane
	HalfEdge* hn = half_edge->next_half_edge;
	HalfEdge* hnn = hn->next_half_edge;
	HalfEdge* on = opposite_edge->next_half_edge;
	HalfEdge* onn = on->next_half_edge;

	t1->half_edge_1 = new_half_edge_1;
	t1->half_edge_2 = onn;
	t1->half_edge_3 = hn;

	t2->half_edge_1 = new_half_edge_2;
	t2->half_edge_2 = hnn;
	t2->half_edge_3 = on;

	// populate new half-edges
	new_half_edge_1->parent_triangle = t1;
	new_half_edge_1->start_point = half_edge->opposite_point;
	new_half_edge_1->end_point = opposite_edge->opposite_point;
	new_half_edge_1->opposite_point = opposite_edge->start_point;
	new_half_edge_1->opposite_half_edge = new_half_edge_2;
	new_half_edge_1->next_half_edge = onn;
	// update the existing half-edges
	onn->next_half_edge = hn;
	onn->opposite_point = new_half_edge_1->start_point;
	onn->parent_triangle = t1;
	hn->next_half_edge = new_half_edge_1;
	hn->opposite_point = new_half_edge_1->end_point;
	hn->parent_triangle = t1;

	new_half_edge_2->parent_triangle = t2;
	new_half_edge_2->start_point = opposite_edge->opposite_point;
	new_half_edge_2->end_point = half_edge->opposite_point;
	new_half_edge_2->opposite_point = half_edge->start_point;
	new_half_edge_2->opposite_half_edge = new_half_edge_1;
	new_half_edge_2->next_half_edge = hnn;
	// update the existing half-edges
	hnn->next_half_edge = on;
	hnn->opposite_point = new_half_edge_2->start_point;
	hnn->parent_triangle = t2;
	on->next_half_edge = new_half_edge_2;
	on->opposite_point = new_half_edge_2->end_point;
	on->parent_triangle = t2;

	// lastly, legalize the next two edges from oppposite_edge
	legalizeEdge(on);
	legalizeEdge(onn);
}

#endif

#ifndef BUFFDOG_COLLISION
#define BUFFDOG_COLLISION

#include <cfloat>


struct Sphere {
  float radius;
	Vector center_pos;
};


struct AABB {
  Vector min_pos;
	Vector max_pos;
};


struct Collision {
  enum class Type {
    aabb,
    sphere
  };

  Type type;

	Sphere sphere;
	AABB box;


  // real time rendering pg. 131
  static float squaredDistanceToAABB(Vector point, AABB box) {
    float squared_distance = 0.0;

    // for each axis, count any excess distance outside AABB extents
    for (int i = 0; i < 3; i++) {
      float point_ex = point.at(i);
      float box_min_ex = box.min_pos.at(i);
      float box_max_ex = box.max_pos.at(i);

      if (point_ex < box_min_ex) {
        squared_distance += (box_min_ex - point_ex) * (box_min_ex - point_ex);
      }

      if (point_ex > box_max_ex) {
        squared_distance += (point_ex - box_max_ex) * (point_ex - box_max_ex);
      }
    }

    return squared_distance;
  }

  static Vector closestPointToAABB(Vector point, AABB box) {
    // clamp point to sides of the box
    for (int i = 0; i < 3; i++) {
      float point_ex = point.at(i);
      float box_min_ex = box.min_pos.at(i);
      float box_max_ex = box.max_pos.at(i);

      if (point_ex < box_min_ex) {
        point.at(i) = box_min_ex;
      }

      if (point_ex > box_max_ex) {
        point.at(i) = box_max_ex;
      }
    }

    return point;
  }

	static bool sphereVsAABB(Sphere sphere, AABB box, Vector& collision_point) {
		// float squared_distance = squaredDistanceToAABB(sphere.center_pos, box);
    // float squared_sphere_radius = sphere.radius * sphere.radius;

    // return squared_distance < squared_sphere_radius;

    collision_point = closestPointToAABB(sphere.center_pos, box);
    Vector center_to_closest = collision_point.subtract(sphere.center_pos);
    float squared_distance_to_center = center_to_closest.dotProduct(center_to_closest);
    float squared_sphere_radius = sphere.radius * sphere.radius;
    return squared_distance_to_center < squared_sphere_radius;
	}

#define EPSILON 0.01

  static bool rayVsAABB(
      Vector point, // ray origin
      Vector direction,
      AABB box,
      float& tmin,
      Vector& collision_point) {
    tmin = 0.0;
    float tmax = FLT_MAX;

    // test against each slab of the AABB
    for (int i = 0; i < 3; i++) {
      float direction_ex = direction.at(i);
      float point_ex = point.at(i);
      float box_min_ex = box.min_pos.at(i);
      float box_max_ex = box.max_pos.at(i);

      if (abs(direction_ex) < EPSILON) {
        // ray is parallel, no hit if origin is not within slab
        if (point_ex < box_min_ex || point_ex > box_max_ex) {
          return false;
        }
      } else {
        float ood = 1.0 / direction_ex;
        float t1 = (box_min_ex - point_ex) * ood;
        float t2 = (box_max_ex - point_ex) * ood;

        if (t1 > t2) {
          // make t1 the near intersection
          float temp = t2;
          t2 = t1;
          t1 = temp;
        }

        tmin = fmax(tmin, t1);
        tmax = fmin(tmax, t2);

        if (tmin > tmax) {
          return false;
        }
      }
    }

    collision_point = point.add(direction.scalarMultiply(tmin));

    return true;
  }
};


#endif

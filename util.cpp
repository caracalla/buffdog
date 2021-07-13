#include <random>

#include "util.h"

std::mt19937 mt;



namespace util {
  void initRandom() {
    std::random_device rd;
    mt = std::mt19937(rd());
  }

	double randomDouble(double lower_bound, double upper_bound) {
		std::uniform_real_distribution<double> dist(lower_bound, upper_bound);

		return dist(mt);
	}

	int randomInt(int lower_bound, int upper_bound) {
		return (int)randomDouble(lower_bound, upper_bound);
	}
}

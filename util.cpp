#include <fstream>
#include <random>
#include <stdexcept>

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

	std::vector<unsigned char> readFile(const char* filename) {
		std::ifstream file(filename, std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file");
		}

		return std::vector<unsigned char>(std::istreambuf_iterator<char>(file), {});
	}
}

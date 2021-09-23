#include <vector>

namespace util {
  void initRandom();

	// [lower_bound, upper_bound)
	double randomDouble(double lower_bound, double upper_bound);
	int randomInt(int lower_bound, int upper_bound);

	std::vector<unsigned char> readFile(const char* filename);
}

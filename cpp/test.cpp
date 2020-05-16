#include <iostream>
#include <vector>
#include <utility>

struct Model {
	std::vector<std::pair<double, double> > uvs;
};

int main() {
	Model item;

	std::pair<double, double> uv = std::make_pair(0.1, 0.1);

	item.uvs = {uv};

	std::cout << "hi" << std::endl;

	return 0;
}

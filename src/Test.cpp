#include <iostream>
#include <vector>

#include "Distribution.hpp"

//======================================

int main()
{
	constexpr int    expected = 1;
	constexpr size_t width    = 100;
	constexpr size_t height   = 50;

	int hist[width] = {};
	for (size_t i = 0; i < 1000; i++)
	{
		int x = ExponentialDistribution(expected);
		if (x < width)
			hist[x]++;
	}

	auto scale = static_cast<double>(height) / *std::max_element(std::begin(hist), std::end(hist));
	for (size_t x = 0; x < width; x++)
	{
		int size = scale*hist[x];

		for (size_t i = 0; i < size; i++)
			std::cout << "\x1B[" << height - size + i + 1 << ";" << x + 1 << "H" << "#";
	}

	std::cout << "\x1B[" << height + 1 << ";1H";
}

//======================================
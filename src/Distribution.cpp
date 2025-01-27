#include <random>
#include "Distribution.hpp"

//======================================

unsigned RandomSeed = std::random_device{}();

//======================================

#ifdef SIMULATION_DISTRIBUTION_USE_STL

int ExponentialDistribution(int expectation)
{
	static std::mt19937 generator(RandomSeed);

	// f(x) = λe^-λx
	// M = integral from -inf to inf of x*λe^-λx dx = 1/λ
	// M = 1/λ => λ = 1/M

	return std::exponential_distribution<>(1.0 / expectation)(generator);
}

#else

#include <cmath>

int ExponentialDistribution(int expectation)
{
	static bool initialized = false;
	if (!initialized)
		srand(RandomSeed), initialized = true;

	// F(x) = 1 - e^-λx => F^-1(y) = -ln(1 - y) / λ
	// M = 1/λ => λ = 1/M => F^-1(y) = -M / ln(1 - y)

	double y = static_cast<double>(rand()) / RAND_MAX;
	return -static_cast<double>(expectation) / log(1.0 - y);
}

#endif // SIMULATION_DISTRIBUTION_USE_STL

//======================================

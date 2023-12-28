#pragma once
#include <corecrt_math.h>

class Easing
{
public:

	float static Linear(const float& n)
	{
		return n;
	}

	float static OutCubic(const float& n)
	{
		return 1 - pow(1 - n, 3);
	}

	float static InOutCubic(const float& n)
	{
		return n < 0.5 ? 4 * n * n * n : 1 - pow(-2 * n + 2, 3) / 2;
	}
};
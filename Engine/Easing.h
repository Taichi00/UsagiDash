#pragma once
#include <corecrt_math.h>

class Easing
{
public:

	float static Linear(float n)
	{
		return n;
	}

	float static InOutCubic(float n)
	{
		return n = n < 0.5 ? 4 * n * n * n : 1 - pow(-2 * n + 2, 3) / 2;
	}
};
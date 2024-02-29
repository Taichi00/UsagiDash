#pragma once
#include <corecrt_math.h>

class Easing
{
public:

	float static Linear(const float n)
	{
		return n;
	}

	float static InCubic(const float n)
	{
		return n * n * n;
	}

	float static OutCubic(const float n)
	{
		return (float)(1 - pow(1 - n, 3));
	}

	float static InOutCubic(const float n)
	{
		return (float)(n < 0.5 ? 4 * n * n * n : 1 - pow(-2 * n + 2, 3) / 2);
	}

	float static InBack(const float n)
	{
		const float c1 = 1.70158f;
		const float c3 = c1 + 1;
		return c3 * n * n * n - c1 * n * n;
	}

	float static OutBack(const float n)
	{
		const float c1 = 1.70158f;
		const float c3 = c1 + 1;
		return (float)(1 + c3 * pow(n - 1, 3) + c1 * pow(n - 1, 2));
	}
};
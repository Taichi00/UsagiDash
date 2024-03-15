#pragma once

class Easing
{
public:
	enum Type
	{
		LINEAR,
		IN_QUAD,
		OUT_QUAD,
		IN_OUT_QUAD,
		IN_CUBIC,
		OUT_CUBIC,
		IN_OUT_CUBIC,
		IN_BACK,
		OUT_BACK,
	};

	static float (*GetFunction(Type type))(const float)
	{
		switch (type)
		{
		case LINEAR: return Linear;
		case IN_QUAD: return InQuad;
		case OUT_QUAD: return OutQuad;
		case IN_OUT_QUAD: return InOutQuad;
		case IN_CUBIC: return InCubic;
		case OUT_CUBIC: return OutCubic;
		case IN_OUT_CUBIC: return InOutCubic;
		case IN_BACK: return InBack;
		case OUT_BACK: return OutBack;
		}

		return Linear;
	}

	static float Linear(const float n)
	{
		return n;
	}

	static float InQuad(const float n)
	{
		return n * n;
	}

	static float OutQuad(const float n)
	{
		return 1.0f - (1.0f - n) * (1.0f - n);
	}

	static float InOutQuad(const float n)
	{
		return n < 0.5f ? 2.0f * n * n : 1.0f - pow(-2.0f * n + 2.0f, 2.0f) / 2.0f;
	}

	static float InCubic(const float n)
	{
		return n * n * n;
	}

	static float OutCubic(const float n)
	{
		return (float)(1 - pow(1 - n, 3));
	}

	static float InOutCubic(const float n)
	{
		return (float)(n < 0.5 ? 4 * n * n * n : 1 - pow(-2 * n + 2, 3) / 2);
	}

	static float InBack(const float n)
	{
		const float c1 = 1.70158f;
		const float c3 = c1 + 1;
		return c3 * n * n * n - c1 * n * n;
	}

	static float OutBack(const float n)
	{
		const float c1 = 1.70158f;
		const float c3 = c1 + 1;
		return (float)(1 + c3 * pow(n - 1, 3) + c1 * pow(n - 1, 2));
	}
};
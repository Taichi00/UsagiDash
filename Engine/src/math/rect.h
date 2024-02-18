#pragma once

struct Rect2
{
	float left;
	float top;
	float right;
	float bottom;

	float Width() const
	{
		return right - left;
	}

	float Height() const
	{
		return bottom - top;
	}

	const Rect2 operator *(const float& n) const
	{
		return Rect2{ left * n, top * n, right * n, bottom * n };
	}
};
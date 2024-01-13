#pragma once

#include <string>

struct Color
{
	Color() : r(0), g(0), b(0) {}
	Color(const float r, const float g, const float b) : r(r), g(g), b(b) {}

	static Color White()	{ return Color(1.0, 1.0, 1.0); }
	static Color Black()	{ return Color(0.0, 0.0, 0.0); }
	static Color Red()		{ return Color(1.0, 0.0, 0.0); }
	static Color Green()	{ return Color(0.0, 1.0, 0.0); }
	static Color Blue()		{ return Color(0.0, 0.0, 1.0); }

	std::string GetString()
	{
		auto sr = std::to_string(r);
		auto sg = std::to_string(g);
		auto sb = std::to_string(b);

		return "Color(" + sr + ", " + sg + ", " + sb + ")";
	}

	float r, g, b;
};
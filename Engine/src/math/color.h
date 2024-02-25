#pragma once

#include <string>

struct Color
{
	float r = 0.0f, g = 0.0f, b = 0.0f;
	float a = 1.0f;

	Color() : r(0), g(0), b(0), a(1) {}
	Color(const float r, const float g, const float b) : r(r), g(g), b(b), a(1) {}
	Color(const float r, const float g, const float b, const float a) : r(r), g(g), b(b), a(a) {}

	static Color White()	{ return Color(1.0, 1.0, 1.0); }
	static Color Black()	{ return Color(0.0, 0.0, 0.0); }
	static Color Red()		{ return Color(1.0, 0.0, 0.0); }
	static Color Green()	{ return Color(0.0, 1.0, 0.0); }
	static Color Blue()		{ return Color(0.0, 0.0, 1.0); }

	bool operator ==(const Color& c) const
	{
		return r == c.r && g == c.g && b == c.b && a == c.a;
	}

	bool operator !=(const Color& c) const
	{
		return !(*this == c);
	}

	size_t GetHashCode() const
	{
		return
			std::hash<decltype(r)>()(this->r) ^
			std::hash<decltype(g)>()(this->g) ^
			std::hash<decltype(b)>()(this->b) ^
			std::hash<decltype(a)>()(this->a);
	}

	struct HashFunctor
	{
		size_t operator ()(const Color& c) const { return c.GetHashCode(); }
	};

	bool IsNearlyEqualTo(const Color& c) const
	{
		return
			fabs(r - c.r) < FLT_EPSILON &&
			fabs(g - c.g) < FLT_EPSILON &&
			fabs(b - c.b) < FLT_EPSILON &&
			fabs(a - c.a) < FLT_EPSILON;
	}

	struct NearEqualityFunctor
	{
		bool operator ()(const Color& a, const Color& b) const { return a.IsNearlyEqualTo(b); }
	};

	bool operator <(const Color& c) const
	{
		if (r < c.r) return true;
		else if (r > c.r) return false;
		if (g < c.g) return true;
		else if (g > c.g) return false;
		if (b < c.b) return true;
		else if (b > c.b) return false;
		if (a < c.a) return true;
		else if (a > c.a) return false;

		return false;
	}

	std::string GetString()
	{
		auto sr = std::to_string(r);
		auto sg = std::to_string(g);
		auto sb = std::to_string(b);
		auto sa = std::to_string(a);

		return "Color(" + sr + ", " + sg + ", " + sb + ", " + sa + ")";
	}
};
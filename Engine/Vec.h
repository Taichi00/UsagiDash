#pragma once

#include <cmath>
#include <string>
#include <DirectXMath.h>

class Vec2
{
public:
	Vec2() {}
	Vec2(float x, float y) : x(x), y(y) {}

	Vec2 operator +(Vec2 v) const
	{
		return Vec2(x + v.x, y + v.y);
	}

	Vec2 operator -(Vec2 v) const
	{
		return Vec2(x - v.x, y - v.y);
	}

	Vec2 operator *(float n) const
	{
		return Vec2(x * n, y * n);
	}

	Vec2 operator /(float n) const
	{
		return Vec2(x / n, y / n);
	}

	operator DirectX::XMFLOAT2() const
	{
		return DirectX::XMFLOAT2(x, y);
	}

	float length() const
	{
		return (float)sqrt(x * x + y * y);
	}

	Vec2 normalized() const
	{
		float len = length();
		if (len == 0)
			return Vec2();

		return Vec2(x / len, y / len);
	}

	static float dot(const Vec2& v1, const Vec2& v2)
	{
		return v1.x * v2.x + v1.y * v2.y;
	}

	std::string getString() const
	{
		std::string sx = std::to_string(x);
		std::string sy = std::to_string(y);

		return std::string("Vec3(" + sx + ", " + sy + ")");
	}

public:
	float x = 0.0f;
	float y = 0.0f;
};


class Vec3
{
public:
	Vec3() {}
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

	Vec3 operator +(Vec3 v) const
	{
		return Vec3(x + v.x, y + v.y, z + v.z);
	}

	Vec3 operator -(Vec3 v) const
	{
		return Vec3(x - v.x, y - v.y, z - v.z);
	}

	Vec3 operator *(float n) const
	{
		return Vec3(x * n, y * n, z * n);
	}

	Vec3 operator /(float n) const
	{
		return Vec3(x / n, y / n, z / n);
	}

	operator DirectX::XMFLOAT3() const
	{
		return DirectX::XMFLOAT3(x, y, z);
	}

	float length() const
	{
		return (float)sqrt(x * x + y * y + z * z);
	}

	Vec3 normalized() const
	{
		float len = length();
		if (len == 0)
			return Vec3();

		return Vec3(x / len, y / len, z / len);
	}

	static float dot(const Vec3& v1, const Vec3& v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	static Vec3 cross(const Vec3& v1, const Vec3& v2)
	{
		Vec3 res;
		res.x = v1.y * v2.z - v1.z * v2.y;
		res.y = v1.z * v2.x - v1.x * v2.z;
		res.z = v1.x * v2.y - v1.y + v2.z;
		return res;
	}

	std::string getString() const
	{
		std::string sx = std::to_string(x);
		std::string sy = std::to_string(y);
		std::string sz = std::to_string(z);

		return std::string("Vec3(" + sx + ", " + sy + ", " + sz + ")");
	}

public:
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};
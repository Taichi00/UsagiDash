#pragma once

#include <cmath>
#include <string>
#include <DirectXMath.h>
#include <utility>

#undef min
#undef max

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

	const Vec2 operator -() const
	{
		return Vec2(-x, -y);
	}

	Vec2& operator +=(const Vec2& v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	Vec2& operator -=(const Vec2& v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	Vec2& operator *=(const float n)
	{
		x *= n;
		y *= n;
		return *this;
	}

	operator DirectX::XMFLOAT2() const
	{
		return DirectX::XMFLOAT2(x, y);
	}

	operator DirectX::XMVECTOR() const
	{
		return { x, y };
	}

	bool operator ==(const Vec2& v) const
	{
		return (x == v.x) && (y == v.y);
	}

	bool operator !=(const Vec2& v) const
	{
		return !(*this == v);
	}

	bool operator >(const Vec2& v) const
	{
		return x > v.x && y > v.y;
	}

	bool operator >=(const Vec2& v) const
	{
		return x >= v.x && y >= v.y;
	}

	bool operator <(const Vec2& v) const
	{
		return x < v.x && y < v.y;
	}

	bool operator <=(const Vec2& v) const
	{
		return x <= v.x && y <= v.y;
	}

	static Vec2 Zero()
	{
		return Vec2(0, 0);
	}

	float Length() const
	{
		return (float)sqrt(x * x + y * y);
	}

	Vec2 Normalized() const
	{
		float len = Length();
		if (len == 0)
			return Vec2();

		return Vec2(x / len, y / len);
	}

	static float Dot(const Vec2& v1, const Vec2& v2)
	{
		return v1.x * v2.x + v1.y * v2.y;
	}

	std::string GetString() const
	{
		std::string sx = std::to_string(x);
		std::string sy = std::to_string(y);

		return std::string("Vec2(" + sx + ", " + sy + ")");
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
	Vec3(DirectX::XMFLOAT3 v) : x(v.x), y(v.y), z(v.z) {}
	Vec3(DirectX::XMVECTOR v) : x(v.m128_f32[0]), y(v.m128_f32[1]), z(v.m128_f32[2]) {}

	const Vec3 operator +(const Vec3& v) const
	{
		return Vec3(x + v.x, y + v.y, z + v.z);
	}

	const Vec3 operator -(const Vec3& v) const
	{
		return Vec3(x - v.x, y - v.y, z - v.z);
	}

	const Vec3 operator *(const Vec3& v) const
	{
		return Vec3(x * v.x, y * v.y, z * v.z);
	}

	const Vec3 operator *(const float& n) const
	{
		return Vec3(x * n, y * n, z * n);
	}

	const Vec3 operator /(const float& n) const
	{
		return Vec3(x / n, y / n, z / n);
	}

	const Vec3 operator -() const
	{
		return Vec3(-x, -y, -z);
	}

	float& operator [](int n)
	{
		return *(&x + n);
	}

	const float& operator [](int n) const
	{
		return *(&x + n);
	}

	Vec3& operator +=(const Vec3& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	Vec3& operator -=(const Vec3& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	Vec3& operator *=(const Vec3& v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		return *this;
	}

	Vec3& operator *=(const float n)
	{
		x *= n;
		y *= n;
		z *= n;
		return *this;
	}

	operator DirectX::XMFLOAT3() const
	{
		return DirectX::XMFLOAT3(x, y, z);
	}

	operator DirectX::XMVECTOR() const
	{
		return { x, y, z };
	}

	bool operator ==(const Vec3& v) const
	{
		return (x == v.x) && (y == v.y) && (z == v.z);
	}

	bool operator !=(const Vec3& v) const
	{
		return !(*this == v);
	}

	bool operator >(const Vec3& v) const
	{
		return x > v.x && y > v.y && z > v.z;
	}

	bool operator >=(const Vec3& v) const
	{
		return x >= v.x && y >= v.y && z >= v.z;
	}

	bool operator <(const Vec3& v) const
	{
		return x < v.x && y < v.y && z < v.z;
	}

	bool operator <=(const Vec3& v) const
	{
		return x <= v.x && y <= v.y && z <= v.z;
	}

	float Length() const
	{
		return (float)sqrt(x * x + y * y + z * z);
	}

	Vec3 Normalized() const
	{
		float len = Length();
		if (len == 0)
			return Vec3();

		return Vec3(x / len, y / len, z / len);
	}

	static float Dot(const Vec3& v1, const Vec3& v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	static Vec3 Cross(const Vec3& v1, const Vec3& v2)
	{
		Vec3 res;
		res.x = v1.y * v2.z - v1.z * v2.y;
		res.y = v1.z * v2.x - v1.x * v2.z;
		res.z = v1.x * v2.y - v1.y * v2.x;
		return res;
	}

	static Vec3 Zero()
	{
		return Vec3(0, 0, 0);
	}

	static Vec3 Scale(const Vec3& v, const float& x, const float& y, const float& z)
	{
		return Vec3(v.x * x, v.y * y, v.z * z);
	}

	static Vec3 Scale(const Vec3& v, const Vec3& scale)
	{
		return Scale(v, scale.x, scale.y, scale.z);
	}

	static float Angle(const Vec3& v1, const Vec3& v2)
	{
		return acos(Dot(v1.Normalized(), v2.Normalized()));
	}

	static Vec3 Lerp(const Vec3& v1, const Vec3& v2, const float& t)
	{
		return v1 * (1 - t) + v2 * t;
	}

	static Vec3 Max(const Vec3& v1, const Vec3& v2)
	{
		float mx = (std::max(v1.x, v2.x));
		float my = (std::max(v1.y, v2.y));
		float mz = (std::max(v1.z, v2.z));
		return Vec3(mx, my, mz);
	}

	static Vec3 Min(const Vec3& v1, const Vec3& v2)
	{
		float mx = (std::min(v1.x, v2.x));
		float my = (std::min(v1.y, v2.y));
		float mz = (std::min(v1.z, v2.z));
		return Vec3(mx, my, mz);
	}

	static Vec3 Abs(const Vec3& v)
	{
		return Vec3(std::abs(v.x), std::abs(v.y), std::abs(v.z));
	}

	std::string ToString() const
	{
		std::string sx = std::to_string(x);
		std::string sy = std::to_string(y);
		std::string sz = std::to_string(z);

		return std::string("Vec3(" + sx + ", " + sy + ", " + sz + ")");
	}

	void Print() const
	{
		printf("%s\n", ToString().c_str());
	}

public:
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
};
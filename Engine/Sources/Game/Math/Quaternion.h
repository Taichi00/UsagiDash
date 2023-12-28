#pragma once

#include <DirectXMath.h>
#include "Vec.h"

# define PI 3.14159265359

class Quaternion
{
public:
	Quaternion() {}
	Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
	Quaternion(DirectX::XMVECTOR v) : x(v.m128_f32[0]), y(v.m128_f32[1]), z(v.m128_f32[2]), w(v.m128_f32[3]) {}

	operator DirectX::XMVECTOR() const
	{
		return { x, y, z, w };
	}

	const Quaternion operator +(const Quaternion& q) const
	{
		return Quaternion(x + q.x, y + q.y, z + q.z, w + q.w);
	}

	const Quaternion operator -(const Quaternion& q) const
	{
		return Quaternion(x - q.x, y - q.y, z - q.z, w - q.w);
	}

	const Quaternion operator *(const float& n) const
	{
		return Quaternion(x * n, y * n, z * n, w * n);
	}

	const Vec3 operator *(const Vec3& v) const
	{
		Quaternion q = *this * Quaternion(v.x, v.y, v.z, 0) * (*this).conjugate();
		return Vec3(q.x, q.y, q.z);
	}

	const Quaternion operator *(const Quaternion& q) const
	{
		return Quaternion(
			q.w * x - q.z * y + q.y * z + q.x * w,
			q.z * x + q.w * y - q.x * z + q.y * w,
			-q.y * x + q.x * y + q.w * z + q.z * w,
			-q.x * x - q.y * y - q.z * z + q.w * w
		);
	}

	static float dot(const Quaternion& q1, const Quaternion& q2)
	{
		return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
	}

	Quaternion conjugate() const
	{
		return Quaternion(-x, -y, -z, w);
	}

	float norm() const
	{
		return (float)sqrt(x * x + y * y + z * z + w * w);
	}

	Quaternion normalized() const
	{
		float len = norm();
		if (len == 0)
			return Quaternion();

		return Quaternion(x / len, y / len, z / len, w / len);
	}

	static Quaternion identity()
	{
		return Quaternion(0.0, 0.0, 0.0, 1.0);
	}

	static Quaternion slerp(const Quaternion& q1, const Quaternion& q2, const float& t)
	{
		float d = dot(q1, q2);
		auto q = q2;
		
		// •½s‚È‚ç
		if (abs(d) >= 1.0)
		{
			return q1;
		}

		// ¬‚³‚¢Šp“x‚ğ‘I‚Ô
		if (d < 0)
		{
			q = q * -1;
			d = -d;
		}
		
		float r = acos(d);
		Quaternion res = q1 * (sin((1.0f - t) * r) / sin(r)) + q * (sin(t * r) / sin(r));
		
		return res;
	}

	static Quaternion FromEuler(const float& x, const float& y, const float& z)
	{
		auto cx = cos(0.5f * x);
		auto sx = sin(0.5f * x);
		auto cy = cos(0.5f * y);
		auto sy = sin(0.5f * y);
		auto cz = cos(0.5f * z);
		auto sz = sin(0.5f * z);

		return Quaternion(
			cx * sy * sz + sx * cy * cz,
			-sx * cy * sz + cx * sy * cz,
			cx * cy * sz + sx * sy * cz,
			-sx * sy * sz + cx * cy * cz
		);
	}

	static Quaternion FromEuler(const Vec3& v)
	{
		return FromEuler(v.x, v.y, v.z);
	}

	Vec3 eulerAngles() const
	{
		Vec3 v;

		v.y = asin(2 * x * z + 2 * y * w);

		if (v.y != 0)
		{
			v.x = atan2(-(2 * y * z - 2 * x * w), (2 * w * w + 2 * z * z - 1));
			v.z = atan2(-(2 * x * y - 2 * z * w), (2 * w * w + 2 * x * x - 1));
		}
		else
		{
			v.x = atan2((2 * y * z + 2 * x * w), (2 * w * w * +2 * y * y - 1));
			v.z = 0;
		}

		return v;
	}

	static Quaternion FromToRotation(const Vec3& from, const Vec3& to)
	{
		auto f = -Vec3::Angle(from, to) / 2;
		auto u = Vec3::cross(from, to).normalized() * sin(f);

		return Quaternion(u.x, u.y, u.z, cos(f));
	}

	operator DirectX::XMFLOAT4() const
	{
		return DirectX::XMFLOAT4(x, y, z, w);
	}

	std::string getString() const
	{
		std::string sx = std::to_string(x);
		std::string sy = std::to_string(y);
		std::string sz = std::to_string(z);
		std::string sw = std::to_string(w);

		return std::string("Quaternion(" + sx + ", " + sy + ", " + sz + ", " + sw + ")");
	}

public:
	float x = 0.0f;
	float y = 0.0f;
	float z = 0.0f;
	float w = 1.0f;
};
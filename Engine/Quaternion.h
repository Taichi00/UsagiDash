#pragma once

#include <DirectXMath.h>

# define PI 3.14159265359

class Quaternion
{
public:
	Quaternion() {}
	Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

	Quaternion operator +(Quaternion q) const
	{
		return Quaternion(x + q.x, y + q.y, z + q.z, w + q.w);
	}

	Quaternion operator -(Quaternion q) const
	{
		return Quaternion(x - q.x, y - q.y, z - q.z, w - q.w);
	}

	Quaternion operator *(float n) const
	{
		return Quaternion(x * n, y * n, z * n, w * n);
	}

	Quaternion operator *(Quaternion q) const
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

	static Quaternion slerp(Quaternion q1, Quaternion q2, float t)
	{
		float d = dot(q1, q2);
		
		// •½s‚È‚ç
		if (d >= 1.0)
		{
			return q1;
		}

		// ¬‚³‚¢Šp“x‚ğ‘I‚Ô
		if (d < 0)
		{
			q2 = q2 * -1;
			d = -d;
		}

		float r = acos(d);
		Quaternion q = q1 * (sin((1.0f - t) * r) / sin(r)) + q2 * (sin(t * r) / sin(r));
		
		return q;
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
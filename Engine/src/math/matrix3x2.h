#pragma once

#include "math/vec.h"

# define PI 3.14159265359

class Matrix3x2
{
public:
	Matrix3x2() : m11(0), m12(0), m21(0), m22(0), m31(0), m32(0) {}
	Matrix3x2(
		float m11, float m12,
		float m21, float m22,
		float m31, float m32
	) :
		m11(m11), m12(m12),
		m21(m21), m22(m22),
		m31(m31), m32(m32) {}

	const Matrix3x2 operator +(const Matrix3x2& m) const
	{
		return Matrix3x2(
			m11 + m.m11, m12 + m.m12,
			m21 + m.m21, m22 + m.m22,
			m31 + m.m31, m32 + m.m32
		);
	}

	const Matrix3x2 operator -(const Matrix3x2& m) const
	{
		return Matrix3x2(
			m11 - m.m11, m12 - m.m12,
			m21 - m.m21, m22 - m.m22,
			m31 - m.m31, m32 - m.m32
		);
	}

	const Matrix3x2 operator *(const Matrix3x2& m) const
	{
		return Matrix3x2(
			m11 * m.m11 + m12 * m.m21        , m11 * m.m12 + m12 * m.m22,
			m21 * m.m11 + m22 * m.m21        , m21 * m.m12 + m22 * m.m22,
			m31 * m.m11 + m32 * m.m21 + m.m31, m31 * m.m12 + m32 * m.m22 + m.m32
		);
	}
	
	static Matrix3x2 Identity()
	{
		return Matrix3x2(
			1, 0,
			0, 1,
			0, 0
		);
	}

	static Matrix3x2 Translation(const Vec2& position)
	{
		return Matrix3x2(
			1, 0,
			0, 1,
			position.x, position.y
		);
	}

	static Matrix3x2 Scale(const Vec2& scale)
	{
		return Matrix3x2(
			scale.x, 0,
			0, scale.y,
			0, 0
		);
	}

	static Matrix3x2 Rotation(const float angle)
	{
		double r = -angle * (PI / 180.0);

		return Matrix3x2(
			cos(r), -sin(r),
			sin(r), cos(r),
			0, 0
		);
	}

	
public:
	float m11, m12;
	float m21, m22;
	float m31, m32;
};
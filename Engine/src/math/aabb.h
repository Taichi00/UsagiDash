#pragma once

#include "math/vec.h"
#include "math/quaternion.h"
#include <DirectXMath.h>

struct AABB
{
	Vec3 max;
	Vec3 min;

	Vec3 Center() const
	{
		return (max + min) / 2;
	}

	Vec3 Size() const
	{
		return max - min;
	}

	void Translate(const Vec3& position)
	{
		max += position;
		min += position;
	}

	void Rotate(const Quaternion& q)
	{
		Vec3 vertices[] = {
			Vec3(min.x, min.y, min.z),
			Vec3(min.x, min.y, max.z),
			Vec3(min.x, max.y, min.z),
			Vec3(max.x, max.y, max.z),
			Vec3(max.x, min.y, min.z),
			Vec3(max.x, min.y, max.z)
		};

		for (auto& vertex : vertices)
		{
			//vertex = XMVector3Transform(vertex, XMMatrixRotationQuaternion(q));
			max = Vec3::Max(vertex, max);
			min = Vec3::Min(vertex, min);
		}
	}

	void Scale(const Vec3& scale)
	{
		max = Vec3::Scale(max, scale);
		min = Vec3::Scale(min, scale);
	}

	bool Intersects(const AABB& b)
	{
		return (min <= b.max) && (max >= b.min);
	}

	void Print()
	{
		printf("AABB\n");
		printf("\tmax(% .2f, % .2f, % .2f)\n", max.x, max.y, max.z);
		printf("\tmin(% .2f, % .2f, % .2f)\n", min.x, min.y, min.z);
	}
};
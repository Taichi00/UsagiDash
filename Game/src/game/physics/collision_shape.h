#pragma once

#include "math/vec.h"
#include "math/aabb.h"

struct CollisionShape
{
	enum Type
	{
		SPHERE,
		CAPSULE,
		POLYGON,
		RAY
	};

	Vec3 position;
	AABB aabb;
	Type type = Type::SPHERE;
};

struct SphereShape : public CollisionShape
{
	float radius = 0;

	SphereShape(float radius) : radius(radius)
	{
		type = SPHERE;
	}
};

struct CapsuleShape : public CollisionShape
{
	float radius = 0;
	float height = 0;

	CapsuleShape(float radius, float height) : radius(radius), height(height)
	{
		type = CAPSULE;
	}
};

struct PolygonShape : public CollisionShape
{
	Vec3 v1, v2, v3;
	Vec3 normal;

	PolygonShape(
		const Vec3& v1, 
		const Vec3& v2, 
		const Vec3& v3, 
		const Vec3& normal
	) : v1(v1), v2(v2), v3(v3), normal(normal)
	{
		type = POLYGON;
	}
};

struct RayShape : public CollisionShape
{
	Vec3 origin;
	Vec3 direction;
	float distance = 0;

	RayShape(
		const Vec3& origin,
		const Vec3& direction,
		const float distance
	) : origin(origin), direction(direction), distance(distance)
	{
		type = RAY;
	}
};


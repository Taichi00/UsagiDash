#include "SphereCollider.h"
#include "Transform.h"
#include "Rigidbody.h"
#include "FloorCollider.h"
#include "MeshCollider.h"
#include "CapsuleCollider.h"
#include "Ray.h"

SphereCollider::SphereCollider(SphereColliderProperty prop)
{
	radius = prop.Radius;
}

SphereCollider::~SphereCollider()
{
}

bool SphereCollider::Init()
{
	Collider::Init();
	return true;
}

bool SphereCollider::Intersects(SphereCollider* sphere)
{
	Vec3 position1 = m_position + offset;
	Vec3 position2 = sphere->GetPosition() + sphere->offset;

	float radius1 = radius;
	float radius2 = sphere->radius;

	Vec3 v = position1 - position2;
	float distance = radius1 + radius2 - v.length();

	if (distance > 0)
	{
		auto normal = v.normalized();

		hitColliders.push_back(sphere);
		hitNormals.push_back(normal);
		hitDistances.push_back(distance);

		sphere->hitColliders.push_back(this);
		sphere->hitNormals.push_back(-normal);
		sphere->hitDistances.push_back(distance);

		return true;
	}

	return false;
}

bool SphereCollider::Intersects(CapsuleCollider* capsule)
{
	return capsule->Intersects(this);
}

bool SphereCollider::Intersects(FloorCollider* floor)
{
	Vec3 position1 = m_position + offset;
	Vec3 position2 = floor->GetPosition() + floor->offset;

	float distance = radius - (position1.y - position2.y);

	if (distance > 0)
	{
		auto normal = Vec3(0, 1, 0);

		hitColliders.push_back(floor);
		hitNormals.push_back(normal);
		hitDistances.push_back(distance);

		floor->hitColliders.push_back(this);
		floor->hitNormals.push_back(-normal);
		floor->hitDistances.push_back(distance);

		return true;
	}

	return false;
}

bool SphereCollider::Intersects(MeshCollider* collider)
{
	return collider->Intersects(this);
}

bool SphereCollider::Intersects(Ray* ray)
{
	Vec3 position = GetPosition() + offset;
	Vec3 rayA = ray->origin;
	Vec3 rayB = rayA + ray->direction * ray->distance;

	auto point = ClosestPointOnLineSegment(rayA, rayB, position);

	auto v = position - point;
	auto distance = radius - v.length();

	if (distance > 0)
	{
		auto normal = v.normalized();

		ray->hitColliders.push_back(this);
		ray->hitNormals.push_back(-normal);
		ray->hitDistances.push_back((position - rayA).length() - radius);

		return true;
	}

	return false;
}

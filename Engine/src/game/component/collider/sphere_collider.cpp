#include "game/component/collider/sphere_collider.h"
#include "game/component/transform.h"
#include "game/component/rigidbody.h"
#include "game/component/collider/floor_collider.h"
#include "game/component/collider/mesh_collider.h"
#include "game/component/collider/capsule_collider.h"
#include "game/component/collider/ray.h"

SphereCollider::SphereCollider(SphereColliderProperty prop)
{
	radius = prop.radius;
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
	Vec3 position1 = position_ + offset;
	Vec3 position2 = sphere->GetPosition() + sphere->offset;

	float radius1 = radius;
	float radius2 = sphere->radius;

	Vec3 v = position1 - position2;
	float distance = radius1 + radius2 - v.Length();

	if (distance > 0)
	{
		auto normal = v.Normalized();

		hit_colliders.push_back(sphere);
		hit_normals.push_back(normal);
		hit_depths.push_back(distance);

		sphere->hit_colliders.push_back(this);
		sphere->hit_normals.push_back(-normal);
		sphere->hit_depths.push_back(distance);

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
	Vec3 position1 = position_ + offset;
	Vec3 position2 = floor->GetPosition() + floor->offset;

	float distance = radius - (position1.y - position2.y);

	if (distance > 0)
	{
		auto normal = Vec3(0, 1, 0);

		hit_colliders.push_back(floor);
		hit_normals.push_back(normal);
		hit_depths.push_back(distance);

		floor->hit_colliders.push_back(this);
		floor->hit_normals.push_back(-normal);
		floor->hit_depths.push_back(distance);

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
	auto distance = radius - v.Length();

	if (distance > 0)
	{
		auto normal = v.Normalized();

		ray->hit_colliders.push_back(this);
		ray->hit_normals.push_back(-normal);
		ray->hit_depths.push_back((position - rayA).Length() - radius);

		return true;
	}

	return false;
}

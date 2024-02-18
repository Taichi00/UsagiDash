#include "game/component/collider/sphere_collider.h"
#include "game/component/transform.h"
#include "game/component/rigidbody.h"
#include "game/component/collider/floor_collider.h"
#include "game/component/collider/polygon_collider.h"
#include "game/component/collider/capsule_collider.h"
#include "game/component/collider/ray.h"
#include "math/aabb.h"
#include <math.h>
#include <cmath>

SphereCollider::SphereCollider(const float radius)
{
	this->radius = radius;
}

SphereCollider::~SphereCollider()
{
	printf("Delete SphereCollider\n");
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

		AddHit({ sphere, normal, distance });
		sphere->AddHit({ this, -normal, distance });

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

		AddHit({ floor, normal, distance });
		floor->AddHit({ this, -normal, distance });
		
		return true;
	}

	return false;
}

bool SphereCollider::Intersects(PolygonCollider* collider)
{
	return collider->Intersects(this);
}

bool SphereCollider::Intersects(Ray* ray)
{
	Vec3 position = GetPosition() + offset;
	Vec3 ray_dir = ray->direction;
	Vec3 rayA = ray->origin;
	Vec3 rayB = rayA + ray_dir * ray->distance;

	auto point = ClosestPointOnLineSegment(rayA, rayB, position);

	auto v = position - point;
	auto distance = radius - v.Length();

	if (distance > 0)
	{
		// ‹…‚ÆRay‚ÌŒð“_‚ð‹‚ß‚é
		auto dis = rayA - position;
		double D = std::pow(Vec3::Dot(ray_dir, dis), 2) - (std::pow(dis.Length(), 2) - radius * radius);
		auto hit_point = rayA + ray_dir * (-Vec3::Dot(ray_dir, dis) - std::sqrt(D));

		auto normal = -ray_dir.Normalized();

		ray->AddHit({ this, normal, (hit_point - rayB).Length() });

		return true;
	}

	return false;
}

void SphereCollider::PrepareAABB()
{
	aabb_ = AABB{};
	aabb_.max = Vec3(1, 1, 1) * radius;
	aabb_.min = -Vec3(1, 1, 1) * radius;
}

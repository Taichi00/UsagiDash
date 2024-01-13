#include "game/component/collider/capsule_collider.h"
#include "game/component/collider/mesh_collider.h"
#include "game/component/collider/sphere_collider.h"

CapsuleCollider::CapsuleCollider(CapsuleColliderProperty prop)
{
	radius = prop.radius;
	height = prop.height;
}

CapsuleCollider::~CapsuleCollider()
{
}

bool CapsuleCollider::Init()
{
	Collider::Init();
	return true;
}

bool CapsuleCollider::Intersects(SphereCollider* sphere)
{
	Vec3 position1 = position_ + offset;
	Vec3 position2 = sphere->GetPosition() + sphere->offset;
	float radius1 = radius;
	float radius2 = sphere->radius;
	
	// TODO: ‰ñ“]‚Ö‚Ì‘Î‰ž
	Vec3 A = position1 - Vec3(0, 1, 0) * (height / 2);
	Vec3 B = position1 + Vec3(0, 1, 0) * (height / 2);

	// ‹…‚Ì’†S‚ÆÅ‚à‹ß‚¢“_‚ðŒ©‚Â‚¯‚é
	Vec3 referencePosition = ClosestPointOnLineSegment(A, B, position2);

	Vec3 v = referencePosition - position2;
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

bool CapsuleCollider::Intersects(CapsuleCollider* capsule)
{
	Vec3 position1 = position_ + offset;
	Vec3 position2 = capsule->GetPosition() + capsule->offset;
	float radius1 = radius;
	float radius2 = capsule->radius;
	float height1 = height;
	float height2 = capsule->height;

	// TODO: ‰ñ“]‚Ö‚Ì‘Î‰ž
	Vec3 A1 = position1 - Vec3(0, 1, 0) * (height1 / 2);
	Vec3 B1 = position1 + Vec3(0, 1, 0) * (height1 / 2);
	Vec3 A2 = position2 - Vec3(0, 1, 0) * (height2 / 2);
	Vec3 B2 = position2 + Vec3(0, 1, 0) * (height2 / 2);

	Vec3 v0 = A2 - A1;
	Vec3 v1 = B2 - A1;
	Vec3 v2 = A2 - B1;
	Vec3 v3 = B2 - B1;

	float d0 = Vec3::Dot(v0, v0);
	float d1 = Vec3::Dot(v1, v1);
	float d2 = Vec3::Dot(v2, v2);
	float d3 = Vec3::Dot(v3, v3);

	Vec3 best1;
	if (d2 < d0 || d2 < d1 || d3 < d0 || d3 < d1)
	{
		best1 = B1;
	}
	else
	{
		best1 = A1;
	}

	Vec3 best2 = ClosestPointOnLineSegment(A2, B2, best1);
	best1 = ClosestPointOnLineSegment(A1, B1, best2);

	Vec3 v = best1 - best2;
	float distance = radius1 + radius2 - v.Length();

	if (distance > 0)
	{
		auto normal = v.Normalized();

		hit_colliders.push_back(capsule);
		hit_normals.push_back(normal);
		hit_depths.push_back(distance);

		capsule->hit_colliders.push_back(this);
		capsule->hit_normals.push_back(-normal);
		capsule->hit_depths.push_back(distance);

		return true;
	}

	return false;
}

bool CapsuleCollider::Intersects(FloorCollider* floor)
{
	return false;
}

bool CapsuleCollider::Intersects(MeshCollider* collider)
{
	return collider->Intersects(this);
}

#include "CapsuleCollider.h"
#include "MeshCollider.h"
#include "SphereCollider.h"

CapsuleCollider::CapsuleCollider(CapsuleColliderProperty prop)
{
	radius = prop.Radius;
	height = prop.Height;
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
	Vec3 position1 = m_position + offset;
	Vec3 position2 = sphere->GetPosition() + sphere->offset;
	float radius1 = radius;
	float radius2 = sphere->radius;
	
	// TODO: ‰ñ“]‚Ö‚Ì‘Î‰ž
	Vec3 A = position1 - Vec3(0, 1, 0) * (height / 2);
	Vec3 B = position1 + Vec3(0, 1, 0) * (height / 2);

	// ‹…‚Ì’†S‚ÆÅ‚à‹ß‚¢“_‚ðŒ©‚Â‚¯‚é
	Vec3 referencePosition = ClosestPointOnLineSegment(A, B, position2);

	Vec3 v = referencePosition - position2;
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

bool CapsuleCollider::Intersects(CapsuleCollider* capsule)
{
	Vec3 position1 = m_position + offset;
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

	float d0 = Vec3::dot(v0, v0);
	float d1 = Vec3::dot(v1, v1);
	float d2 = Vec3::dot(v2, v2);
	float d3 = Vec3::dot(v3, v3);

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
	float distance = radius1 + radius2 - v.length();

	if (distance > 0)
	{
		auto normal = v.normalized();

		hitColliders.push_back(capsule);
		hitNormals.push_back(normal);
		hitDistances.push_back(distance);

		capsule->hitColliders.push_back(this);
		capsule->hitNormals.push_back(-normal);
		capsule->hitDistances.push_back(distance);

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

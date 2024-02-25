#pragma once

#include "game/component/collider/collider.h"
#include "game/resource/collision_model.h"
#include "math/aabb.h"

class Rigidbody;
class MeshCollider;

class PolygonCollider : public Collider
{
public:
	PolygonCollider(const CollisionPolygon& polygon, MeshCollider* mesh_collider);
	~PolygonCollider() override;

	bool Init() override;

	bool Intersects(SphereCollider* sphere) override;
	bool Intersects(CapsuleCollider* capsule) override;
	bool Intersects(Ray* ray) override;

private:
	bool SphereIntersectsPolygon(
		const Vec3& center, const float radius,
		Vec3& normal, float& distance);

	void PrepareAABB() override;

private:
	Vec3 vertices_[3];
	Vec3 normal_;

	AABB default_aabb_;

	MeshCollider* mesh_collider_ = nullptr;
};
#pragma once

#include "game/component/collider/collider.h"
#include "engine/shared_struct.h"
#include <set>
#include <vector>

class SphereCollider;
class CapsuleCollider;
class Ray;

struct MeshColliderProperty
{
	CollisionModel model;
};

class MeshCollider : public Collider
{
public:
	MeshCollider(MeshColliderProperty prop);
	~MeshCollider();

	bool Init();

	bool Intersects(SphereCollider* sphere);
	bool Intersects(CapsuleCollider* capsule);
	bool Intersects(Ray* ray);

private:
	bool SphereIntersectsMesh(
		const Vec3& center, const float radius,
		const CollisionMesh& mesh, const CollisionFace& face,
		Vec3& normal, float& distance, std::vector<Vec3>& collidedPoints);

public:
	CollisionModel model;
};
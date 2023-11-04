#pragma once

#include "Collider.h"
#include "SharedStruct.h"

class SphereCollider;
class CapsuleCollider;

struct MeshColliderProperty
{
	CollisionModel Model;
};

class MeshCollider : public Collider
{
public:
	MeshCollider(MeshColliderProperty prop);
	~MeshCollider();

	bool Init();

	bool Intersects(SphereCollider* sphere);
	bool Intersects(CapsuleCollider* capsule);

private:
	bool SphereIntersectsMesh(
		const Vec3& center, const float radius,
		const CollisionMesh& mesh, const uint32_t* indices,
		Vec3& normal, float& distance);

public:
	CollisionModel model;
};
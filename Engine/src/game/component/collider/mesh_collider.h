#pragma once

#include "game/component/collider/collider.h"
#include "game/resource/collision_model.h"
#include "engine/shared_struct.h"
#include <set>
#include <vector>
#include <memory>

class SphereCollider;
class CapsuleCollider;
class PolygonCollider;
class Ray;

class MeshCollider : public Collider
{
public:
	MeshCollider(std::shared_ptr<CollisionModel> model);
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

	void PrepareAABB() override;

public:
	std::shared_ptr<CollisionModel> model;

	std::vector<std::shared_ptr<PolygonCollider>> polygons;
};
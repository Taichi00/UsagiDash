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
	~MeshCollider() override;

	bool Init();

private:
	void PrepareAABB() override;

public:
	std::shared_ptr<CollisionModel> model;

private:
	std::vector<std::shared_ptr<PolygonCollider>> polygons_;
};
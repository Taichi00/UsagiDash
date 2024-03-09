#include "game/entity.h"
#include "game/scene.h"
#include "game/collision_manager.h"
#include "game/component/collider/mesh_collider.h"
#include "game/component/collider/sphere_collider.h"
#include "game/component/collider/capsule_collider.h"
#include "game/component/collider/ray.h"
#include "game/component/collider/polygon_collider.h"
#include "game/component/rigidbody.h"
#include "math/vec.h"
#include <algorithm>

MeshCollider::MeshCollider(std::shared_ptr<CollisionModel> model)
{
	this->model = model;
}

MeshCollider::~MeshCollider()
{
	
}

bool MeshCollider::Init()
{
	rigidbody_ = GetEntity()->GetComponent<Rigidbody>();

	collision_manager_ = Game::Get()->GetCollisionManager();

	// PolygonCollider‚ð¶¬
	for (auto& mesh : model->meshes)
	{
		for (auto& polygon : mesh.polygons)
		{
			auto pc = std::make_shared<PolygonCollider>(polygon, this);
			pc->RegisterEntity(GetEntity());
			pc->Init();
			polygons_.push_back(pc);
		}
	}

	return true;
}

void MeshCollider::PrepareAABB()
{
	aabb_ = model->aabb;
}


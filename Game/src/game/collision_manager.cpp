#include "game/collision_manager.h"
#include "game/component/collider/collider.h"
#include "game/component/collider/sphere_collider.h"
#include "game/component/transform.h"
#include "game/component/rigidbody.h"
#include "game/component/collider/ray.h"
#include "game/entity.h"
#include "math/aabb.h"
#include "game/game.h"
#include "game/layer_manager.h"
#include <unordered_map>

CollisionManager::CollisionManager()
{
	octree_ = std::make_unique<Octree<Collider>>();
	octree_->Init(6, { Vec3(1000, 1000, 1000), Vec3(-1000, -1000, -1000) });

	layer_manager_ = Game::Get()->GetLayerManager();
}

void CollisionManager::Update(const float delta_time)
{
	// 衝突検知済みのコライダーのペア
	std::unordered_map<Collider*, std::unordered_map<Collider*, bool>> collided_map;

	PrepareRigidbodies(delta_time);
	PrepareColliders();

	for (int i = 0; i < 3; i++)
	{
		// 準備
		Prepare();

		// 衝突判定
		std::vector<Collider*> collision_list;
		octree_->GetAllCollisionList(collision_list);

		for (int i = 0; i < collision_list.size(); i += 2)
		{
			auto collider1 = collision_list[i];
			auto collider2 = collision_list[i + 1];

			// 衝突判定
			if (collider1->Intersects(collider2))
			{
				collided_map[collider1][collider2] = true;
				collided_map[collider2][collider1] = true;
			}
		}

		// 衝突応答
		for (auto rigidbody : active_rigidbodies_)
		{
			rigidbody->Resolve(delta_time);
		}
	}

	for (auto rigidbody : active_rigidbodies_)
	{
		// 位置の更新
		rigidbody->transform->position = rigidbody->position;
		rigidbody->velocity = (rigidbody->position - rigidbody->position_prev) / delta_time;
	}

	// イベント通知
	for (auto& pair1 : collided_map)
	{
		auto entity = pair1.first->GetEntity();

		for (auto& pair2 : collided_map)
		{
			entity->OnCollisionEnter(pair2.first);
		}
	}
}

void CollisionManager::Add(Collider* collider)
{
	colliders_.push_back(collider);
}

void CollisionManager::Add(Rigidbody* rigidbody)
{
	rigidbodies_.push_back(rigidbody);
}

void CollisionManager::Remove(Collider* collider)
{
	colliders_.erase(
		std::remove_if(colliders_.begin(), colliders_.end(), [&collider](Collider* c) {
			return c == collider;
			}),
		colliders_.end()
	);
}

void CollisionManager::Remove(Rigidbody* rigidbody)
{
	rigidbodies_.erase(
		std::remove_if(rigidbodies_.begin(), rigidbodies_.end(), [&rigidbody](Rigidbody* r) {
			return r == rigidbody;
			}),
		rigidbodies_.end()
	);
}

void CollisionManager::Detect(Ray* ray, const std::vector<std::string>& mask_tags)
{
	// 準備
	//for (auto collider : colliders_)
	//{
	//	// コンポーネントが無効なら判定しない
	//	if (!collider->enabled)
	//		continue;

	//	collider->Prepare();
	//}

	// 衝突判定
	for (auto collider : colliders_)
	{
		// mask_tagsが空ならすべてのコライダーと衝突する
		if (!mask_tags.empty())
		{
			// mask_tagsに含まれていなければ無視
			if (std::find(mask_tags.begin(), mask_tags.end(), collider->GetEntity()->tag) == mask_tags.end())
			{
				continue;
			}
		}

		// AABBが衝突していなければ無視
		if (!collider->GetAABB().Intersects(ray->GetAABB()))
		{
			continue;
		}

		collider->Intersects(ray);
	}
}

void CollisionManager::SetOctreeSize(const AABB& aabb)
{
	octree_->Init(6, aabb);
}

void CollisionManager::PrepareRigidbodies(const float delta_time)
{
	active_rigidbodies_.clear();

	for (auto rigidbody : rigidbodies_)
	{
		if (!rigidbody->enabled)
			continue;

		// 更新停止中なら判定しない
		if (!rigidbody->GetEntity()->IsUpdateEnabled())
			continue;

		rigidbody->Prepare(delta_time);

		active_rigidbodies_.push_back(rigidbody);
	}
}

void CollisionManager::PrepareColliders()
{
	active_colliders_.clear();

	for (auto collider : colliders_)
	{
		// コンポーネントが無効なら判定しない
		if (!collider->enabled)
			continue;

		// 更新停止中なら判定しない
		if (!collider->GetEntity()->IsUpdateEnabled())
			continue;

		collider->ResetHits();
		active_colliders_.push_back(collider);
	}
}

void CollisionManager::Prepare()
{
	octree_objects_.clear();

	for (auto collider : active_colliders_)
	{
		collider->Prepare();

		// octree object を生成
		auto entity = collider->GetEntity();
		auto object = std::make_unique<OctreeObject<Collider>>();

		object->object = collider;
		object->group = entity;
		object->layer = layer_manager_->GetLayerIndex(entity->layer);
		object->aabb = collider->GetAABB();

		// Octreeに追加
		octree_->Regist(object.get());
		octree_objects_.push_back(std::move(object));
	}
}

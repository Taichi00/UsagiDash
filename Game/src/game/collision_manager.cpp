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

	std::vector<Collider*> colliders;
	std::vector<Rigidbody*> rigidbodies;
	std::vector<std::unique_ptr<OctreeObject<Collider>>> octree_objects;

	// rigidbody の準備
	for (auto rigidbody : rigidbodies_)
	{
		// 更新停止中なら判定しない
		if (!rigidbody->GetEntity()->IsUpdateEnabled())
			continue;

		// 重力
		if (rigidbody->use_gravity)
		{
			rigidbody->velocity += Vec3(0.f, -64.8f, 0.f) * delta_time;
		}

		// 空気抵抗
		rigidbody->velocity *= 0.995f;

		rigidbody->Prepare(delta_time);

		rigidbodies.push_back(rigidbody);
	}

	// collider の準備
	for (auto collider : colliders_)
	{
		// コンポーネントが無効なら判定しない
		if (!collider->enabled)
			continue;

		// 更新停止中なら判定しない
		if (!collider->GetEntity()->IsUpdateEnabled())
			continue;
		
		collider->ResetHits();
		collider->Prepare();

		auto entity = collider->GetEntity();
		auto obj = std::make_unique<OctreeObject<Collider>>();

		obj->object = collider;
		obj->group = entity;
		obj->layer = layer_manager_->GetLayerIndex(entity->layer);
		obj->aabb = collider->GetAABB();

		// Octreeに追加
		octree_->Regist(obj.get());

		colliders.push_back(collider);
		octree_objects.push_back(std::move(obj));
	}

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
	for (auto rigidbody : rigidbodies)
	{
		rigidbody->Resolve(delta_time);
	}

	for (int i = 0; i < 2; i++)
	{
		// 準備
		for (auto& octree_object : octree_objects)
		{
			auto collider = octree_object->object;

			collider->Prepare();

			octree_object->Remove();
			octree_object->aabb = collider->GetAABB();
			octree_->Regist(octree_object.get());
		}

		// 衝突判定
		collision_list.clear();
		octree_->GetAllCollisionList(collision_list);

		for (int j = 0; j < collision_list.size(); j += 2)
		{
			auto collider1 = collision_list[j];
			auto collider2 = collision_list[j + 1];

			// 衝突判定
			if (collider1->Intersects(collider2))
			{
				collided_map[collider1][collider2] = true;
				collided_map[collider2][collider1] = true;
			}
		}

		// 衝突応答
		for (auto rigidbody : rigidbodies)
		{
			rigidbody->Resolve(delta_time);
		}
	}

	for (auto rigidbody : rigidbodies)
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

	//auto entity = collider->GetEntity();
	//auto obj = std::make_unique<OctreeObject<Collider>>();

	//obj->object = collider;
	//obj->group = entity;
	//obj->layer = layer_manager_->GetLayerIndex(entity->layer);

	//// Octreeに追加
	//octree_objects_.push_back(std::move(obj));
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

	//auto it = std::find_if(octree_objects_.begin(), octree_objects_.end(), [&collider](std::unique_ptr<OctreeObject<Collider>>& obj) {
	//	return obj->object == collider;
	//	});
	//
	//if (it != octree_objects_.end())
	//{
	//	(*it)->Remove();
	//	octree_objects_.erase(it);
	//}
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
	for (auto& collider : colliders_)
	{
		if (collider == nullptr) continue;

		collider->Prepare();
	}

	// 衝突判定
	for (auto i = 0; i < colliders_.size(); i++)
	{
		auto collider = colliders_[i];

		if (collider == nullptr) continue;

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

void CollisionManager::SetNonCollisionTags(const std::string& tag1, const std::string& tag2)
{
	non_collision_tags_.push_back(std::make_pair(tag1, tag2));
}

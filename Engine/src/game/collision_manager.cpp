#include "game/collision_manager.h"
#include "game/component/collider/collider.h"
#include "game/component/collider/sphere_collider.h"
#include "game/component/transform.h"
#include "game/component/rigidbody.h"
#include "game/component/collider/ray.h"
#include "game/entity.h"

void CollisionManager::Update()
{
	colliders_.erase(
		std::remove_if(colliders_.begin(), colliders_.end(), [](Collider* c) {
			return c == nullptr;
			}),
		colliders_.end()
	);

	collided_entities_.clear();

	// colliderの準備
	for (auto& collider : colliders_)
	{
		collider->ResetHits();
	}

	// rigidbodyの準備
	for (auto& rigidbody : rigidbodies_)
	{
		// 重力
		if (rigidbody->use_gravity)
		{
			rigidbody->velocity += Vec3(0, -0.018, 0);
		}

		// 空気抵抗
		rigidbody->velocity *= 0.995;

		rigidbody->Prepare();
	}

	// 準備
	for (auto& collider : colliders_)
	{
		collider->Prepare();
	}

	// 衝突判定
	for (auto i = 0; i < colliders_.size(); i++)
	{
		auto collider1 = colliders_[i];

		for (auto j = i + 1; j < colliders_.size(); j++)
		{
			auto collider2 = colliders_[j];

			// 衝突しないタグの組であれば無視
			/*auto& tag1 = collider1->GetEntity()->tag;
			auto& tag2 = collider2->GetEntity()->tag;
			if (std::find(non_collision_tags_.begin(), non_collision_tags_.end(), std::make_pair(tag1, tag2)) != non_collision_tags_.end())
			{
				continue;
			}*/

			// AABBが衝突していなければ無視
			if (!collider1->GetAABB().Intersects(collider2->GetAABB()))
			{
				continue;
			}

			// 衝突判定
			if (collider1->Intersects(collider2))
			{
				collided_entities_.insert({ collider1->GetEntity(), collider2 });
				collided_entities_.insert({ collider2->GetEntity(), collider1 });
			}
		}
	}

	// 衝突応答
	for (auto& rigidbody : rigidbodies_)
	{
		rigidbody->Resolve();
	}

	for (int i = 0; i < 2; i++)
	{
		// 準備
		for (auto& collider : colliders_)
		{
			collider->Prepare();
		}

		// 衝突判定
		for (auto i = 0; i < colliders_.size(); i++)
		{
			auto collider1 = colliders_[i];

			if (!collider1->GetRigidbody()) continue;

			for (auto j = i + 1; j < colliders_.size(); j++)
			{
				auto collider2 = colliders_[j];

				if (!collider2->GetRigidbody()) continue;

				// 衝突しないタグの組であれば無視
				/*auto& tag1 = collider1->GetEntity()->tag;
				auto& tag2 = collider2->GetEntity()->tag;
				if (std::find(non_collision_tags_.begin(), non_collision_tags_.end(), std::make_pair(tag1, tag2)) != non_collision_tags_.end())
				{
					continue;
				}*/

				// AABBが衝突していなければ無視
				if (!collider1->GetAABB().Intersects(collider2->GetAABB()))
				{
					continue;
				}
				
				// 衝突判定
				if (collider1->Intersects(collider2))
				{
					collided_entities_.insert({ collider1->GetEntity(), collider2 });
					collided_entities_.insert({ collider2->GetEntity(), collider1 });
				}
			}
		}
		
		// 衝突応答
		for (auto& rigidbody : rigidbodies_)
		{
			rigidbody->Resolve();
		}
	}

	for (auto& rigidbody : rigidbodies_)
	{
		// 位置の更新
		if (rigidbody->floor_rigidbody)
		{
			rigidbody->floor_velocity = rigidbody->floor_rigidbody->velocity;
		}
		rigidbody->floor_velocity *= 0.98;

		//rigidbody->transform->position += rigidbody->floor_velocity;

		rigidbody->transform->position = rigidbody->position;
		rigidbody->velocity = rigidbody->position - rigidbody->position_prev;

		//rigidbody->transform->position += rigidbody->floor_velocity;
	}

	/*for (auto& rigidbody : rigidbodies_)
	{
		if (rigidbody->floor_rigidbody)
		{
			rigidbody->floor_velocity = rigidbody->floor_rigidbody->velocity;
		}
		rigidbody->floor_velocity *= 0.98;

		rigidbody->transform->position += rigidbody->floor_velocity;
	}*/

	for (auto& p : collided_entities_)
	{
		p.first->OnCollisionEnter(p.second);
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

//void CollisionManager::Remove()
//{
//	// nullptrであれば削除
//	for (auto it = colliders_.begin(); it != colliders_.end(); ++it)
//	{
//		if (*it == nullptr) colliders_.erase(it);
//	}
//
//	for (auto it = rigidbodies_.begin(); it != rigidbodies_.end(); ++it)
//	{
//		if (*it == nullptr) rigidbodies_.erase(it);
//	}
//
//}

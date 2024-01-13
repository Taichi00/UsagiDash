#include "game/collision_manager.h"
#include "game/component/collider/collider.h"
#include "game/component/collider/sphere_collider.h"
#include "game/component/transform.h"
#include "game/component/rigidbody.h"

void CollisionManager::Update()
{
	for (auto rigidbody : rigidbodies_)
	{
		rigidbody->is_grounded = false;
		//rigidbody->floorVelocity = Vec3::Zero();
		rigidbody->floor_rigidbody = nullptr;
	}

	// €”õ
	for (int i = 0; i < 3; i++)
	{
		for (auto collider : colliders_)
		{
			collider->Prepare();
		}

		// Õ“Ë”»’è
		for (auto i = 0; i < colliders_.size(); i++)
		{
			auto collider = colliders_[i];
			for (auto j = i + 1; j < colliders_.size(); j++)
			{
				auto ret = collider->Intersects(colliders_[j]);
			}
		}

		// Õ“Ë‰“š
		for (auto rigidbody : rigidbodies_)
		{
			rigidbody->Resolve();
		}
	}

	for (auto rigidbody : rigidbodies_)
	{
		// ˆÊ’u‚ÌXV
		//rigidbody->velocity += rigidbody->floorVelocity * rigidbody->friction;
		
		/*if (rigidbody->floorRigidbody)
		{
			rigidbody->transform->position += rigidbody->floorRigidbody->velocity;
		}*/
		if (rigidbody->floor_rigidbody)
		{
			rigidbody->floor_velocity = rigidbody->floor_velocity + rigidbody->floor_rigidbody->floor_velocity;
		}
		rigidbody->transform->position += rigidbody->floor_velocity;
		rigidbody->floor_velocity *= 0.98;

		rigidbody->transform->position += rigidbody->velocity;

		if (rigidbody->use_gravity)
		{
			rigidbody->velocity += Vec3(0, -0.018, 0);
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

void CollisionManager::Detect(Ray* ray)
{
	// €”õ
	for (auto collider : colliders_)
	{
		collider->Prepare();
	}

	// Õ“Ë”»’è
	for (auto i = 0; i < colliders_.size(); i++)
	{
		colliders_[i]->Intersects(ray);
	}
}

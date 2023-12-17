#include "CollisionManager.h"
#include "Collider.h"
#include "SphereCollider.h"
#include "Transform.h"
#include "Rigidbody.h"

void CollisionManager::Update()
{
	for (auto rigidbody : m_pRigidbodies)
	{
		rigidbody->isGrounded = false;
	}

	// €”õ
	for (int i = 0; i < 3; i++)
	{
		for (auto collider : m_pColliders)
		{
			collider->Prepare();
		}

		// Õ“Ë”»’è
		for (auto i = 0; i < m_pColliders.size(); i++)
		{
			auto collider = m_pColliders[i];
			for (auto j = i + 1; j < m_pColliders.size(); j++)
			{
				auto ret = collider->Intersects(m_pColliders[j]);
			}
		}

		// Õ“Ë‰“š
		for (auto rigidbody : m_pRigidbodies)
		{
			rigidbody->Resolve();
		}
	}

	for (auto rigidbody : m_pRigidbodies)
	{
		// ˆÊ’u‚ÌXV
		rigidbody->transform->position += rigidbody->velocity;

		if (rigidbody->useGravity)
		{
			rigidbody->velocity += Vec3(0, -0.018, 0);
		}
	}
}

void CollisionManager::Add(Collider* collider)
{
	m_pColliders.push_back(collider);
}

void CollisionManager::Add(Rigidbody* rigidbody)
{
	m_pRigidbodies.push_back(rigidbody);
}

void CollisionManager::Detect(Ray* ray)
{
	// €”õ
	for (auto collider : m_pColliders)
	{
		collider->Prepare();
	}

	// Õ“Ë”»’è
	for (auto i = 0; i < m_pColliders.size(); i++)
	{
		m_pColliders[i]->Intersects(ray);
	}
}

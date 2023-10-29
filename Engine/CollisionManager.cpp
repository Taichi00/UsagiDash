#include "CollisionManager.h"
#include "Collider.h"
#include "SphereCollider.h"
#include "Transform.h"
#include "Rigidbody.h"

void CollisionManager::Update()
{
	// €”õ
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

void CollisionManager::Add(Collider* collider)
{
	m_pColliders.push_back(collider);
}

void CollisionManager::Add(Rigidbody* rigidbody)
{
	m_pRigidbodies.push_back(rigidbody);
}

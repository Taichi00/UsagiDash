#pragma once

#include <vector>

class Collider;
class Rigidbody;

class CollisionManager
{
public:
	void Update();

	void Add(Collider* collider);
	void Add(Rigidbody* rigidbody);

private:
	std::vector<Collider*> m_pColliders;
	std::vector<Rigidbody*> m_pRigidbodies;
};
#pragma once

#include <vector>

class Collider;
class Rigidbody;
class Ray;

class CollisionManager
{
public:
	void Update();

	void Add(Collider* collider);
	void Add(Rigidbody* rigidbody);

	void Detect(Ray* ray);

private:
	std::vector<Collider*> m_pColliders;
	std::vector<Rigidbody*> m_pRigidbodies;
};
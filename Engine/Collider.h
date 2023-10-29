#pragma once

#include "Component.h"
#include "Vec.h"

class Rigidbody;
class SphereCollider;
class FloorCollider;
class MeshCollider;

class Collider : public Component
{
public:
	bool Init();

	void Prepare();
	Vec3 GetPosition();

	Rigidbody* GetRigidbody();

	bool Intersects(Collider* collider);
	virtual bool Intersects(SphereCollider* sphere);
	virtual bool Intersects(FloorCollider* floor);
	virtual bool Intersects(MeshCollider* collider);

public:
	Vec3 offset;
	
	std::vector<Collider*> hitColliders;
	std::vector<Vec3> hitNormals;
	std::vector<float> hitDistances;
	
protected:
	Rigidbody* m_pRigidbody;
	Vec3 m_position;
	Vec3 m_scale;
};
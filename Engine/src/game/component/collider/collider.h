#pragma once

#include "game/component/component.h"
#include "math/vec.h"

class Rigidbody;
class SphereCollider;
class CapsuleCollider;
class FloorCollider;
class MeshCollider;
class Ray;

class Collider : public Component
{
public:
	bool Init();

	void Prepare();
	Vec3 GetPosition();
	Vec3 GetExtension();

	Rigidbody* GetRigidbody();

	bool Intersects(Collider* collider);
	virtual bool Intersects(SphereCollider* sphere);
	virtual bool Intersects(CapsuleCollider* sphere);
	virtual bool Intersects(FloorCollider* floor);
	virtual bool Intersects(MeshCollider* collider);
	virtual bool Intersects(Ray* ray);

	static Vec3 ClosestPointOnLineSegment(const Vec3& a, const Vec3& b, const Vec3& point);

public:
	Vec3 offset;
	
	std::vector<Collider*> hit_colliders;
	std::vector<Vec3> hit_normals;
	std::vector<float> hit_depths;
	
protected:
	Rigidbody* rigidbody_;
	Vec3 position_;
	Vec3 scale_;
	Vec3 extension_;	// 地面吸着用の延長ベクトル
};
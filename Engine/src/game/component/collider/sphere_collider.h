#pragma once

#include "game/component/collider/collider.h"

class FloorCollider;
class PolygonCollider;
class CapsuleCollider;
class Ray;

class SphereCollider : public Collider
{
public:
	SphereCollider(const float radius);
	~SphereCollider();

	bool Init();

	bool Intersects(SphereCollider* sphere);
	bool Intersects(CapsuleCollider* capsule);
	bool Intersects(FloorCollider* floor);
	bool Intersects(PolygonCollider* collider);
	bool Intersects(Ray* ray);

private:
	void PrepareAABB() override;

public:
	float radius;
};
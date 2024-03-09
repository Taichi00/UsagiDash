#pragma once

#include "game/component/component.h"
#include "math/vec.h"
#include "math/aabb.h"

class Rigidbody;
class SphereCollider;
class CapsuleCollider;
class FloorCollider;
class PolygonCollider;
class Ray;
class CollisionManager;

class Collider : public Component
{
public:
	enum Type
	{
		NONE,
		SPHERE,
		CAPSULE,
		FLOOR,
		POLYGON,
		RAY,
	};

	// 衝突の情報
	struct HitInfo
	{
		Collider* collider = nullptr;
		Vec3 normal;
		float depth = 0.f;
	};

	Collider() {}
	virtual ~Collider();

	bool Init();

	void ResetHits();
	void Prepare();

	bool Intersects(Collider* collider);
	virtual bool Intersects(SphereCollider* sphere) { return false; }
	virtual bool Intersects(CapsuleCollider* sphere) { return false; }
	virtual bool Intersects(FloorCollider* floor) { return false; }
	virtual bool Intersects(PolygonCollider* collider) { return false; }
	virtual bool Intersects(Ray* ray) { return false; }

	// 線分と点の最近傍点を計算
	static Vec3 ClosestPointOnLineSegment(const Vec3& a, const Vec3& b, const Vec3& point);

	void AddHit(const HitInfo& hit);

	Vec3 GetPosition() const { return position_; }
	AABB GetAABB() const { return aabb_; }
	Rigidbody* GetRigidbody() const { return rigidbody_; }
	const std::vector<HitInfo>& GetHits() const { return hits_; }
	const HitInfo& GetNearestHit() const { return nearest_hit_; }

private:
	virtual void PrepareAABB() = 0;

public:
	Vec3 offset;
	Vec3 scale = Vec3(1, 1, 1);
	
protected:
	Rigidbody* rigidbody_ = nullptr;
	Vec3 position_;
	AABB aabb_;

	// コライダーの種類
	Type type_ = NONE;

	std::vector<HitInfo> hits_;
	HitInfo nearest_hit_; // 最も近い衝突

	CollisionManager* collision_manager_ = nullptr;
};
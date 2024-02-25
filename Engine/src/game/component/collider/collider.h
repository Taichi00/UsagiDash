#pragma once

#include "game/component/component.h"
#include "math/vec.h"
#include "math/aabb.h"
#include <memory>
#include <string>

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
	struct HitInfo
	{
		Collider* collider = nullptr;
		Vec3 normal;
		float depth = 0.f;
	};

	Collider() = default;
	virtual ~Collider();

	bool Init();

	void ResetHits();
	void Prepare();

	Vec3 GetPosition();
	Vec3 GetExtension();
	AABB GetAABB();

	bool HasDetected();
	void SetHasDetected(bool flag);

	Rigidbody* GetRigidbody();

	bool Intersects(Collider* collider);
	virtual bool Intersects(SphereCollider* sphere);
	virtual bool Intersects(CapsuleCollider* sphere);
	virtual bool Intersects(FloorCollider* floor);
	virtual bool Intersects(PolygonCollider* collider);
	virtual bool Intersects(Ray* ray);

	static Vec3 ClosestPointOnLineSegment(const Vec3& a, const Vec3& b, const Vec3& point);

	void AddHit(const HitInfo& hit);
	const std::vector<HitInfo>& GetHits();

	const HitInfo& GetNearestHit();

private:
	virtual void PrepareAABB() = 0;

public:
	Vec3 offset;
	Vec3 scale = Vec3(1, 1, 1);
	
protected:
	Rigidbody* rigidbody_;
	Vec3 position_;
	Vec3 extension_;	// �n�ʋz���p�̉����x�N�g��
	AABB aabb_;

	std::vector<HitInfo> hits_;
	HitInfo nearest_hit_; // �ł��߂��Փ�

	CollisionManager* collision_manager_;

	bool has_detected_; // ���肪�I���������ǂ���
};
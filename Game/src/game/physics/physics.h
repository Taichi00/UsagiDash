#pragma once

#define NOMINMAX

#include "game/physics/collision_shape.h"
#include "math/vec.h"
#include <string>
#include <vector>

class Collider;

class Physics
{
public:
	struct Hit
	{
		// 衝突点の法線ベクトル
		Vec3 normal;
		// 衝突深度
		float depth = 0;
	};

	struct RaycastHit
	{
		// 衝突したコライダー
		Collider* collider = nullptr;
		// 衝突点
		Vec3 point;
		// 衝突店の法線ベクトル
		Vec3 normal;
		// 衝突点への距離
		float distance = 0;
	};

	static bool Raycast(
		const Vec3& origin,
		const Vec3& direction,
		const float max_distance,
		RaycastHit& hit,
		const std::vector<std::string>& mask_layers = {}
	);

	static bool DetectCollision(
		CollisionShape* shape1,
		CollisionShape* shape2,
		Hit& hit
	);

private:
	// 球と球の衝突検知
	static bool DetectSphereSphereCollision(
		SphereShape* sphere1,
		SphereShape* sphere2,
		Hit& hit
	);
	// 球とレイの衝突検知
	static bool DetectSphereRayCollision(
		SphereShape* sphere,
		RayShape* ray,
		Hit& hit
	);
	// カプセルとカプセルの衝突検知
	static bool DetectCapsuleCapsuleCollision(
		CapsuleShape* capsule1,
		CapsuleShape* capsule2,
		Hit& hit
	);
	// カプセルと球の衝突検知
	static bool DetectCapsuleSphereCollision(
		CapsuleShape* capsule,
		SphereShape* sphere,
		Hit& hit
	);
	// カプセルとレイの衝突検知
	static bool DetectCapsuleRayCollision(
		CapsuleShape* capsule,
		RayShape* ray,
		Hit& hit
	);
	// ポリゴンと球の衝突検知
	static bool DetectPolygonSphereCollision(
		PolygonShape* polygon,
		SphereShape* sphere,
		Hit& hit
	);
	// ポリゴンとカプセルの衝突検知
	static bool DetectPolygonCapsuleCollision(
		PolygonShape* polygon,
		CapsuleShape* capsule,
		Hit& hit
	);
	// ポリゴンとレイの衝突検知
	static bool DetectPolygonRayCollision(
		PolygonShape* polygon,
		RayShape* ray,
		Hit& hit
	);
	
	// 点に最も近い線分上の座標を取得する
	static Vec3 ClosestPointOnLineSegment(const Vec3& a, const Vec3& b, const Vec3& point);

};
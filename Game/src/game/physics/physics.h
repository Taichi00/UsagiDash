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
		// �Փ˓_�̖@���x�N�g��
		Vec3 normal;
		// �Փː[�x
		float depth = 0;
	};

	struct RaycastHit
	{
		// �Փ˂����R���C�_�[
		Collider* collider = nullptr;
		// �Փ˓_
		Vec3 point;
		// �Փ˓X�̖@���x�N�g��
		Vec3 normal;
		// �Փ˓_�ւ̋���
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
	// ���Ƌ��̏Փˌ��m
	static bool DetectSphereSphereCollision(
		SphereShape* sphere1,
		SphereShape* sphere2,
		Hit& hit
	);
	// ���ƃ��C�̏Փˌ��m
	static bool DetectSphereRayCollision(
		SphereShape* sphere,
		RayShape* ray,
		Hit& hit
	);
	// �J�v�Z���ƃJ�v�Z���̏Փˌ��m
	static bool DetectCapsuleCapsuleCollision(
		CapsuleShape* capsule1,
		CapsuleShape* capsule2,
		Hit& hit
	);
	// �J�v�Z���Ƌ��̏Փˌ��m
	static bool DetectCapsuleSphereCollision(
		CapsuleShape* capsule,
		SphereShape* sphere,
		Hit& hit
	);
	// �J�v�Z���ƃ��C�̏Փˌ��m
	static bool DetectCapsuleRayCollision(
		CapsuleShape* capsule,
		RayShape* ray,
		Hit& hit
	);
	// �|���S���Ƌ��̏Փˌ��m
	static bool DetectPolygonSphereCollision(
		PolygonShape* polygon,
		SphereShape* sphere,
		Hit& hit
	);
	// �|���S���ƃJ�v�Z���̏Փˌ��m
	static bool DetectPolygonCapsuleCollision(
		PolygonShape* polygon,
		CapsuleShape* capsule,
		Hit& hit
	);
	// �|���S���ƃ��C�̏Փˌ��m
	static bool DetectPolygonRayCollision(
		PolygonShape* polygon,
		RayShape* ray,
		Hit& hit
	);
	
	// �_�ɍł��߂�������̍��W���擾����
	static Vec3 ClosestPointOnLineSegment(const Vec3& a, const Vec3& b, const Vec3& point);

};
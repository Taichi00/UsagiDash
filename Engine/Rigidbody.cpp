#include "Rigidbody.h"
#include "Entity.h"
#include "Collider.h"
#include "Entity.h"
#include "Scene.h"
#include "CollisionManager.h"
#include <math.h>

Rigidbody::Rigidbody(RigidbodyProperty prop)
{
	collider = prop.Collider;
	mass = prop.Mass;
	useGravity = prop.UseGravity;
	isStatic = prop.IsStatic;
	friction = prop.Friction;
}

Rigidbody::~Rigidbody()
{
}

bool Rigidbody::Init()
{
	m_pEntity->GetScene()->GetCollisionManager()->Add(this);
	
	return true;
}

void Rigidbody::Resolve()
{
	if (isStatic)
		return;

	
	Vec3 gnormal = Vec3(0, 1, 0);	// 重力の方向

	float maxDistance = 0;
	Collider* hitCollider = nullptr;
	int idx = 0;
	for (auto i = 0; i < collider->hitColliders.size(); i++)
	{
		auto hitRigidbody = collider->hitColliders[i]->GetRigidbody();

		if (hitRigidbody == nullptr)
			continue;

		auto distance = collider->hitDistances[i];
		if (distance > maxDistance)
		{
			maxDistance = distance;
			hitCollider = collider->hitColliders[i];
			idx = i;
		}
	}
	
	//for (auto i = 0; i < collider->hitColliders.size(); i++)
	if (hitCollider != nullptr)
	{
		//auto hitCollider = collider->hitColliders[i];
		auto hitRigidbody = hitCollider->GetRigidbody();

		//if (hitRigidbody == nullptr)
		//	continue;

		auto normal = collider->hitNormals[idx];
		auto distance = collider->hitDistances[idx];

		float e = -0.4;	// 反発係数
		float k;	// ばね定数

		auto hitMass = hitRigidbody->mass;
		auto hitVelocity = hitRigidbody->velocity;
		auto hitFriction = hitRigidbody->friction;
		Vec3 tangent;

		float J, B;

		// 接触面から離れる方向に動いている場合は無視
		if (Vec3::dot(velocity - hitVelocity, normal) <= 0.2)
		{
			// 地面なら
			if (Vec3::Angle(normal, Vec3(0, 1, 0)) < 0.9)
			{
				isGrounded |= true;
			}

			if (hitRigidbody->isStatic)
			{
				// 不動オブジェクトと衝突した場合
				tangent = -(velocity - normal * Vec3::dot(normal, velocity)).normalized();

				J = -Vec3::dot(velocity, normal) * (1.0 + e) / (1.0 / mass);	// 撃力
				B = std::min(friction, hitFriction) * J;	// 摩擦力
				k = 0.3;
			}
			else
			{
				// 不動でないオブジェクトと衝突した場合
				auto v = velocity - hitVelocity;
				tangent = -(v - normal * Vec3::dot(normal, v)).normalized();

				J = -Vec3::dot(v, normal) * (1.0 + e) / (1.0 / mass + 1.0 / hitMass);	// 撃力
				B = std::min(friction, hitFriction) * J;	// 摩擦力
				k = 0.3;
			}
			
			velocity += normal * J / mass;
			velocity += tangent * B / mass;
			velocity += normal * distance * k;
		}
	}	

	//float g = -0.016;	// 重力
	//
	//if (useGravity)
	//{
	//	velocity += gnormal.normalized() * g;
	//}

	velocity *= 0.995;

	prevVelocity = velocity;
}

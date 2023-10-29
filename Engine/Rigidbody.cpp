#include "Rigidbody.h"
#include "Entity.h"
#include "Collider.h"
#include "Entity.h"
#include "Scene.h"
#include "CollisionManager.h"

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

	for (auto i = 0; i < collider->hitColliders.size(); i++)
	{
		auto hitCollider = collider->hitColliders[i];
		auto hitRigidbody = hitCollider->GetRigidbody();

		if (hitRigidbody == nullptr)
			continue;
		
		auto normal = collider->hitNormals[i];
		auto distance = collider->hitDistances[i];

		float e = -0.4;	// 反発係数
		float k;	// ばね定数

		auto hitMass = hitRigidbody->mass;
		auto hitVelocity = hitRigidbody->velocity;
		auto hitFriction = hitRigidbody->friction;
		Vec3 tangent;

		float J, B;

		// 接触面から離れる方向に動いている場合
		if (Vec3::dot(velocity - hitVelocity, normal) > 0)
			continue;

		if (hitRigidbody->isStatic)
		{
			// 不動オブジェクトと衝突した場合
			tangent = -(velocity - normal * Vec3::dot(normal, velocity)).normalized();

			J = -Vec3::dot(velocity, normal) * (1.0 + e) / (1.0 / mass);	// 撃力
			//B = -Vec3::dot(velocity, tangent) / (1.0 / mass);		// 摩擦力
			B = std::min(friction, hitFriction) * J;

			k = 0.3;
			/*if (J > 0)
			{
				float lim = abs(B / J);
				if (mu < lim)
				{
					B = mu * J;
				}
			}*/
			//B = 0;
		}
		else
		{
			// 不動でないオブジェクトと衝突した場合
			auto v = velocity - hitVelocity;
			tangent = -(v - normal * Vec3::dot(normal, v)).normalized();

			J = -Vec3::dot(v, normal) * (1.0 + e) / (1.0 / mass + 1.0 / hitMass);	// 撃力
			//B = -Vec3::dot(v, tangent) / (1.0 / mass + 1.0 / hitMass);		// 摩擦力
			B = std::min(friction, hitFriction) * J;

			k = 0.3;
		}
		
		velocity += normal * J / mass;
		velocity += tangent * B / mass;
		velocity += normal * distance * k;
	}

	// 位置の更新
	transform->position += velocity;

	float g = -0.013;	// 重力
	
	if (useGravity)
		velocity.y += g;

	velocity *= 0.995;
}

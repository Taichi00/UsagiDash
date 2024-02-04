#include "game/component/rigidbody.h"
#include "game/entity.h"
#include "game/component/collider/collider.h"
#include "game/entity.h"
#include "game/scene.h"
#include "game/collision_manager.h"
#include <math.h>
#include <game/game.h>

Rigidbody::Rigidbody(RigidbodyProperty prop)
{
	collider = prop.collider;
	mass = prop.mass;
	use_gravity = prop.use_gravity;
	is_static = prop.is_static;
	friction = prop.friction;
}

Rigidbody::~Rigidbody()
{
	collision_manager_->Remove(this);
}

bool Rigidbody::Init()
{
	collision_manager_ = Game::Get()->GetCollisionManager();
	collision_manager_->Add(this);
	
	return true;
}

void Rigidbody::Prepare()
{
	position = transform->position + velocity;
	position_prev = transform->position;
	velocity_prev = velocity;

	is_grounded = false;
	floor_rigidbody = nullptr;
	floor_normal = Vec3(0, 1, 0);
	floor_velocity = Vec3::Zero();
}

void Rigidbody::Resolve()
{
	velocity_prev = velocity;

	if (is_static)
		return;

	Vec3 gnormal = Vec3(0, 1, 0);	// 重力の方向

	auto& hit = collider->GetNearestHit();
	/*int idx = 0;
	for (auto i = 0; i < collider->hit_colliders.size(); i++)
	{
		auto hitRigidbody = collider->hit_colliders[i]->GetRigidbody();

		if (hitRigidbody == nullptr)
			continue;

		auto distance = collider->hit_depths[i];
		if (distance > maxDistance)
		{
			maxDistance = distance;
			hitCollider = collider->hit_colliders[i];
			idx = i;
		}
	}*/
	
	//for (auto i = 0; i < collider->hitColliders.size(); i++)
	if (hit.collider != nullptr)
	{
		//auto hitCollider = collider->hitColliders[i];
		auto hitRigidbody = hit.collider->GetRigidbody();

		//if (hitRigidbody == nullptr)
		//	continue;

		auto normal = hit.normal;
		auto depth = hit.depth;

		float e = -0.4;	// 反発係数
		float k;	// ばね定数

		auto hitMass = hitRigidbody->mass;
		auto hitVelocity = hitRigidbody->velocity;
		auto hitFriction = hitRigidbody->friction;

		float J, B;
		Vec3 dp;
		auto v = velocity - hitVelocity;

		Vec3 tangent = Vec3::Cross(normal, Vec3::Cross(v, normal)).Normalized();
		tangent = Vec3::Scale(tangent, 1, 0, 1);

		// 接触面から離れる方向に動いている場合は無視
		if (Vec3::Dot(v, normal) <= 0.2)
		{
			// 地面なら
			if (Vec3::Angle(normal, Vec3(0, 1, 0)) < 0.9)
			{
				is_grounded |= true;
				floor_rigidbody = hitRigidbody;
				floor_normal = normal;
				floor_velocity = hitRigidbody->velocity;
			}

			if (hitRigidbody->is_static)
			{
				// 不動オブジェクトと衝突した場合
				//tangent = -(velocity - normal * Vec3::Dot(normal, velocity)).Normalized();

				J = -Vec3::Dot(v, normal) * (1.0 + e) / (1.0 / mass);	// 撃力
				B = -Vec3::Dot(v, tangent) / (1.0 / mass);

				if (hitFriction < std::abs(B / J)) B = std::min(friction, hitFriction) * B;

				dp = normal * depth;

				k = 0.3;
			}
			else
			{
				// 不動でないオブジェクトと衝突した場合
				//tangent = -(v - normal * Vec3::Dot(normal, v)).Normalized();

				J = -Vec3::Dot(v, normal) * (1.0 + e) / (1.0 / mass + 1.0 / hitMass);	// 撃力
				//B = std::min(friction, hitFriction) * J;	// 摩擦力

				B = -Vec3::Dot(v, tangent) / (1.0 / mass + 1.0 / hitMass);

				if (hitFriction < std::abs(B / J)) B = std::min(friction, hitFriction) * B;

				dp = normal * depth * (1.0 / mass) / (1.0 / mass + 1.0 / hitMass);

				k = 0.3;
			}
			
			/*velocity += normal * J / mass;
			velocity += tangent * B / mass;
			velocity += normal * depth * k;*/

			position += dp + tangent * B / mass;
			//velocity = Vec3::Zero();
		}
	}

}

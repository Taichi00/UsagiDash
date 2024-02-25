#include "game/component/rigidbody.h"
#include "game/entity.h"
#include "game/component/collider/collider.h"
#include "game/entity.h"
#include "game/scene.h"
#include "game/collision_manager.h"
#include <math.h>
#include <game/game.h>

Rigidbody::Rigidbody(float mass, bool use_gravity, bool is_static, float friction)
{
	this->mass = mass;
	this->use_gravity = use_gravity;
	this->is_static = is_static;
	this->friction = friction;
}

Rigidbody::~Rigidbody()
{
	collision_manager_->Remove(this);
}

bool Rigidbody::Init()
{
	collider = GetEntity()->GetComponent<Collider>();

	collision_manager_ = Game::Get()->GetCollisionManager();
	collision_manager_->Add(this);
	
	return true;
}

void Rigidbody::Prepare(const float delta_time)
{
	position = transform->position + velocity;
	position_prev = transform->position;
	velocity_prev = velocity;

	is_grounded = false;
	floor_rigidbody = nullptr;
	floor_normal = Vec3::Zero();
	floor_velocity = Vec3::Zero();

	is_touching_wall = false;
	wall_normal = Vec3::Zero();
}

void Rigidbody::Resolve()
{
	velocity_prev = velocity;

	if (is_static)
		return;

	Vec3 gnormal = Vec3(0, 1, 0);	// 重力の方向

	auto& hit = collider->GetNearestHit();
	
	if (hit.collider != nullptr)
	{
		auto hitRigidbody = hit.collider->GetRigidbody();

		auto normal = hit.normal;
		auto depth = hit.depth;

		float e = -0.4f;	// 反発係数
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
			auto angle = Vec3::Angle(normal, Vec3(0, 1, 0));

			// 地面なら
			if (angle < 0.9 && angle >= 0.0)
			{
				is_grounded |= true;
				floor_rigidbody = hitRigidbody;
				floor_normal = normal;
				floor_velocity = hitRigidbody->velocity;
			}
			else if (angle >= 0.9f && angle < 2.7f)
			{
				is_touching_wall |= true;
				wall_normal = normal;
			}
			
			if (hitRigidbody->is_static)
			{
				// 不動オブジェクトと衝突した場合
				J = -Vec3::Dot(v, normal) * (1.0f + e) / (1.0f / mass);	// 撃力
				B = -Vec3::Dot(v, tangent) / (1.0f / mass);

				if (hitFriction < std::abs(B / J)) B = std::min(friction, hitFriction) * B;

				dp = normal * depth;

				k = 0.3f;
			}
			else
			{
				// 不動でないオブジェクトと衝突した場合
				J = -Vec3::Dot(v, normal) * (1.0f + e) / (1.0f / mass + 1.0f / hitMass);	// 撃力
				B = -Vec3::Dot(v, tangent) / (1.0f / mass + 1.0f / hitMass);

				if (hitFriction < std::abs(B / J)) B = std::min(friction, hitFriction) * B;

				dp = normal * depth * (1.0f / mass) / (1.0f / mass + 1.0f / hitMass);

				k = 0.3f;
			}
			
			/*velocity += normal * J / mass;
			velocity += tangent * B / mass;
			velocity += normal * depth * k;*/

			position += dp + tangent * B / mass;
		}
	}

}

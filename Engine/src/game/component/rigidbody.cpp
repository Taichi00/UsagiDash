#include "game/component/rigidbody.h"
#include "game/entity.h"
#include "game/component/collider/collider.h"
#include "game/entity.h"
#include "game/scene.h"
#include "game/collision_manager.h"
#include <math.h>

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
}

bool Rigidbody::Init()
{
	GetEntity()->GetScene()->GetCollisionManager()->Add(this);
	
	return true;
}

void Rigidbody::Resolve()
{
	if (is_static)
		return;

	
	Vec3 gnormal = Vec3(0, 1, 0);	// �d�͂̕���

	float maxDistance = 0;
	Collider* hitCollider = nullptr;
	int idx = 0;
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
	}
	
	//for (auto i = 0; i < collider->hitColliders.size(); i++)
	if (hitCollider != nullptr)
	{
		//auto hitCollider = collider->hitColliders[i];
		auto hitRigidbody = hitCollider->GetRigidbody();

		//if (hitRigidbody == nullptr)
		//	continue;

		auto normal = collider->hit_normals[idx];
		auto distance = collider->hit_depths[idx];

		float e = -0.4;	// �����W��
		float k;	// �΂˒萔

		auto hitMass = hitRigidbody->mass;
		auto hitVelocity = hitRigidbody->velocity;
		auto hitFriction = hitRigidbody->friction;
		Vec3 tangent;

		float J, B;

		// �ڐG�ʂ��痣�������ɓ����Ă���ꍇ�͖���
		if (Vec3::Dot(velocity - hitVelocity, normal) <= 0.2)
		{
			// �n�ʂȂ�
			if (Vec3::Angle(normal, Vec3(0, 1, 0)) < 0.9)
			{
				is_grounded |= true;
				floor_rigidbody = hitRigidbody;
				floor_normal = normal;
				floor_velocity = hitRigidbody->velocity;
			}

			if (hitRigidbody->is_static)
			{
				// �s���I�u�W�F�N�g�ƏՓ˂����ꍇ
				tangent = -(velocity - normal * Vec3::Dot(normal, velocity)).Normalized();

				J = -Vec3::Dot(velocity, normal) * (1.0 + e) / (1.0 / mass);	// ����
				B = std::min(friction, hitFriction) * J;	// ���C��
				k = 0.3;
			}
			else
			{
				// �s���łȂ��I�u�W�F�N�g�ƏՓ˂����ꍇ
				auto v = velocity - hitVelocity;
				tangent = -(v - normal * Vec3::Dot(normal, v)).Normalized();

				J = -Vec3::Dot(v, normal) * (1.0 + e) / (1.0 / mass + 1.0 / hitMass);	// ����
				B = std::min(friction, hitFriction) * J;	// ���C��
				k = 0.3;
			}
			
			velocity += normal * J / mass;
			velocity += tangent * B / mass;
			velocity += normal * distance * k;
		}
	}	

	//float g = -0.016;	// �d��
	//
	//if (useGravity)
	//{
	//	velocity += gnormal.normalized() * g;
	//}

	velocity *= 0.995;

	velocity_prev = velocity;
}

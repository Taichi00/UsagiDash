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

	
	Vec3 gnormal = Vec3(0, 1, 0);	// �d�͂̕���

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

		float e = -0.4;	// �����W��
		float k;	// �΂˒萔

		auto hitMass = hitRigidbody->mass;
		auto hitVelocity = hitRigidbody->velocity;
		auto hitFriction = hitRigidbody->friction;
		Vec3 tangent;

		float J, B;

		// �ڐG�ʂ��痣�������ɓ����Ă���ꍇ�͖���
		if (Vec3::dot(velocity - hitVelocity, normal) <= 0.2)
		{
			// �n�ʂȂ�
			if (Vec3::Angle(normal, Vec3(0, 1, 0)) < 0.9)
			{
				isGrounded |= true;
			}

			if (hitRigidbody->isStatic)
			{
				// �s���I�u�W�F�N�g�ƏՓ˂����ꍇ
				tangent = -(velocity - normal * Vec3::dot(normal, velocity)).normalized();

				J = -Vec3::dot(velocity, normal) * (1.0 + e) / (1.0 / mass);	// ����
				B = std::min(friction, hitFriction) * J;	// ���C��
				k = 0.3;
			}
			else
			{
				// �s���łȂ��I�u�W�F�N�g�ƏՓ˂����ꍇ
				auto v = velocity - hitVelocity;
				tangent = -(v - normal * Vec3::dot(normal, v)).normalized();

				J = -Vec3::dot(v, normal) * (1.0 + e) / (1.0 / mass + 1.0 / hitMass);	// ����
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

	prevVelocity = velocity;
}

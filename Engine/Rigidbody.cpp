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

		float e = -0.4;	// �����W��
		float k;	// �΂˒萔

		auto hitMass = hitRigidbody->mass;
		auto hitVelocity = hitRigidbody->velocity;
		auto hitFriction = hitRigidbody->friction;
		Vec3 tangent;

		float J, B;

		// �ڐG�ʂ��痣�������ɓ����Ă���ꍇ
		if (Vec3::dot(velocity - hitVelocity, normal) > 0)
			continue;

		if (hitRigidbody->isStatic)
		{
			// �s���I�u�W�F�N�g�ƏՓ˂����ꍇ
			tangent = -(velocity - normal * Vec3::dot(normal, velocity)).normalized();

			J = -Vec3::dot(velocity, normal) * (1.0 + e) / (1.0 / mass);	// ����
			//B = -Vec3::dot(velocity, tangent) / (1.0 / mass);		// ���C��
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
			// �s���łȂ��I�u�W�F�N�g�ƏՓ˂����ꍇ
			auto v = velocity - hitVelocity;
			tangent = -(v - normal * Vec3::dot(normal, v)).normalized();

			J = -Vec3::dot(v, normal) * (1.0 + e) / (1.0 / mass + 1.0 / hitMass);	// ����
			//B = -Vec3::dot(v, tangent) / (1.0 / mass + 1.0 / hitMass);		// ���C��
			B = std::min(friction, hitFriction) * J;

			k = 0.3;
		}
		
		velocity += normal * J / mass;
		velocity += tangent * B / mass;
		velocity += normal * distance * k;
	}

	// �ʒu�̍X�V
	transform->position += velocity;

	float g = -0.013;	// �d��
	
	if (useGravity)
		velocity.y += g;

	velocity *= 0.995;
}

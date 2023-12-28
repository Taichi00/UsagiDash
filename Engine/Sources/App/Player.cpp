#include "Player.h"
#include "Input.h"
#include "Entity.h"
#include "Animator.h"
#include "Camera.h"
#include "Scene.h"
#include "Rigidbody.h"
#include "Physics.h"
#include "ParticleEmitter.h"

Player::Player(PlayerProperty prop)
{
	m_speed = prop.Speed;
	m_acceleration = prop.Acceleration;
	
}

Player::~Player()
{
}

bool Player::Init()
{
	m_pAnimator = GetEntity()->GetComponent<Animator>();
	m_pRigidbody = GetEntity()->GetComponent<Rigidbody>();
	return true;
}

void Player::Update()
{
	Move();
	Animate();

	_m_isRunning = m_isRunning;
	_m_isGrounded = m_isGrounded;
	_m_jumpFrame = m_jumpFrame;

}

void Player::Move()
{
	auto camera = GetEntity()->GetScene()->GetMainCamera();
	auto cameraRot = camera->transform->rotation;
	auto forward = Vec3::Scale(cameraRot * Vec3(0, 0, -1), 1, 0, 1).normalized();
	auto left = Vec3::Scale(cameraRot * Vec3(-1, 0, 0), 1, 0, 1).normalized();

	/*m_isGrounded = false;
	for (int i = 0; i < 5; i++)
	{
		m_isGrounded |= m_groundBuffer[i];
	}*/
	m_isGrounded = m_pRigidbody->isGrounded;

	float friction = 0.9;
	float acceleration = m_acceleration;

	if (!m_isGrounded)
	{
		friction = 0.99;
		acceleration *= 0.4;
	}

	auto velocity = &(m_pRigidbody->velocity);
	
	Vec3 v = Vec3::Zero();

	if (Input::GetKey(DIK_UP))
	{
		v += forward;
	}
	else if (Input::GetKey(DIK_DOWN))
	{
		v += -forward;
	}

	if (Input::GetKey(DIK_LEFT))
	{
		v += left;
	}
	else if (Input::GetKey(DIK_RIGHT))
	{
		v += -left;
	}

	v = v.normalized();

	auto pv = v * m_acceleration;
	

	// ��
	if (m_isGrounded)
	{
		//auto origin = transform->position + Vec3(0, 1, 0) + *velocity + pv;
		//auto direction = Vec3(0, -1, 0);
		//auto distance = 5;
		//RaycastHit hit;
		//if (Physics::Raycast(origin, direction, distance, hit))
		//{
		//	auto runSmokeEmitter = GetEntity()->GetChild("Run Smoke Emitter")->GetComponent<ParticleEmitter>();

		//	auto q = Quaternion::FromToRotation(Vec3(0, 1, 0), hit.normal);
		//	if ((q * pv).y < 0)
		//	{
		//		runSmokeEmitter->transform->rotation = q.conjugate();
		//		pv = q * pv;
		//	}
		//	else
		//	{
		//		runSmokeEmitter->transform->rotation = q;
		//	}

		//	//*velocity += Vec3(0, 0.018, 0) - hit.normal * 0.018;
		//}

		auto runSmokeEmitter = GetEntity()->GetChild("Run Smoke Emitter")->GetComponent<ParticleEmitter>();

		auto q = Quaternion::FromToRotation(Vec3(0, 1, 0), m_pRigidbody->floorNormal);
		if ((q * pv).y < 0)
		{
			runSmokeEmitter->transform->rotation = q.conjugate();
			pv = q * pv;
		}
		else
		{
			runSmokeEmitter->transform->rotation = q;
		}

		*velocity += Vec3(0, 0.018, 0) - m_pRigidbody->floorNormal * 0.018;
	}

	// ���x�̍X�V
	*velocity += pv;

	// ���C
	if (v == Vec3::Zero())
	{
		m_isRunning = false;

		(*velocity).x *= friction;
		(*velocity).z *= friction;
	}
	else
	{
		m_isRunning = true;
	}

	// �ő呬�x�𐧌�����
	auto velocityXZ = Vec3::Scale(*velocity, 1, 0, 1);
	auto speedXZ = Vec3::dot(velocityXZ.normalized(), v) * m_speed;

	// speedXZ > 0 �Ƃ��邱�ƂŐi�s�����Ɣ��΂̕����ւ̉����͐������Ȃ�
	if (velocityXZ.length() > speedXZ && speedXZ > 0)
	{
		auto xz = velocityXZ.normalized() * speedXZ;
		(*velocity).x = xz.x;
		(*velocity).z = xz.z;
	}

	/*if (m_isGrounded)
	{
		(*velocity).y += -0.15;
	}*/
	
	// �W�����v
	if (Input::GetKeyDown(DIK_SPACE))
	{
		m_jumpFrame = 0;
		(*velocity).y = 0;
	}
	if (m_jumpFrame < m_jumpFrameMax)
	{
		if (Input::GetKey(DIK_SPACE))
		{
			(*velocity).y += 0.15 * std::pow((m_jumpFrameMax - m_jumpFrame) / (float)m_jumpFrameMax, 4);
		}
		m_jumpFrame++;
	}

	// ��]
	auto quat = transform->rotation;
	if (v.length() > 0)
	{
		auto newQuat = Quaternion::FromEuler(0, atan2(v.x, v.z), 0);
		/*auto angle = newQuat.eulerAngles().y - quat.eulerAngles().y;
		if (std::isnan(angle))
			angle = 0;
		newQuat = Quaternion::FromEuler(0, 0, angle) * newQuat;*/
		quat = Quaternion::slerp(quat, newQuat, 0.15);
	}

	transform->rotation = quat;

	//printf("%s\n", velocity->getString().c_str());
}

void Player::Animate()
{
	if (m_pAnimator == nullptr)
	{
		return;
	}

	auto runSmokeEmitter = GetEntity()->GetChild("Run Smoke Emitter")->GetComponent<ParticleEmitter>();
	auto jumpSmokeEmitter = GetEntity()->GetChild("Jump Smoke Emitter")->GetComponent<ParticleEmitter>();
	auto circleSmokeEmitter = GetEntity()->GetChild("Circle Smoke Emitter")->GetComponent<ParticleEmitter>();
	
	if (m_isGrounded)
	{
		if ((m_isRunning && !_m_isRunning) || (m_isRunning && !_m_isGrounded))
		{
			m_pAnimator->Play("Run", 2.0);
		}
		else if (!m_isRunning && _m_isRunning || (!m_isRunning && !_m_isGrounded))
		{
			m_pAnimator->Play("Idle", 2.0);
		}
		if (m_isRunning)
		{
			auto speed = Vec3::Scale(m_pRigidbody->velocity, 1, 0, 1).length();
			m_pAnimator->SetSpeed(speed * 7 + 1.0);
			runSmokeEmitter->SetSpawnRate(speed / m_speed * 0.22);
		}
		else
		{
			runSmokeEmitter->SetSpawnRate(0);
		}

		if (!_m_isGrounded && m_airCount > 10)
		{
			circleSmokeEmitter->Emit();
		}

		m_airCount = 0;
	}
	else
	{
		runSmokeEmitter->SetSpawnRate(0);

		if (_m_isGrounded && m_jumpFrame > 2)
		{
			m_pAnimator->Play("Jump_Idle", 2.0);
		}

		m_airCount++;
	}

	if (m_jumpFrame == 1)
	{
		m_pAnimator->Play("Jump", 4.0, false);
		m_pAnimator->Push("Jump_Idle", 2.0);

		jumpSmokeEmitter->Emit();
	}
	if (m_jumpFrame > 0 && m_jumpFrame < m_jumpFrameMax)
	{
		auto& prop = jumpSmokeEmitter->GetProperety();
		float frame = prop.spawnCount / prop.spawnRate;
		float rate = m_jumpFrame / frame;
		
		prop.scaleEasing.start = Vec3::lerp(Vec3(0.8, 0.8, 0.8), Vec3(0.3, 0.3, 0.3), rate);
		prop.timeToLive = 50 * (1.0 - rate) + 20;
	}
	
}

#include "Player.h"
#include "Input.h"
#include "Entity.h"
#include "Animator.h"
#include "Camera.h"
#include "Scene.h"
#include "Rigidbody.h"
#include "Physics.h"

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
	m_pAnimator = m_pEntity->GetComponent<Animator>();
	m_pRigidbody = m_pEntity->GetComponent<Rigidbody>();
	return true;
}

void Player::Update()
{
	Move();
	Animate();

	_m_isRunning = m_isRunning;
	_m_isGrounded = m_isGrounded;
	_m_jumpFrame = m_jumpFrame;

	m_groundIndex = (m_groundIndex + 1) % 5;
	m_groundBuffer[m_groundIndex] = m_pRigidbody->isGrounded;
}

void Player::Move()
{
	auto camera = m_pEntity->GetScene()->GetMainCamera();
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
	

	// 坂
	if (m_isGrounded)
	{
		auto origin = transform->position + Vec3(0, 1, 0) + *velocity + pv;
		auto direction = Vec3(0, -1, 0);
		auto distance = 5;
		RaycastHit hit;
		if (Physics::Raycast(origin, direction, distance, hit))
		{
			auto q = Quaternion::FromToRotation(Vec3(0, 1, 0), hit.normal);
			if ((q * pv).y < 0)
				pv = q * pv;

			*velocity += Vec3(0, 0.018, 0) - hit.normal * 0.018;
		}
	}

	// 速度の更新
	*velocity += pv;

	// 摩擦
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

	// 最大速度を制限する
	auto velocityXZ = Vec3::Scale(*velocity, 1, 0, 1);
	auto speedXZ = Vec3::dot(velocityXZ.normalized(), v) * m_speed;

	// speedXZ > 0 とすることで進行方向と反対の方向への加速は制限しない
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
	
	// ジャンプ
	if (Input::GetKeyDown(DIK_SPACE))
	{
		m_jumpFrame = 0;
		(*velocity).y = 0;
	}
	if (Input::GetKey(DIK_SPACE) && m_jumpFrame < m_jumpFrameMax)
	{
		(*velocity).y += 0.15 * std::pow((m_jumpFrameMax - m_jumpFrame) / (float)m_jumpFrameMax, 4);
		m_jumpFrame++;
	}

	// 回転
	auto quat = transform->rotation;
	if (v.length() > 0)
	{
		auto newQuat = Quaternion::FromEuler(0, atan2(v.x, v.z), 0);
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
			auto speed = Vec3::Scale(m_pRigidbody->velocity, 1, 0, 1).length() * 7 + 1.0;
			m_pAnimator->SetSpeed(speed);
		}
	}
	else
	{
		if (_m_isGrounded && m_jumpFrame > 2)
		{
			m_pAnimator->Play("Jump_Idle", 2.0);
		}
	}

	if (m_jumpFrame == 1)
	{
		m_pAnimator->Play("Jump", 4.0, false);
		m_pAnimator->Push("Jump_Idle", 2.0);
	}
	
}

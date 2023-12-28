#include "FloatingPlayer.h"
#include "Input.h"
#include "Entity.h"
#include "Animator.h"
#include "Camera.h"
#include "Scene.h"
#include "Rigidbody.h"

FloatingPlayer::FloatingPlayer(FloatingPlayerProperty prop)
{
	m_speed = prop.Speed;
	m_acceleration = prop.Acceleration;
}

FloatingPlayer::~FloatingPlayer()
{
}

bool FloatingPlayer::Init()
{
	m_pAnimator = GetEntity()->GetComponent<Animator>();
	m_pRigidbody = GetEntity()->GetComponent<Rigidbody>();
	return true;
}

void FloatingPlayer::Update()
{
	Move();
	Animate();
	_m_isRunning = m_isRunning;
}

void FloatingPlayer::Move()
{
	auto camera = GetEntity()->GetScene()->GetMainCamera();
	auto cameraRot = camera->transform->rotation;
	auto forward = Vec3::Scale(cameraRot * Vec3(0, 0, -1), 1, 0, 1).normalized();
	auto left = Vec3::Scale(cameraRot * Vec3(-1, 0, 0), 1, 0, 1).normalized();
	auto up = Vec3(0, 1, 0);

	Vec3 v = Vec3::Zero();

	if (Input::GetKey(DIK_LSHIFT))
	{
		if (Input::GetKey(DIK_UP))
		{
			v += up;
		}
		else if (Input::GetKey(DIK_DOWN))
		{
			v += -up;
		}
	}
	else
	{
		if (Input::GetKey(DIK_UP))
		{
			v += forward;
		}
		else if (Input::GetKey(DIK_DOWN))
		{
			v += -forward;
		}
	}

	if (Input::GetKey(DIK_LEFT))
	{
		v += left;
	}
	else if (Input::GetKey(DIK_RIGHT))
	{
		v += -left;
	}

	if (v == Vec3::Zero())
	{
		m_isRunning = false;
		m_pRigidbody->velocity.x *= 0.9;
		m_pRigidbody->velocity.z *= 0.9;
	}
	else
	{
		m_isRunning = true;
	}

	v = v.normalized();
	m_pRigidbody->velocity += v * m_acceleration;

	if (Vec3::Scale(m_pRigidbody->velocity, 1, 0, 1).length() > m_speed)
	{
		auto xz = Vec3::Scale(m_pRigidbody->velocity, 1, 0, 1).normalized() * m_speed;
		m_pRigidbody->velocity.x = xz.x;
		m_pRigidbody->velocity.z = xz.z;
	}

	//transform->position += m_pRigidbody->velocity;

	if (Input::GetKeyDown(DIK_SPACE))
	{
		m_pRigidbody->velocity.y += 0.3;
	}

	// ‰ñ“]
	auto quat = transform->rotation;
	if (v.length() > 0)
	{
		auto newQuat = Quaternion::FromEuler(0, atan2(v.x, v.z), 0);
		quat = Quaternion::slerp(quat, newQuat, 0.2);
	}
	transform->rotation = quat;
}

void FloatingPlayer::Animate()
{
	if (m_pAnimator == nullptr)
	{
		return;
	}

	if (m_isRunning == true && _m_isRunning == false)
	{
		m_pAnimator->Play("Run", 2.0);
	}
	else if (m_isRunning == false && _m_isRunning == true)
	{
		m_pAnimator->Play("Idle", 2.0);
	}
}

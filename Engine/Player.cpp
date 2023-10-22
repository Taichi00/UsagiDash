#include "Player.h"
#include "Input.h"
#include "Entity.h"
#include "Animator.h"
#include "Camera.h"
#include "Game.h"

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
	return true;
}

void Player::Update()
{
	Move();
	Animate();
	_m_isRunning = m_isRunning;
}

void Player::Move()
{
	auto camera = m_pEntity->GetGame()->GetMainCamera();
	auto cameraRot = camera->transform->rotation;
	auto forward = Vec3::Scale(cameraRot * Vec3(0, 0, -1), 1, 0, 1).normalized();
	auto left = Vec3::Scale(cameraRot * Vec3(-1, 0, 0), 1, 0, 1).normalized();
	
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
	
	if (v == Vec3::Zero())
	{
		m_isRunning = false;
		m_velocity = m_velocity * 0.9;
	}
	else
	{
		m_isRunning = true;
	}

	v = v.normalized();
	m_velocity += v * m_acceleration;
	
	if (m_velocity.length() > m_speed)
	{
		m_velocity = m_velocity.normalized() * m_speed;
	}
	
	transform->position += m_velocity;

	// ‰ñ“]
	auto quat = transform->rotation;
	if (v.length() > 0)
	{
		auto newQuat = Quaternion::FromEuler(0, atan2(v.x, v.z), 0);
		quat = Quaternion::slerp(quat, newQuat, 0.2);
	}
	transform->rotation = quat;
}

void Player::Animate()
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

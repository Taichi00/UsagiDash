#include "GameCamera.h"
#include "Input.h"
#include "Entity.h"
#include "Camera.h"
#include "Physics.h"

GameCamera::GameCamera(GameCameraProperty prop)
{
	m_pTarget = prop.Target;
	m_angle = Vec3(0.5, 0, 0);
	m_currDistance = m_distance;
}

GameCamera::~GameCamera()
{
}

bool GameCamera::Init()
{
	m_pCamera = m_pEntity->GetComponent<Camera>();
	return true;
}

void GameCamera::BeforeCameraUpdate()
{
	Move();
}

void GameCamera::Move()
{
	float rate = 0.05;

	if (Input::GetKey(DIK_A))
	{
		m_angleVelocity.y += -0.002;
	}
	else if (Input::GetKey(DIK_D))
	{
		m_angleVelocity.y += 0.002;
	}
	else
	{
		m_angleVelocity.y *= 0.92;
	}

	if (Input::GetKey(DIK_W))
	{
		m_angleVelocity.x += -0.002;
	}
	else if (Input::GetKey(DIK_S))
	{
		m_angleVelocity.x += 0.002;
	}
	else
	{
		m_angleVelocity.x *= 0.92;
	}

	m_angleVelocity.y = min(max(m_angleVelocity.y, -0.04), 0.04);
	m_angleVelocity.x = min(max(m_angleVelocity.x, -0.04), 0.04);
	m_angle += m_angleVelocity;

	if (m_angle.x < -1.5)
	{
		m_angle.x = -1.5;
		m_angleVelocity.x = 0;
	}
	else if (m_angle.x > 1.5)
	{
		m_angle.x = 1.5;
		m_angleVelocity.x = 0;
	}

	Vec3 targetPos = m_pTarget->transform->position + Vec3(0, 3, 0);
	Vec3 currPos = m_pCamera->GetFocusPosition();
	Vec3 v = targetPos - currPos;

	auto origin = currPos + v * rate;
	auto direction = Quaternion::FromEuler(m_angle.x, m_angle.y, m_angle.z) * Vec3(0, 0, 1);
	auto distance = m_distance * max(0.5, min(1.5, 0.75 + m_angle.x * 0.8));

	// めり込み防止
	RaycastHit hit;
	if (Physics::Raycast(origin, direction, distance, hit))
	{
		distance = max(hit.distance - 0.1, 0);
	}

	// ズームインは素早く、ズームアウトはゆっくり
	auto drate = 0.15;
	if (distance - m_currDistance > 0) drate = 0.02;

	distance = m_currDistance + (distance - m_currDistance) * drate;
	
	transform->position = Vec3(origin + direction * distance);
	m_pCamera->SetFocusPosition(origin);

	m_currDistance = distance;
}

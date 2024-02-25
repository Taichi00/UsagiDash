#include "app/floating_player.h"
#include "game/input/input.h"
#include "game/entity.h"
#include "game/component/animator.h"
#include "game/component/camera.h"
#include "game/scene.h"
#include "game/component/rigidbody.h"
#include <dinput.h>

FloatingPlayer::FloatingPlayer(FloatingPlayerProperty prop)
{
	speed_ = prop.speed;
	acceleration_ = prop.acceleration;
}

FloatingPlayer::~FloatingPlayer()
{
}

bool FloatingPlayer::Init()
{
	animator_ = GetEntity()->GetComponent<Animator>();
	rigidbody_ = GetEntity()->GetComponent<Rigidbody>();
	return true;
}

void FloatingPlayer::Update(const float delta_time)
{
	Move();
	Animate();
	is_running_prev_ = is_running_;
}

void FloatingPlayer::Move()
{
	auto camera = GetEntity()->GetScene()->GetMainCamera();
	auto cameraRot = camera->transform->rotation;
	auto forward = Vec3::Scale(cameraRot * Vec3(0, 0, -1), 1, 0, 1).Normalized();
	auto left = Vec3::Scale(cameraRot * Vec3(-1, 0, 0), 1, 0, 1).Normalized();
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
		is_running_ = false;
		rigidbody_->velocity.x *= 0.9f;
		rigidbody_->velocity.z *= 0.9f;
	}
	else
	{
		is_running_ = true;
	}

	v = v.Normalized();
	rigidbody_->velocity += v * acceleration_;

	if (Vec3::Scale(rigidbody_->velocity, 1, 0, 1).Length() > speed_)
	{
		auto xz = Vec3::Scale(rigidbody_->velocity, 1, 0, 1).Normalized() * speed_;
		rigidbody_->velocity.x = xz.x;
		rigidbody_->velocity.z = xz.z;
	}

	//transform->position += m_pRigidbody->velocity;

	if (Input::GetKeyDown(DIK_SPACE))
	{
		rigidbody_->velocity.y += 0.3f;
	}

	// ‰ñ“]
	auto quat = transform->rotation;
	if (v.Length() > 0)
	{
		auto newQuat = Quaternion::FromEuler(0, atan2(v.x, v.z), 0);
		quat = Quaternion::Slerp(quat, newQuat, 0.2f);
	}
	transform->rotation = quat;
}

void FloatingPlayer::Animate()
{
	if (animator_ == nullptr)
	{
		return;
	}

	if (is_running_ == true && is_running_prev_ == false)
	{
		animator_->Play("Run", 2.0);
	}
	else if (is_running_ == false && is_running_prev_ == true)
	{
		animator_->Play("Idle", 2.0);
	}
}

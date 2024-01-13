#include "app/player.h"
#include "game/input.h"
#include "game/entity.h"
#include "game/component/animator.h"
#include "game/component/camera.h"
#include "game/scene.h"
#include "game/component/rigidbody.h"
#include "game/physics.h"
#include "game/component/particle_emitter.h"

Player::Player(PlayerProperty prop)
{
	speed_ = prop.speed;
	acceleration_ = prop.acceleration;
	
}

Player::~Player()
{
}

bool Player::Init()
{
	animator_ = GetEntity()->GetComponent<Animator>();
	rigidbody_ = GetEntity()->GetComponent<Rigidbody>();
	return true;
}

void Player::Update()
{
	Move();
	Animate();

	is_running_prev_ = is_running_;
	is_grouded_prev_ = is_grounded_;
	jump_frame_prev_ = jump_frame_;

}

void Player::Move()
{
	auto camera = GetEntity()->GetScene()->GetMainCamera();
	auto cameraRot = camera->transform->rotation;
	auto forward = Vec3::Scale(cameraRot * Vec3(0, 0, -1), 1, 0, 1).Normalized();
	auto left = Vec3::Scale(cameraRot * Vec3(-1, 0, 0), 1, 0, 1).Normalized();

	/*m_isGrounded = false;
	for (int i = 0; i < 5; i++)
	{
		m_isGrounded |= m_groundBuffer[i];
	}*/
	is_grounded_ = rigidbody_->is_grounded;

	float friction = 0.9;
	float acceleration = acceleration_;

	if (!is_grounded_)
	{
		friction = 0.99;
		acceleration *= 0.4;
	}

	auto velocity = &(rigidbody_->velocity);
	
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

	v = v.Normalized();

	auto pv = v * acceleration_;
	

	// 坂
	if (is_grounded_)
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

		auto q = Quaternion::FromToRotation(Vec3(0, 1, 0), rigidbody_->floor_normal);
		if ((q * pv).y < 0)
		{
			runSmokeEmitter->transform->rotation = q.Conjugate();
			pv = q * pv;
		}
		else
		{
			runSmokeEmitter->transform->rotation = q;
		}

		*velocity += Vec3(0, 0.018, 0) - rigidbody_->floor_normal * 0.018;
	}

	// 速度の更新
	*velocity += pv;

	// 摩擦
	if (v == Vec3::Zero())
	{
		is_running_ = false;

		(*velocity).x *= friction;
		(*velocity).z *= friction;
	}
	else
	{
		is_running_ = true;
	}

	// 最大速度を制限する
	auto velocityXZ = Vec3::Scale(*velocity, 1, 0, 1);
	auto speedXZ = Vec3::Dot(velocityXZ.Normalized(), v) * speed_;

	// speedXZ > 0 とすることで進行方向と反対の方向への加速は制限しない
	if (velocityXZ.Length() > speedXZ && speedXZ > 0)
	{
		auto xz = velocityXZ.Normalized() * speedXZ;
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
		jump_frame_ = 0;
		(*velocity).y = 0;
	}
	if (jump_frame_ < jump_frame_max_)
	{
		if (Input::GetKey(DIK_SPACE))
		{
			(*velocity).y += 0.15 * std::pow((jump_frame_max_ - jump_frame_) / (float)jump_frame_max_, 4);
		}
		jump_frame_++;
	}

	// 回転
	auto quat = transform->rotation;
	if (v.Length() > 0)
	{
		auto newQuat = Quaternion::FromEuler(0, atan2(v.x, v.z), 0);
		/*auto angle = newQuat.eulerAngles().y - quat.eulerAngles().y;
		if (std::isnan(angle))
			angle = 0;
		newQuat = Quaternion::FromEuler(0, 0, angle) * newQuat;*/
		quat = Quaternion::Slerp(quat, newQuat, 0.15);
	}

	transform->rotation = quat;

	//printf("%s\n", velocity->getString().c_str());
}

void Player::Animate()
{
	if (animator_ == nullptr)
	{
		return;
	}

	auto runSmokeEmitter = GetEntity()->GetChild("Run Smoke Emitter")->GetComponent<ParticleEmitter>();
	auto jumpSmokeEmitter = GetEntity()->GetChild("Jump Smoke Emitter")->GetComponent<ParticleEmitter>();
	auto circleSmokeEmitter = GetEntity()->GetChild("Circle Smoke Emitter")->GetComponent<ParticleEmitter>();
	
	if (is_grounded_)
	{
		if ((is_running_ && !is_running_prev_) || (is_running_ && !is_grouded_prev_))
		{
			animator_->Play("Run", 2.0);
		}
		else if (!is_running_ && is_running_prev_ || (!is_running_ && !is_grouded_prev_))
		{
			animator_->Play("Idle", 2.0);
		}
		if (is_running_)
		{
			auto speed = Vec3::Scale(rigidbody_->velocity, 1, 0, 1).Length();
			animator_->SetSpeed(speed * 7 + 1.0);
			runSmokeEmitter->SetSpawnRate(speed / speed_ * 0.22);
		}
		else
		{
			runSmokeEmitter->SetSpawnRate(0);
		}

		if (!is_grouded_prev_ && air_count_ > 10)
		{
			circleSmokeEmitter->Emit();
		}

		air_count_ = 0;
	}
	else
	{
		runSmokeEmitter->SetSpawnRate(0);

		if (is_grouded_prev_ && jump_frame_ > 2)
		{
			animator_->Play("Jump_Idle", 2.0);
		}

		air_count_++;
	}

	if (jump_frame_ == 1)
	{
		animator_->Play("Jump", 4.0, false);
		animator_->Push("Jump_Idle", 2.0);

		jumpSmokeEmitter->Emit();
	}
	if (jump_frame_ > 0 && jump_frame_ < jump_frame_max_)
	{
		auto& prop = jumpSmokeEmitter->GetProperety();
		float frame = prop.spawn_count / prop.spawn_rate;
		float rate = jump_frame_ / frame;
		
		prop.scale_easing.start = Vec3::Lerp(Vec3(0.8, 0.8, 0.8), Vec3(0.3, 0.3, 0.3), rate);
		prop.time_to_live = 50 * (1.0 - rate) + 20;
	}
	
}

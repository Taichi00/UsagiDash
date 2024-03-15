#pragma once

#include "game/component/component.h"
#include "math/vec.h"

class Rigidbody;
class Collider;
class AudioSource;

class JumpLiftController : public Component
{
public:
	JumpLiftController(
		const Vec3& direction, 
		const float distance,
		AudioSource* audio_move,
		AudioSource* audio_stop
	);
	~JumpLiftController();

	bool Init() override;
	void Update(const float delta_time) override;

	void OnCollisionEnter(Collider* collider) override;

private:
	// ��������
	Vec3 direction_;
	// ��������
	float distance_;

	// �����x
	float acceleration_ = 100.0f;

	// �����ʒu�ɂ��ǂ鑬�x
	float back_speed_ = 15.0f;

	// �����ʒu
	Vec3 start_position_;
	Vec3 end_position_;

	bool is_moving_ = false;
	bool is_back_ = false;

	Rigidbody* rigidbody_ = nullptr;
	AudioSource* audio_move_ = nullptr;
	AudioSource* audio_stop_ = nullptr;
};
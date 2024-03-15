#pragma once

#include "game/component/component.h"
#include "math/vec.h"

class Rigidbody;
class Collider;

class JumpRiftController : public Component
{
public:
	JumpRiftController(const Vec3& direction, const float distance);
	~JumpRiftController();

	bool Init() override;
	void Update(const float delta_time) override;

	void OnCollisionEnter(Collider* collider) override;

private:
	// 動く方向
	Vec3 direction_;
	// 動く距離
	float distance_;

	// 加速度
	float acceleration_ = 100.0f;

	// 初期位置にもどる速度
	float back_speed_ = 15.0f;

	// 初期位置
	Vec3 start_position_;
	Vec3 end_position_;

	bool is_moving_ = false;
	bool is_back_ = false;

	Rigidbody* rigidbody_ = nullptr;
};
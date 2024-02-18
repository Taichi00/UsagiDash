#pragma once

#include "game/component/component.h"
#include "math/vec.h"

class Animator;
class Rigidbody;

struct FloatingPlayerProperty
{
	float speed;
	float acceleration;
};

class FloatingPlayer : public Component
{
public:
	FloatingPlayer(FloatingPlayerProperty prop);
	~FloatingPlayer();

	bool Init() override;
	void Update(const float delta_time) override;

private:
	void Move();
	void Animate();

private:
	Vec3 velocity_;
	float speed_;
	float acceleration_;
	float angle_;

	bool is_running_, is_running_prev_;

	Animator* animator_;
	Rigidbody* rigidbody_;
};
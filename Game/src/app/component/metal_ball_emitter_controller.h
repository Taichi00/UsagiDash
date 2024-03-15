#pragma once

#include "game/component/component.h"
#include <random>

class MetalBallEmitterController : public Component
{
public:
	MetalBallEmitterController(const float radius, const float width, const float spawn_rate);
	~MetalBallEmitterController();

	bool Init() override;
	void Update(const float delta_time) override;

private:
	// �S���̔��a
	float radius_;
	// �����͈͂̕�
	float width_;
	// �����Ԋu
	float spawn_rate_;

	// �^�C�}�[
	float spawn_timer_ = 0;

	// ����������
	std::mt19937 rand_; 
};
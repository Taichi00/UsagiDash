#pragma once

#include "game/component/component.h"
#include "math/vec.h"
#include "math/quaternion.h"

class Entity;
class Camera;

class CameraController : public Component
{
public:
	CameraController(Entity* target);
	~CameraController();

	bool Init() override;

	void BeforeCameraUpdate(const float delta_time) override;

	// �����I�ɍ��̃^�[�Q�b�g�̈ʒu�Ɉړ�������
	void ForceMove(const float angle);

private:
	void Rotate(const float delta_time);
	void Move(const float delta_time, const float move_speed);

private:
	Entity* target_ = nullptr;
	Camera* camera_ = nullptr;

	float angle_speed_;
	float angle_acceleration_;
	float angle_friction_;

	Vec3 angle_velocity_;
	Vec3 angle_;

	Vec3 target_position_;

	float distance_; // �J�����̃f�t�H���g����
	float current_distance_;

	float focus_height_; // �t�H�[�J�X����ʒu�̍���

	float move_speed_; // �ړ����鑬��
	float zoom_in_speed_; // �Y�[�����鑬��
	float zoom_out_speed_;
};
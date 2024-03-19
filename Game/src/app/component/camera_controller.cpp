#include "app/component/camera_controller.h"
#include "game/input/input.h"
#include "game/entity.h"
#include "game/component/camera.h"
#include "game/physics.h"

CameraController::CameraController(Entity* target)
{
	angle_speed_ = 2.4f;
	angle_acceleration_ = 0.12f;
	angle_friction_ = 0.92f;
	distance_ = 20.f;

	target_ = target;
	angle_ = Vec3(0.5, 0, 0);
	current_distance_ = distance_;

	focus_height_ = 3;

	move_speed_ = 0.05f;
	zoom_in_speed_ = 0.15f;
	zoom_out_speed_ = 0.02f;
}

CameraController::~CameraController()
{
}

bool CameraController::Init()
{
	camera_ = GetEntity()->GetComponent<Camera>();
	return true;
}

void CameraController::BeforeCameraUpdate(const float delta_time)
{
	Rotate(delta_time);
	Move(delta_time, move_speed_);
}

void CameraController::ForceMove()
{
	Move(0, 1);
}

void CameraController::Rotate(const float delta_time)
{
	Vec2 angle_speed = Vec2(1, 1) * angle_speed_;
	float angle_acceleration = angle_acceleration_;
	float angle_friction = angle_friction_;

	// ���݂̓��͎��
	auto input_type = Input::CurrentInputType();

	// �������͂��擾
	Vec2 input_dir;
	input_dir.x = Input::GetAxis("camera_horizontal");
	input_dir.y = Input::GetAxis("camera_vertical");

	float input_length = input_dir.Length();

	// �Q�[���p�b�h�p�̒l��ݒ�
	if (input_type == Input::InputType::GAMEPAD)
	{
		auto speed = angle_speed * 1.2f;
		if (std::abs(angle_velocity_.y) < speed.y * std::abs(input_dir.x) * 1.1f)
		{
			angle_speed.y = speed.y * std::abs(input_dir.x);
		}
		else
		{
			angle_speed.y = std::abs(angle_velocity_.y);
		}

		if (std::abs(angle_velocity_.x) < speed.x * std::abs(input_dir.y) * 1.1f)
		{
			angle_speed.x = speed.x * std::abs(input_dir.y);
		}
		else
		{
			angle_speed.x = std::abs(angle_velocity_.x);
		}

		angle_acceleration = angle_acceleration_ * 1.5f;
		angle_friction = 0.9f;
	}
	
	// ����
	angle_velocity_.y += input_dir.x * angle_acceleration;
	angle_velocity_.x += -input_dir.y * angle_acceleration;

	// ���C�Ŏ~�܂�
	if (input_dir.y == 0.f)
		angle_velocity_.x *= angle_friction;

	if (input_dir.x == 0.f)
		angle_velocity_.y *= angle_friction;

	// ���x�𐧌�
	angle_velocity_.y = std::min(std::max(angle_velocity_.y, -angle_speed.y), angle_speed.y);
	angle_velocity_.x = std::min(std::max(angle_velocity_.x, -angle_speed.x), angle_speed.x);

	// �p�x���X�V
	angle_ += angle_velocity_ * delta_time;

	// x����]�𐧌�
	if (angle_.x < -1.5)
	{
		angle_.x = -1.5;
		angle_velocity_.x = 0;
	}
	else if (angle_.x > 1.5)
	{
		angle_.x = 1.5;
		angle_velocity_.x = 0;
	}
}

void CameraController::Move(const float delta_time, const float move_speed)
{
	Vec3 target_pos = target_->transform->position + Vec3(0, focus_height_, 0);
	Vec3 curr_pos = camera_->GetFocusPosition();

	Vec3 v = target_pos - curr_pos;

	auto origin = curr_pos + v * move_speed;
	auto direction = Quaternion::FromEuler(angle_) * Vec3(0, 0, 1);
	auto distance = distance_;

	// X�p�x�ɂ���ċ�����ς���
	distance *= std::max(0.5f, std::min(1.5f, 0.75f + angle_.x * 0.8f));

	// �߂荞�ݖh�~
	RaycastHit hit;
	if (Physics::Raycast(origin, direction, distance, hit, {"map"}))
	{
		distance = std::max(hit.distance - 0.1f, 0.f);
	}

	// �Y�[���C���͑f�����A�Y�[���A�E�g�͂������
	float zoom_speed;
	if (distance - current_distance_ > 0)
	{
		zoom_speed = zoom_out_speed_;
	}
	else
	{
		zoom_speed = zoom_in_speed_;
	}

	// �J�����������v�Z
	distance = current_distance_ + (distance - current_distance_) * zoom_speed;
	current_distance_ = distance;
	
	// �ʒu���X�V
	transform->position = Vec3(origin + direction * distance);
	camera_->SetFocusPosition(origin);
}

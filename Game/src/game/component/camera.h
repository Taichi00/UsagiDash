#pragma once
#include "game/entity.h"
#include "engine/engine.h"
#include "math/vec.h"
#include "math/quaternion.h"
#include "game/component/component.h"

using namespace DirectX;

class Camera : public Component
{
public:
	Camera();

	// フォーカスモード切替
	void Focus(bool flag);

	XMMATRIX GetViewMatrix() const;
	XMMATRIX GetProjMatrix() const;

	Vec3 GetFocusPosition() const;
	Vec3 GetViewDirection() const;

	void SetFocusPosition(const Vec3& focus_pos);
	void SetFocusPosition(float x, float y, float z);

	void SetFocusTarget(Entity* focus_target);

	void SetUpDirection(const Vec3& up_dir);
	void SetUpDirection(float x, float y, float z);

	void SetFov(float fov);

	Vec3 Position() const { return position_; }
	Quaternion Rotation() const { return rotation_; }

private:
	bool Init() override;
	void CameraUpdate(const float delta_time) override;

private:
	XMMATRIX mtx_view_;
	XMMATRIX mtx_proj_;

	Vec3 position_;
	Quaternion rotation_;

	// 注視点
	Vec3 focus_position_;
	// 注視対象エンティティ
	Entity* focus_target_ = nullptr;
	// 上方向ベクトル
	Vec3 up_direction_;

	float fov_;
	float near_, far_;

	bool focus_;
};
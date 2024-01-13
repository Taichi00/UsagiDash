#pragma once
#include "game/entity.h"
#include "engine/engine.h"
#include "math/vec.h"
#include "game/component/component.h"

using namespace DirectX;

class Camera : public Component
{
public:

	XMMATRIX GetViewMatrix();
	XMMATRIX GetProjMatrix();

	Vec3 GetFocusPosition();
	Vec3 GetViewDirection();

	void SetFocusPosition(XMFLOAT3 focusPos);
	void SetFocusPosition(float x, float y, float z);

	void SetUpDirection(XMFLOAT3 upDir);
	void SetUpDirection(float x, float y, float z);

	void SetFov(float fov);

private:
	bool Init() override;
	void CameraUpdate() override;

private:
	XMMATRIX mtx_view_;
	XMMATRIX mtx_proj_;

	XMFLOAT3 focus_pos_;
	XMFLOAT3 up_dir_;
	float fov_;
	float near_, far_;
};
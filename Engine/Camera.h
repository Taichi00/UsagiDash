#pragma once
#include "Entity.h"
#include "Engine.h"
#include "Vec.h"
#include "Component.h"

class Game;

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
	XMMATRIX m_mtxView;
	XMMATRIX m_mtxProj;

	XMFLOAT3 m_focusPos;
	XMFLOAT3 m_upDir;
	float m_fov;
	float m_near, m_far;
};
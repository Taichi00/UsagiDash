#pragma once

#include "game/component/component.h"
#include "math/vec.h"
#include "math/quaternion.h"
#include <vector>

using namespace DirectX;

class Transform : public Component
{
public:
	Transform();

	bool Init() override;
	void TransformUpdate(const float delta_time) override;

	// ���[���h�ϊ��s����擾
	XMMATRIX WorldMatrix() { return world_matrix_; }; 

	// Billboard�p�̃��[���h�ϊ��s����擾
	XMMATRIX BillboardWorldMatrix(); 

	Vec3 WorldPosition() { return world_position_; }
	Quaternion WorldRotation();

private:
	void MulParentWorld(XMMATRIX& world);	// �eEntity�̃��[���h�ϊ��s���������

public:
	Vec3 position;
	Vec3 scale;
	Quaternion rotation;

private:
	XMMATRIX world_matrix_;
	Vec3 world_position_;
};
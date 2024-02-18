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

	bool Init();

	XMMATRIX GetWorldMatrix();			// ���[���h�ϊ��s��
	XMMATRIX GetBillboardWorldMatrix();	// Billboard�p�̃��[���h�ϊ��s��
	Quaternion GetWorldRotation();

private:
	void MulParentWorld(XMMATRIX& world);	// �eEntity�̃��[���h�ϊ��s���������

public:
	Vec3 position;
	Vec3 scale;
	Quaternion rotation;
};
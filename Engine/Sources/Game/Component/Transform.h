#pragma once

#include "Component.h"
#include "Vec.h"
#include "Quaternion.h"
#include <vector>

using namespace DirectX;

class Transform : public Component
{
public:
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
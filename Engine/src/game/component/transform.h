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

	XMMATRIX GetWorldMatrix();			// ワールド変換行列
	XMMATRIX GetBillboardWorldMatrix();	// Billboard用のワールド変換行列
	Quaternion GetWorldRotation();

private:
	void MulParentWorld(XMMATRIX& world);	// 親Entityのワールド変換行列をかける

public:
	Vec3 position;
	Vec3 scale;
	Quaternion rotation;
};
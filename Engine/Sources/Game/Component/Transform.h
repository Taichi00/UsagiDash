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
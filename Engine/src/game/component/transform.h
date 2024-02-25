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

	// ワールド変換行列を取得
	XMMATRIX WorldMatrix() { return world_matrix_; }; 

	// Billboard用のワールド変換行列を取得
	XMMATRIX BillboardWorldMatrix(); 

	Vec3 WorldPosition() { return world_position_; }
	Quaternion WorldRotation();

private:
	void MulParentWorld(XMMATRIX& world);	// 親Entityのワールド変換行列をかける

public:
	Vec3 position;
	Vec3 scale;
	Quaternion rotation;

private:
	XMMATRIX world_matrix_;
	Vec3 world_position_;
};
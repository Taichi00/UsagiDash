#pragma once
#include "math/vec.h"
#include "math/quaternion.h"
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <memory>

using namespace DirectX;

class Bone
{
public:
	Bone() = delete;
	Bone(const std::string& name);
	Bone(const Bone& bone);

	// 変換行列を更新する
	void UpdateMatrices();

	void SetPosition(Vec3 pos) { position_ = pos; }
	void SetRotation(Quaternion rot) { rotation_ = rot; }
	void SetScale(Vec3 scale) { scale_ = scale; }
	void SetInvBindMatrix(XMMATRIX mtx) { mtx_inv_bind_ = mtx; }
	void SetGlobalMatrix(XMMATRIX mtx) { mtx_global_ = mtx; }

	void SetParent(Bone* parent);
	void AddChild(Bone* child);

	Vec3 Position() const { return position_; }
	Quaternion Rotation() const { return rotation_; }
	Vec3 Scale() const { return scale_; }
	XMMATRIX InvBindMatrix() const { return mtx_inv_bind_; }
	XMMATRIX LocalMatrix() const { return mtx_local_; }
	XMMATRIX WorldMatrix() const { return mtx_world_; }
	XMMATRIX GlobalMatrix() const { return mtx_global_; }
	std::string Name() const { return name_; }
	Bone* Parent() const { return parent_; }
	std::vector<Bone*> Children() const { return children_; }

	// 現在の状態をバッファに保存
	void SaveBuffer();

	Vec3 PositionBuffer() const { return position_buf_; }
	Quaternion RotationBuffer() const { return rotation_buf_; }
	Vec3 ScaleBuffer() const { return scale_buf_; }

private:
	// ローカル座標変換行列を更新する
	void UpdateLocalMatrix();

	// ワールド座標変換行列を更新する
	void UpdateWorldMatrix();

private:
	Vec3 position_, position_buf_;
	Quaternion rotation_, rotation_buf_;
	Vec3 scale_, scale_buf_;

	XMMATRIX mtx_local_;
	XMMATRIX mtx_world_;
	XMMATRIX mtx_inv_bind_;
	XMMATRIX mtx_global_;

	std::string name_;
	Bone* parent_ = nullptr;
	std::vector<Bone*> children_;
};
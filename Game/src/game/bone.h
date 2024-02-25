#pragma once
#include <string>
#include <DirectXMath.h>
#include <vector>

using namespace DirectX;

class Bone
{
public:
	Bone();
	Bone(const std::string name);

	void UpdateMatrices();

	void SetPosition(XMFLOAT3 pos);
	void SetRotation(XMFLOAT4 rot);
	void SetScale(XMFLOAT3 scale);
	void SetInvBindMatrix(XMMATRIX mtx);
	void SetGlobalMatrix(XMMATRIX mtx);
	void SetParent(Bone* parent);
	void AddChild(Bone* child);

	XMVECTOR GetPosition();
	XMVECTOR GetRotation();
	XMVECTOR GetScale();
	XMMATRIX GetInvBindMatrix();
	XMMATRIX GetLocalMatrix();
	XMMATRIX GetWorldMatrix();
	XMMATRIX GetGlobalMatrix();
	std::string GetName();
	Bone* GetParent();
	std::vector<Bone*> GetChildren();

	void SaveBuffer();
	XMVECTOR GetPositionBuffer();
	XMVECTOR GetRotationBuffer();
	XMVECTOR GetScaleBuffer();

private:
	void UpdateLocalMatrix();
	void UpdateWorldMatrix();

private:
	XMVECTOR position_, position_buf_;
	XMVECTOR rotation_, rotation_buf_;
	XMVECTOR scale_, scale_buf_;
	XMMATRIX mtx_local_;
	XMMATRIX mtx_world_;
	XMMATRIX mtx_inv_bind_;
	XMMATRIX mtx_global_;
	std::string name_;
	Bone* parent_ = nullptr;
	std::vector<Bone*> children_;
};
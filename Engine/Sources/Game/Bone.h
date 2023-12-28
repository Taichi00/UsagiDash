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
	XMVECTOR m_position, m_positionBuf;
	XMVECTOR m_rotation, m_rotationBuf;
	XMVECTOR m_scale, m_scaleBuf;
	XMMATRIX m_mtxLocal;
	XMMATRIX m_mtxWorld;
	XMMATRIX m_mtxInvBind;
	XMMATRIX m_mtxGlobal;
	std::string m_name;
	Bone* m_pParent = nullptr;
	std::vector<Bone*> m_pChildren;
};
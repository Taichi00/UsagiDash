#include "Bone.h"

Bone::Bone()
{
}

Bone::Bone(const std::string name)
{
	m_name = name;
	m_rotation = XMQuaternionIdentity();
	m_mtxInvBind = XMMatrixIdentity();
	m_mtxLocal = XMMatrixIdentity();
	m_mtxWorld = XMMatrixIdentity();
}

void Bone::SetPosition(XMFLOAT3 pos)
{
	m_position = XMLoadFloat3(&pos);
}

void Bone::SetRotation(XMFLOAT4 rot)
{
	m_rotation = XMLoadFloat4(&rot);
}

void Bone::SetScale(XMFLOAT3 scale)
{
	m_scale = XMLoadFloat3(&scale);
}

void Bone::SetInvBindMatrix(XMMATRIX mtx)
{
	m_mtxInvBind = mtx;
}

void Bone::SetGlobalMatrix(XMMATRIX mtx)
{
	m_mtxGlobal = mtx;
}

void Bone::SetParent(Bone* parent)
{
	m_pParent = parent;
}

void Bone::AddChild(Bone* child)
{
	m_pChildren.push_back(child);
}

XMVECTOR Bone::GetPosition()
{
	return m_position;
}

XMVECTOR Bone::GetRotation()
{
	return m_rotation;
}

XMVECTOR Bone::GetScale()
{
	return m_scale;
}

XMMATRIX Bone::GetInvBindMatrix()
{
	return m_mtxInvBind;
}

XMMATRIX Bone::GetLocalMatrix()
{
	return m_mtxLocal;
}

XMMATRIX Bone::GetWorldMatrix()
{
	return m_mtxWorld;
}

XMMATRIX Bone::GetGlobalMatrix()
{
	return m_mtxGlobal;
}

std::string Bone::GetName()
{
	return m_name;
}

Bone* Bone::GetParent()
{
	return m_pParent;
}

std::vector<Bone*> Bone::GetChildren()
{
	return m_pChildren;
}

void Bone::SaveBuffer()
{
	m_positionBuf = m_position;
	m_rotationBuf = m_rotation;
	m_scaleBuf = m_scale;
}

XMVECTOR Bone::GetPositionBuffer()
{
	return m_positionBuf;
}

XMVECTOR Bone::GetRotationBuffer()
{
	return m_rotationBuf;
}

XMVECTOR Bone::GetScaleBuffer()
{
	return m_scaleBuf;
}

void Bone::UpdateLocalMatrix()
{
	auto mtx = XMMatrixScalingFromVector(m_scale)
		* XMMatrixRotationQuaternion(m_rotation)
		* XMMatrixTranslationFromVector(m_position);
	m_mtxLocal = XMMatrixTranspose(mtx);

}

void Bone::UpdateWorldMatrix()
{
	UpdateLocalMatrix();
	auto mtxParent = XMMatrixIdentity();
	if (m_pParent)
	{
		mtxParent = m_pParent->GetWorldMatrix();
	}
	m_mtxWorld = mtxParent * m_mtxLocal;//mtxParent * m_mtxGlobal * m_mtxLocal;
}

void Bone::UpdateMatrices()
{
	UpdateWorldMatrix();
	for (auto c : m_pChildren)
	{
		c->UpdateMatrices();
	}
}

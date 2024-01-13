#include "game/bone.h"

Bone::Bone()
{
}

Bone::Bone(const std::string name)
{
	name_ = name;
	rotation_ = XMQuaternionIdentity();
	mtx_inv_bind_ = XMMatrixIdentity();
	mtx_local_ = XMMatrixIdentity();
	mtx_world_ = XMMatrixIdentity();
}

void Bone::SetPosition(XMFLOAT3 pos)
{
	position_ = XMLoadFloat3(&pos);
}

void Bone::SetRotation(XMFLOAT4 rot)
{
	rotation_ = XMLoadFloat4(&rot);
}

void Bone::SetScale(XMFLOAT3 scale)
{
	scale_ = XMLoadFloat3(&scale);
}

void Bone::SetInvBindMatrix(XMMATRIX mtx)
{
	mtx_inv_bind_ = mtx;
}

void Bone::SetGlobalMatrix(XMMATRIX mtx)
{
	mtx_global_ = mtx;
}

void Bone::SetParent(Bone* parent)
{
	parent_ = parent;
}

void Bone::AddChild(Bone* child)
{
	children_.push_back(child);
}

XMVECTOR Bone::GetPosition()
{
	return position_;
}

XMVECTOR Bone::GetRotation()
{
	return rotation_;
}

XMVECTOR Bone::GetScale()
{
	return scale_;
}

XMMATRIX Bone::GetInvBindMatrix()
{
	return mtx_inv_bind_;
}

XMMATRIX Bone::GetLocalMatrix()
{
	return mtx_local_;
}

XMMATRIX Bone::GetWorldMatrix()
{
	return mtx_world_;
}

XMMATRIX Bone::GetGlobalMatrix()
{
	return mtx_global_;
}

std::string Bone::GetName()
{
	return name_;
}

Bone* Bone::GetParent()
{
	return parent_;
}

std::vector<Bone*> Bone::GetChildren()
{
	return children_;
}

void Bone::SaveBuffer()
{
	position_buf_ = position_;
	rotation_buf_ = rotation_;
	scale_buf_ = scale_;
}

XMVECTOR Bone::GetPositionBuffer()
{
	return position_buf_;
}

XMVECTOR Bone::GetRotationBuffer()
{
	return rotation_buf_;
}

XMVECTOR Bone::GetScaleBuffer()
{
	return scale_buf_;
}

void Bone::UpdateLocalMatrix()
{
	auto mtx = XMMatrixScalingFromVector(scale_)
		* XMMatrixRotationQuaternion(rotation_)
		* XMMatrixTranslationFromVector(position_);
	mtx_local_ = XMMatrixTranspose(mtx);

}

void Bone::UpdateWorldMatrix()
{
	UpdateLocalMatrix();
	auto mtxParent = XMMatrixIdentity();
	if (parent_)
	{
		mtxParent = parent_->GetWorldMatrix();
	}
	mtx_world_ = mtxParent * mtx_local_;//mtxParent * m_mtxGlobal * m_mtxLocal;
}

void Bone::UpdateMatrices()
{
	UpdateWorldMatrix();
	for (auto c : children_)
	{
		c->UpdateMatrices();
	}
}

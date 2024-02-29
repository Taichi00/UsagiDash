#include "game/bone.h"

Bone::Bone(const std::string& name)
{
	name_ = name;
	rotation_ = XMQuaternionIdentity();
	mtx_inv_bind_ = XMMatrixIdentity();
	mtx_local_ = XMMatrixIdentity();
	mtx_world_ = XMMatrixIdentity();
	mtx_global_ = XMMatrixIdentity();
	children_.clear();
}

Bone::Bone(const Bone& bone)
{
	name_ = bone.name_;
	position_ = bone.position_;
	position_buf_ = bone.position_buf_;
	rotation_ = bone.rotation_;
	rotation_buf_ = bone.rotation_buf_;
	scale_ = bone.scale_;
	scale_buf_ = bone.scale_buf_;
	mtx_local_ = bone.mtx_local_;
	mtx_world_ = bone.mtx_world_;
	mtx_inv_bind_ = bone.mtx_inv_bind_;
	mtx_global_ = bone.mtx_global_;
	parent_ = bone.parent_;
}

void Bone::SetParent(Bone* parent)
{
	if (parent)
	{
		parent->AddChild(this);
	}
}

void Bone::AddChild(Bone* child)
{
	if (child)
	{
		child->parent_ = this;
		children_.push_back(child);
	}
}

void Bone::SaveBuffer()
{
	position_buf_ = position_;
	rotation_buf_ = rotation_;
	scale_buf_ = scale_;
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
	auto mtx_parent = XMMatrixIdentity();
	if (parent_)
	{
		mtx_parent = parent_->mtx_world_;
	}
	mtx_world_ = mtx_parent * mtx_local_;
}

void Bone::UpdateMatrices()
{
	UpdateWorldMatrix();
	for (auto c : children_)
	{
		c->UpdateMatrices();
	}
}

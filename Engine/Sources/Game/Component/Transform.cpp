#include "Transform.h"
#include "Entity.h"
#include "Scene.h"
#include "Camera.h"

bool Transform::Init()
{
	position = Vec3::Zero();
	scale = Vec3(1, 1, 1);
	rotation = Quaternion::identity();
	return true;
}

XMMATRIX Transform::GetWorldMatrix()
{
	auto world = XMMatrixIdentity();
	world *= XMMatrixScalingFromVector(scale);
	world *= XMMatrixRotationQuaternion(rotation);
	world *= XMMatrixTranslationFromVector(position);

	MulParentWorld(world);

	return world;
}

XMMATRIX Transform::GetBillboardWorldMatrix()
{
	auto view = GetEntity()->GetScene()->GetMainCamera()->GetViewMatrix();
	auto viewRot = XMQuaternionRotationMatrix(view);

	auto world = XMMatrixIdentity();
	world *= XMMatrixScalingFromVector(scale);
	world *= XMMatrixRotationQuaternion(rotation);
	world *= XMMatrixRotationQuaternion(XMQuaternionInverse(viewRot));
	world *= XMMatrixTranslationFromVector(position);

	MulParentWorld(world);

	return world;
}

Quaternion Transform::GetWorldRotation()
{
	auto rot = rotation;

	Entity* parent = GetEntity()->GetParent();
	while (parent != nullptr)
	{
		rot = rot * parent->transform->rotation;

		parent = parent->GetParent();
	}

	return rot;
}

void Transform::MulParentWorld(XMMATRIX& world)
{
	Entity* parent = GetEntity()->GetParent();
	while (parent != nullptr)
	{
		world *= XMMatrixScalingFromVector(parent->transform->scale);
		world *= XMMatrixRotationQuaternion(parent->transform->rotation);
		world *= XMMatrixTranslationFromVector(parent->transform->position);

		parent = parent->GetParent();
	}
}

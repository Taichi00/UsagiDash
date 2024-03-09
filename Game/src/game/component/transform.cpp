#include "game/component/transform.h"
#include "game/entity.h"
#include "game/scene.h"
#include "game/component/camera.h"

Transform::Transform()
{
	position = Vec3::Zero();
	scale = Vec3(1, 1, 1);
	rotation = Quaternion::Identity();

	world_matrix_ = XMMatrixIdentity();
}

bool Transform::Init()
{
	return true;
}

void Transform::TransformUpdate(const float delta_time)
{
	// ワールド変換行列を更新
	world_matrix_ = XMMatrixIdentity();
	world_matrix_ *= XMMatrixScalingFromVector(scale);
	world_matrix_ *= XMMatrixRotationQuaternion(rotation);
	world_matrix_ *= XMMatrixTranslationFromVector(position);

	world_matrix_ *= GetEntity()->Parent()->transform->world_matrix_;

	world_position_ = world_matrix_.r[3];
}

XMMATRIX Transform::BillboardWorldMatrix()
{
	auto view = GetEntity()->GetScene()->GetMainCamera()->GetViewMatrix();
	auto viewRot = XMQuaternionRotationMatrix(view);

	auto world = XMMatrixIdentity();
	world *= XMMatrixScalingFromVector(scale);
	world *= XMMatrixRotationQuaternion(rotation);
	world *= XMMatrixRotationQuaternion(XMQuaternionInverse(viewRot));
	world *= XMMatrixTranslationFromVector(position);

	world *= GetEntity()->Parent()->transform->world_matrix_;

	return world;
}

Quaternion Transform::WorldRotation()
{
	auto rot = rotation;

	Entity* parent = GetEntity()->Parent();
	while (parent != nullptr)
	{
		rot = rot * parent->transform->rotation;

		parent = parent->Parent();
	}

	return rot;
}

void Transform::MulParentWorld(XMMATRIX& world)
{
	Entity* parent = GetEntity()->Parent();
	while (parent)
	{
		world *= XMMatrixScalingFromVector(parent->transform->scale);
		world *= XMMatrixRotationQuaternion(parent->transform->rotation);
		world *= XMMatrixTranslationFromVector(parent->transform->position);

		parent = parent->Parent();
	}
}


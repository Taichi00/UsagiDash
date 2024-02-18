#include "game/component/component.h"
#include "game/entity.h"
#include "game/scene.h"

Component::Component()
{
	
}

Component::~Component()
{
	printf("Delete Component\n");
}

bool Component::Init()
{
	return true;
}

void Component::BeforeCameraUpdate(const float delta_time)
{
}

void Component::CameraUpdate(const float delta_time)
{
}

void Component::Update(const float delta_time)
{
}

void Component::PhysicsUpdate(const float delta_time)
{
}

void Component::OnCollisionEnter(Collider* collider)
{
}

void Component::Draw()
{
}

void Component::DrawAlpha()
{
}

void Component::DrawShadow()
{
}

void Component::DrawDepth()
{
}

void Component::DrawGBuffer()
{
}

void Component::DrawOutline()
{
}

void Component::Draw2D()
{
}

void Component::RegisterEntity(Entity* entity)
{
	entity_ = entity;
	transform = entity_->transform;
}

Entity* Component::GetEntity()
{
	return entity_;
}

Scene* Component::GetScene()
{
	return entity_->GetScene();
}


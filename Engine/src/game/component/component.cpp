#include "game/component/component.h"
#include "game/entity.h"

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

void Component::BeforeCameraUpdate()
{
}

void Component::CameraUpdate()
{
}

void Component::Update()
{
}

void Component::PhysicsUpdate()
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

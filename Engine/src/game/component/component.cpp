#include "game/component/component.h"
#include "game/entity.h"
#include "game/scene.h"

Component::Component()
{
	
}

Component::~Component()
{
}

bool Component::Init()
{
	return true;
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


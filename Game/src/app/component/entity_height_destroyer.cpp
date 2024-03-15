#include "entity_height_destroyer.h"
#include "game/component/transform.h"
#include "game/entity.h"

EntityHeightDestroyer::EntityHeightDestroyer(const float height)
{
	height_ = height;
}

void EntityHeightDestroyer::Update(const float delta_time)
{
	// �w��̍��������������폜����
	if (transform->WorldPosition().y < height_)
	{
		GetEntity()->Destroy();
	}
}

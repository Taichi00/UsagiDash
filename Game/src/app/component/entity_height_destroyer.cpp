#include "entity_height_destroyer.h"
#include "game/component/transform.h"
#include "game/entity.h"

EntityHeightDestroyer::EntityHeightDestroyer(const float height)
{
	height_ = height;
}

void EntityHeightDestroyer::Update(const float delta_time)
{
	// Žw’è‚Ì‚‚³‚ð‰º‰ñ‚Á‚½‚çíœ‚·‚é
	if (transform->WorldPosition().y < height_)
	{
		GetEntity()->Destroy();
	}
}

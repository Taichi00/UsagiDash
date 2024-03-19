#include "entity_height_destroyer.h"
#include "game/component/transform.h"
#include "game/entity.h"
#include "game/component/mesh_renderer.h"
#include "math/easing.h"

EntityHeightDestroyer::EntityHeightDestroyer(const float height, const float destroy_time)
{
	height_ = height;
	destroy_time_ = destroy_time;
}

bool EntityHeightDestroyer::Init()
{
	mesh_renderer_ = GetEntity()->GetComponent<MeshRenderer>();

	return true;
}

void EntityHeightDestroyer::Update(const float delta_time)
{
	if (destroy_timer_ > 0)
	{
		auto t = destroy_timer_ / destroy_time_;

		// ‚¾‚ñ‚¾‚ñ“§–¾‚É‚È‚Á‚Ä‚¢‚­
		mesh_renderer_->SetDitherLevel(Easing::Linear(1 - t));

		destroy_timer_ -= 60.0f * delta_time;

		if (destroy_timer_ <= 0)
		{
			// íœ
			GetEntity()->Destroy();
		}
	}
	else
	{
		// Žw’è‚Ì‚‚³‚ð‰º‰ñ‚Á‚½‚çíœŠJŽn
		if (transform->WorldPosition().y < height_)
		{
			if (mesh_renderer_)
			{
				destroy_timer_ = destroy_time_;
			}
			else
			{
				// mesh renderer ‚ª–³‚¯‚ê‚Î‚·‚®‚Éíœ
				GetEntity()->Destroy();
			}
		}
	}
}

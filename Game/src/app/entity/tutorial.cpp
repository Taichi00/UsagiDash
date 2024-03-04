#include "tutorial.h"
#include "app/component/tutorial_text_display.h"
#include "game/component/collider/sphere_collider.h"
#include <memory>

Tutorial::Tutorial(const std::string& text, const float radius) : Entity("", "tutorial", "event")
{
	auto collider = (Collider*)AddComponent(new SphereCollider(radius));
	AddComponent(new TutorialTextDisplay(text, 0, collider));
}

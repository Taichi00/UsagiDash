#include "tutorial.h"
#include "app/component/tutorial_text_controller.h"
#include "game/component/collider/sphere_collider.h"
#include <memory>

Tutorial::Tutorial(
	const std::string& text, 
	const std::string& task, 
	const float radius)
	: Entity("", "tutorial", "event")
{
	AddComponent(new SphereCollider(radius));
	AddComponent(new TutorialTextController(text, task));
}

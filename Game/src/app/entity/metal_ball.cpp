#include "metal_ball.h"
#include "game/resource/model.h"
#include "game/component/mesh_renderer.h"
#include "game/component/collider/sphere_collider.h"
#include "game/component/rigidbody.h"
#include "app/component/pause_behavior.h"
#include "app/component/entity_height_destroyer.h"
#include "game/game.h"

MetalBall::MetalBall(const float radius) : Entity("metal_ball", "object", "object")
{
	auto game = Game::Get();

	AddComponent(new MeshRenderer(game->LoadResource<Model>(L"assets/model/object/metal_ball.gltf")));
	AddComponent(new SphereCollider(radius));
	AddComponent(new Rigidbody(10, true, false, 0.0f));
	AddComponent(new EntityHeightDestroyer(-40));
	AddComponent(new PauseBehavior());

	transform->scale = Vec3(radius, radius, radius);
}

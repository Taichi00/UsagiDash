#include "jump_rift.h"
#include "app/component/pause_behavior.h"
#include "app/component/jump_rift_controller.h"
#include "game/component/mesh_renderer.h"
#include "game/component/rigidbody.h"
#include "game/component/collider/mesh_collider.h"
#include "game/game.h"

JumpRift::JumpRift(const Vec3& direction, const float distance) : Entity("jump_rift", "object", "object")
{
	auto game = Game::Get();

	auto mesh_renderer = AddComponent<MeshRenderer>(new MeshRenderer(game->LoadResource<Model>(L"assets/model/object/jump_rift.gltf")));
	AddComponent(new MeshCollider(game->LoadResource<CollisionModel>(L"assets/model/object/jump_rift.gltf")));
	AddComponent(new Rigidbody(1000, false, true, 0.1f));
	AddComponent(new JumpRiftController(direction, distance));
	AddComponent(new PauseBehavior());

	mesh_renderer->SetOutlineWidth(0);
}

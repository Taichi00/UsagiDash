#include "jump_lift.h"
#include "app/component/pause_behavior.h"
#include "app/component/jump_lift_controller.h"
#include "game/component/mesh_renderer.h"
#include "game/component/rigidbody.h"
#include "game/component/collider/mesh_collider.h"
#include "game/component/audio/audio_source.h"
#include "game/game.h"

JumpLift::JumpLift(const Vec3& direction, const float distance) : Entity("jump_lift", "object", "object")
{
	auto game = Game::Get();

	auto mesh_renderer = AddComponent<MeshRenderer>(game->LoadResource<Model>(L"assets/model/object/jump_lift.gltf"));
	AddComponent(new MeshCollider(game->LoadResource<CollisionModel>(L"assets/model/object/jump_lift.gltf")));
	AddComponent(new Rigidbody(1000, false, true, 0.1f));

	auto audio_move = AddComponent<AudioSource>(game->LoadResource<Audio>(L"assets/se/lift_move.wav"), 80.0f);
	auto audio_stop = AddComponent<AudioSource>(game->LoadResource<Audio>(L"assets/se/lift_stop.wav"), 80.0f);

	AddComponent(new JumpLiftController(direction, distance, audio_move, audio_stop));
	AddComponent(new PauseBehavior());

	mesh_renderer->SetOutlineWidth(0);
}

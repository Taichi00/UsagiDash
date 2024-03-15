#include "coin.h"
#include "game/game.h"
#include "game/component/mesh_renderer.h"
#include "game/component/collider/sphere_collider.h"
#include "game/resource/model.h"
#include "game/resource/audio.h"
#include "app/component/coin_script.h"
#include "game/component/audio/audio_source.h"
#include "app/component/pause_behavior.h"

Coin::Coin(const std::string& name) : Entity(name, "coin", "item")
{
	auto game = Game::Get();

	auto model = game->LoadResource<Model>(L"assets/model/item/Coin.gltf");
	auto se = game->LoadResource<Audio>(L"assets/se/MI_SFX 06.wav");

	AddComponent(new MeshRenderer(model));
	AddComponent(new SphereCollider(1.5f));
	AddComponent(new AudioSource(se));
	AddComponent(new CoinScript());
	AddComponent(new PauseBehavior());
}

Coin::~Coin()
{
}

#include "coin.h"
#include "game/game.h"
#include "game/component/mesh_renderer.h"
#include "game/component/collider/sphere_collider.h"
#include "game/resource/model.h"
#include "app/component/coin_script.h"

Coin::Coin(const std::string& name) : Entity(name, "coin")
{
	auto game = Game::Get();

	auto model = game->LoadResource<Model>("Assets/PlatformerPack/Coin.gltf");

	AddComponent(new MeshRenderer(model));
	AddComponent(new SphereCollider(1.5));
	AddComponent(new CoinScript());
}

Coin::~Coin()
{
}

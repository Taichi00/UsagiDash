#include "audio_listener.h"
#include "game/component/transform.h"
#include "game/game.h"
#include "game/audio_engine.h"

AudioListener::AudioListener()
{
	auto engine = Game::Get()->GetAudioEngine();

	engine->RegisterListener(&listener_);
}

AudioListener::~AudioListener()
{
	auto engine = Game::Get()->GetAudioEngine();

	engine->RemoveListener();
}

void AudioListener::Update(const float delta_time)
{
	// 座標、回転、速度を更新
	auto front = transform->rotation * Vec3(0, 0, 1);
	auto top = transform->rotation * Vec3(0, 1, 0);
	auto position = transform->WorldPosition();
	auto velocity = position - prev_position_;

	listener_.OrientFront = { front.x, front.y, front.z };
	listener_.OrientTop = { top.x, top.y, top.z };
	listener_.Position = { position.x, position.y, position.z };
	listener_.Velocity = { velocity.x, velocity.y, velocity.z };
	
	prev_position_ = position;
}

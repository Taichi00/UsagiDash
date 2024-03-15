#include "audio_source.h"
#include "game/audio_engine.h"
#include "game/game.h"
#include "game/resource/audio.h"
#include "game/component/transform.h"

AudioSource::AudioSource(const std::shared_ptr<Audio>& audio)
{
	engine_ = Game::Get()->GetAudioEngine();
	
	audio_ = audio;

	// source voice を作成
	engine_->CreateSourceVoice(*audio_->Data(), &source_voice_);
}

AudioSource::AudioSource(const std::shared_ptr<Audio>& audio, const float radius) : AudioSource(audio)
{
	is_3d_ = true;

	// emitter の設定
	emitter_.ChannelCount = 1;
	emitter_.DopplerScaler = 0.0f;
	emitter_.CurveDistanceScaler = radius;
}

AudioSource::~AudioSource()
{
	auto engine = Game::Get()->GetAudioEngine();
	
	engine->DestroySourceVoice(source_voice_);
}

void AudioSource::Update(const float delta_time)
{
	if (is_playing_)
	{
		// source voice の状態を取得
		XAUDIO2_VOICE_STATE state;
		source_voice_->GetState(&state);

		// 再生中の場合
		if (state.BuffersQueued > 0)
		{
			if (is_3d_)
				UpdateEmitterState();
		}
		else
		{
			// 再生終了
			is_playing_ = false;
		}
	}
}

void AudioSource::Play(const float volume, const bool loop)
{
	if (is_playing_)
	{
		engine_->Stop(source_voice_);
	}

	is_playing_ = true;

	if (is_3d_)
		UpdateEmitterState();

	engine_->PlayWaveSound(*audio_->Data(), source_voice_, volume, loop);
}

void AudioSource::Playing(const float volume, const bool loop)
{
	if (is_playing_)
		return;

	Play(volume, loop);
}

void AudioSource::Stop()
{
	engine_->Stop(source_voice_);
}

void AudioSource::SetVolume(const float volume)
{
	engine_->SetVolume(source_voice_, volume);
}

void AudioSource::SetPitch(const float pitch)
{
	engine_->SetPitch(source_voice_, pitch);
}

void AudioSource::UpdateEmitterState()
{
	// 座標、回転、速度を更新
	auto front = transform->rotation * Vec3(0, 0, 1);
	auto top = transform->rotation * Vec3(0, 1, 0);
	auto position = transform->WorldPosition();
	auto velocity = position - prev_position_;

	emitter_.OrientFront = { front.x, front.y, front.z };
	emitter_.OrientTop = { top.x, top.y, top.z };
	emitter_.Position = { position.x, position.y, position.z };
	emitter_.Velocity = { velocity.x, velocity.y, velocity.z };

	prev_position_ = position;

	engine_->Update(source_voice_, &emitter_);
}

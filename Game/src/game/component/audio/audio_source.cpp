#include "audio_source.h"
#include "game/audio_engine.h"
#include "game/game.h"
#include "game/resource/audio.h"
#include "game/component/transform.h"

AudioSource::AudioSource()
{
	engine_ = Game::Get()->GetAudioEngine();
}

AudioSource::AudioSource(
	std::shared_ptr<Audio> audio,
	const float volume,
	const float pitch,
	const float radius
) : AudioSource()
{
	Load(audio, volume, pitch, radius);
}

AudioSource::~AudioSource()
{
	auto engine = Game::Get()->GetAudioEngine();
	
	engine->DestroySourceVoice(source_voice_);
}

void AudioSource::Update(const float delta_time)
{
	if (!source_voice_)
		return;

	if (is_playing_)
	{
		// source voice �̏�Ԃ��擾
		XAUDIO2_VOICE_STATE state;
		source_voice_->GetState(&state);

		// �Đ����̏ꍇ
		if (state.BuffersQueued > 0)
		{
			if (is_3d_)
				UpdateEmitterState();
		}
		else
		{
			// �Đ��I��
			is_playing_ = false;
		}
	}
}

void AudioSource::Load(
	std::shared_ptr<Audio> audio,
	const float volume,
	const float pitch,
	const float radius
)
{
	audio_ = audio;
	volume_ = volume;
	pitch_ = pitch;

	if (radius > 0)
	{
		is_3d_ = true;
		// emitter �̐ݒ�
		emitter_.ChannelCount = 1;
		emitter_.DopplerScaler = 0.0f;
		emitter_.CurveDistanceScaler = radius;
	}

	// source voice ���쐬
	engine_->CreateSourceVoice(*audio_->Data(), &source_voice_);
	engine_->SetVolume(source_voice_, volume_);
	engine_->SetPitch(source_voice_, pitch_);
}

void AudioSource::Play(const bool loop)
{
	if (!source_voice_)
		return;

	if (is_playing_)
	{
		engine_->Stop(source_voice_);
	}

	is_playing_ = true;

	if (is_3d_)
		UpdateEmitterState();

	engine_->PlayWaveSound(*audio_->Data(), source_voice_, loop);
}

void AudioSource::Playing(const bool loop)
{
	if (is_playing_)
		return;

	Play(loop);
}

void AudioSource::Stop()
{
	engine_->Stop(source_voice_);
}

void AudioSource::SetVolumePercentage(const float p)
{
	if (!source_voice_)
		return;

	volume_percentage_ = p;
	engine_->SetVolume(source_voice_, volume_ * p);
}

void AudioSource::SetPitchPercentage(const float p)
{
	if (!source_voice_)
		return;

	pitch_percentage_ = p;
	engine_->SetPitch(source_voice_, pitch_ * p);
}

void AudioSource::SetVolume(const float volume)
{
	volume_ = volume;

	if (!source_voice_)
		return;

	engine_->SetVolume(source_voice_, volume_);
}

void AudioSource::SetPitch(const float pitch)
{
	pitch_ = pitch;

	if (!source_voice_)
		return;

	engine_->SetPitch(source_voice_, pitch_);
}

void AudioSource::UpdateEmitterState()
{
	// ���W�A��]�A���x���X�V
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

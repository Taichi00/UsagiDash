#include "audio.h"
#include "game/game.h"

Audio::~Audio()
{
	delete[] wave_data_.buffer;
}

std::unique_ptr<Audio> Audio::Load(const std::wstring& path)
{
	auto audio = Game::Get()->GetAudioEngine();

	auto audio_source = std::make_unique<Audio>();
	audio->LoadWaveFile(path, &audio_source->wave_data_);

	return std::move(audio_source);
}

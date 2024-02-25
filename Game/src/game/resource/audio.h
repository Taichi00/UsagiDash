#pragma once

#include "resource.h"
#include "game/audio_engine.h"
#include <memory>
#include <string>

class Audio : public Resource
{
public:
	Audio() {};
	~Audio();

	static std::unique_ptr<Audio> Load(const std::wstring& path);

	AudioEngine::WaveData* Data() { return &wave_data_; }

private:
	AudioEngine::WaveData wave_data_{};
};
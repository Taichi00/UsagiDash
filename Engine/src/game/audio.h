#pragma once

#pragma comment(lib, "xaudio2.lib")
#include <xaudio2.h>

#pragma comment(lib, "winmm.lib")
#include <mmsystem.h>

#include "engine/comptr.h"
#include <string>

class Audio
{
public:
	struct WaveData
	{
		WAVEFORMATEX wav_format;
		char* buffer;
		DWORD size;
	};

	Audio();
	~Audio();

	void LoadWaveFile(const std::wstring& file_path, WaveData* out_data);

	void PlayWaveSound(const WaveData& wave_data, bool loop);

private:
	void Init();
	void Cleanup();

private:
	IXAudio2* xaudio_;
	IXAudio2MasteringVoice* mastering_voice_;
	IXAudio2SourceVoice* source_voice_;
};
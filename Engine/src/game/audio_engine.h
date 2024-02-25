#pragma once

#pragma comment(lib, "xaudio2.lib")
#include <xaudio2.h>
#include <x3daudio.h>

#pragma comment(lib, "winmm.lib")
#include <mmsystem.h>

#include "engine/comptr.h"
#include <string>
#include <vector>

class AudioEngine
{
public:
	struct WaveData
	{
		WAVEFORMATEX wav_format;
		char* buffer;
		DWORD size;
	};

	AudioEngine();
	~AudioEngine();

	void LoadWaveFile(const std::wstring& file_path, WaveData* out_data);

	void CreateSourceVoice(const WaveData& wave_data, IXAudio2SourceVoice** source_voice);
	void DestroySourceVoice(IXAudio2SourceVoice* source_voice);

	void PlayWaveSound(
		const WaveData& wave_data, IXAudio2SourceVoice* source_voice, 
		float volume, bool loop);

	void SetVolume(IXAudio2SourceVoice* source_voice, float volume);
	void SetPitch(IXAudio2SourceVoice* source_voice, float pitch);

	void Update(IXAudio2SourceVoice* source_voice, X3DAUDIO_EMITTER* emitter);

	void RegisterListener(X3DAUDIO_LISTENER* listener);
	void RemoveListener();

private:
	void Init();
	void Cleanup();

private:
	IXAudio2* xaudio_;
	IXAudio2MasteringVoice* mastering_voice_;
	IXAudio2SubmixVoice* submix_voice_;

	X3DAUDIO_HANDLE x3d_instance_;

	// x3daudio ‚ÌŒvŽZŒ‹‰Ê‚ðŠi”[
	X3DAUDIO_DSP_SETTINGS dsp_settings_ = {};

	X3DAUDIO_LISTENER* listener_ = nullptr;
	X3DAUDIO_CONE listener_directional_cone_ = {};
};
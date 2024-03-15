#include "audio_engine.h"
#include <iostream>

AudioEngine::AudioEngine()
{
    try
    {
        Init();
    }
    catch (const char* e)
    {
        std::cout << "Failed initialize XAudio2: " << e << std::endl;
    }
}

AudioEngine::~AudioEngine()
{
    Cleanup();
}

void AudioEngine::LoadWaveFile(const std::wstring& file_path, WaveData* out_data)
{
    if (out_data)
    {
        free(out_data->buffer);
    }
    else
    {
        throw "WaveData is nullptr.";
    }

    HMMIO mmio = nullptr;
    MMCKINFO riff_chunk{};
    MMCKINFO chunk{};
    MMRESULT mret;
    
    // wavファイルを開く
    mmio = mmioOpen((LPWSTR)file_path.data(), nullptr, MMIO_READ);
    if (!mmio)
    {
        throw "mmioOpen";
    }

    // RIFF chunk へ進入
    riff_chunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    mret = mmioDescend(mmio, &riff_chunk, nullptr, MMIO_FINDRIFF);
    if (mret != MMSYSERR_NOERROR)
    {
        mmioClose(mmio, MMIO_FHOPEN);
        throw "mmioDescend(wave) ret=" + mret;
    }

    // fmt chunk へ進入
    chunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    mret = mmioDescend(mmio, &chunk, &riff_chunk, MMIO_FINDCHUNK);
    if (mret != MMSYSERR_NOERROR)
    {
        mmioClose(mmio, MMIO_FHOPEN);
        throw "mmioDescend(fmt) ret=" + mret;
    }

    // fmt chunk の読み込み
    DWORD size = mmioRead(mmio, (HPSTR)&out_data->wav_format, chunk.cksize);
    if (size != chunk.cksize)
    {
        mmioClose(mmio, MMIO_FHOPEN);
        throw "mmioRead(fmt) ret=" + mret;
    }

    // フォーマットチェック
    if (out_data->wav_format.wFormatTag != WAVE_FORMAT_PCM)
    {
        mmioClose(mmio, MMIO_FHOPEN);
        throw "format error";
    }

    // fmt chunk を退出
    mret = mmioAscend(mmio, &chunk, 0);
    if (mret != MMSYSERR_NOERROR)
    {
        mmioClose(mmio, MMIO_FHOPEN);
        throw "mmioAscend(fmt) ret=" + mret;
    }

    // data chunk へ進入
    chunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    mret = mmioDescend(mmio, &chunk, &riff_chunk, MMIO_FINDCHUNK);
    if (mret != MMSYSERR_NOERROR)
    {
        mmioClose(mmio, MMIO_FHOPEN);
        throw "mmioDescend(data) ret=" + mret;
    }

    // data chunk の読み込み
    out_data->size = chunk.cksize;
    out_data->buffer = new char[chunk.cksize];
    size = mmioRead(mmio, (HPSTR)out_data->buffer, chunk.cksize);
    if (size != chunk.cksize)
    {
        mmioClose(mmio, MMIO_FHOPEN);
        delete[] out_data->buffer;
        throw "mmioRead(data) ret=" + mret;
    }

    // wavファイルを閉じる
    mmioClose(mmio, MMIO_FHOPEN);
}

void AudioEngine::CreateSourceVoice(const WaveData& wave_data, IXAudio2SourceVoice** source_voice)
{
    // 波形フォーマットの設定
    WAVEFORMATEX wave_format = wave_data.wav_format;

    // 1サンプル当たりのバッファサイズを算出
    wave_format.wBitsPerSample = wave_data.wav_format.nBlockAlign * 8 / wave_data.wav_format.nChannels;

    // submix voice への参照リスト
    XAUDIO2_SEND_DESCRIPTOR send = { 0, submix_voice_ };
    XAUDIO2_VOICE_SENDS send_list = { 1, &send };

    // source voice の作成
    if (FAILED(xaudio_->CreateSourceVoice(
        source_voice, 
        (WAVEFORMATEX*)&wave_format,
        0,
        XAUDIO2_DEFAULT_FREQ_RATIO,
        nullptr,
        &send_list,
        nullptr
        )))
    {
        throw "CreateSourceVoice";
    }
}

void AudioEngine::DestroySourceVoice(IXAudio2SourceVoice* source_voice)
{
    if (!source_voice)
        return;

    source_voice->DestroyVoice();
    source_voice = nullptr;
}

void AudioEngine::PlayWaveSound(const WaveData& wave_data, IXAudio2SourceVoice* source_voice, float volume, bool loop)
{
    // 波形データを source voice に渡す
    XAUDIO2_BUFFER buffer{};
    buffer.pAudioData = (BYTE*)wave_data.buffer;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = wave_data.size;
    buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

    source_voice->SubmitSourceBuffer(&buffer);
    
    // 音を鳴らす
    source_voice->SetVolume(volume);
    source_voice->Start();
}

void AudioEngine::Stop(IXAudio2SourceVoice* source_voice)
{
    source_voice->Stop();
    source_voice->FlushSourceBuffers();
}

void AudioEngine::SetVolume(IXAudio2SourceVoice* source_voice, float volume)
{
    source_voice->SetVolume(volume);
}

void AudioEngine::SetPitch(IXAudio2SourceVoice* source_voice, float pitch)
{
    source_voice->SetFrequencyRatio(pitch);
}

void AudioEngine::Update(IXAudio2SourceVoice* source_voice, X3DAUDIO_EMITTER* emitter)
{
    if (!listener_)
        return;

    X3DAudioCalculate(
        x3d_instance_,
        listener_, emitter,
        X3DAUDIO_CALCULATE_MATRIX |
        X3DAUDIO_CALCULATE_DOPPLER |
        X3DAUDIO_CALCULATE_LPF_DIRECT |
        X3DAUDIO_CALCULATE_REVERB,
        &dsp_settings_
    );
    
    for (auto i = 0u; i < dsp_settings_.DstChannelCount; i++)
    {
        dsp_settings_.pMatrixCoefficients[i] *= dsp_settings_.ReverbLevel;
    }

    source_voice->SetOutputMatrix(submix_voice_, 1, dsp_settings_.DstChannelCount, dsp_settings_.pMatrixCoefficients);
    
    source_voice->SetFrequencyRatio(dsp_settings_.DopplerFactor);
    
    XAUDIO2_FILTER_PARAMETERS filter_params = {
        LowPassFilter,
        2.0f * sinf(X3DAUDIO_PI / 6.0f * dsp_settings_.LPFDirectCoefficient),
        1.0f
    };
    source_voice->SetFilterParameters(&filter_params);
}

void AudioEngine::RegisterListener(X3DAUDIO_LISTENER* listener)
{
    listener_ = listener;
    listener_->pCone = (X3DAUDIO_CONE*)&listener_directional_cone_;
}

void AudioEngine::RemoveListener()
{
    listener_ = nullptr;
}

void AudioEngine::Init()
{
    // XAudio2 の初期化
    UINT32 flags = 0;
#ifdef _DEBUG
    flags |= XAUDIO2_DEBUG_ENGINE;
#endif

    if (FAILED(XAudio2Create(&xaudio_, flags)))
    {
        throw "XAudio2Create";
    }

    // mastering voice の作成
    if (FAILED(xaudio_->CreateMasteringVoice(&mastering_voice_)))
    {
        throw "CreateMasteringVoice";
    }

    XAUDIO2_VOICE_DETAILS voice_details;
    mastering_voice_->GetVoiceDetails(&voice_details);

    // submix voice の作成
    if (FAILED(xaudio_->CreateSubmixVoice(&submix_voice_, 2, voice_details.InputSampleRate, 0, 0, 0, 0)))
    {
        throw "CreateSubmixVoice";
    }

    // X3DAudio の初期化
    DWORD channel_mask;
    mastering_voice_->GetChannelMask(&channel_mask);

    if (FAILED(X3DAudioInitialize(channel_mask, X3DAUDIO_SPEED_OF_SOUND, x3d_instance_)))
    {
        throw "X3DAudioInitialize";
    }
    
    FLOAT32* matrix = new FLOAT32[voice_details.InputChannels];
    dsp_settings_.SrcChannelCount = 1;
    dsp_settings_.DstChannelCount = voice_details.InputChannels;
    dsp_settings_.pMatrixCoefficients = matrix;

    listener_directional_cone_ = { X3DAUDIO_PI * 5.0f / 6.0f, X3DAUDIO_PI * 1.0f / 6.0f, 1.0f, 0.75f, 0.0f, 0.25f, 0.708f, 1.0f };
}

void AudioEngine::Cleanup()
{
    delete[] dsp_settings_.pMatrixCoefficients;

    // submix voice の破棄
    if (submix_voice_)
    {
        submix_voice_->DestroyVoice();
        submix_voice_ = nullptr;
    }

    // mastering voice の破棄
    if (mastering_voice_)
    {
        mastering_voice_->DestroyVoice();
        mastering_voice_ = nullptr;
    }

    // xaudio の破棄
    if (xaudio_)
    {
        xaudio_->Release();
        xaudio_ = nullptr;
    }

}

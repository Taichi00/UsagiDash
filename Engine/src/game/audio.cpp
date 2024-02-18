#include "audio.h"
#include <iostream>

Audio::Audio()
{
    try
    {
        Init();
    }
    catch (const char* e)
    {
        std::cout << "Failed initialize XAudio2: " << e << std::endl;
    }

    /*try
    {
        WaveData wave_data{};
        LoadWaveFile(L"Assets/bgm/y014.wav", &wave_data);
        PlayWaveSound(wave_data, true);
    }
    catch (const char* e)
    {
        std::cout << "Failed test: " << e << std::endl;
    }*/
}

Audio::~Audio()
{
    Cleanup();
}

void Audio::LoadWaveFile(const std::wstring& file_path, WaveData* out_data)
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
    
    // wav�t�@�C�����J��
    mmio = mmioOpen((LPWSTR)file_path.data(), nullptr, MMIO_READ);
    if (!mmio)
    {
        throw "mmioOpen";
    }

    // RIFF chunk �֐i��
    riff_chunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
    mret = mmioDescend(mmio, &riff_chunk, nullptr, MMIO_FINDRIFF);
    if (mret != MMSYSERR_NOERROR)
    {
        mmioClose(mmio, MMIO_FHOPEN);
        throw "mmioDescend(wave) ret=" + mret;
    }

    // fmt chunk �֐i��
    chunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
    mret = mmioDescend(mmio, &chunk, &riff_chunk, MMIO_FINDCHUNK);
    if (mret != MMSYSERR_NOERROR)
    {
        mmioClose(mmio, MMIO_FHOPEN);
        throw "mmioDescend(fmt) ret=" + mret;
    }

    // fmt chunk �̓ǂݍ���
    DWORD size = mmioRead(mmio, (HPSTR)&out_data->wav_format, chunk.cksize);
    if (size != chunk.cksize)
    {
        mmioClose(mmio, MMIO_FHOPEN);
        throw "mmioRead(fmt) ret=" + mret;
    }

    // �t�H�[�}�b�g�`�F�b�N
    if (out_data->wav_format.wFormatTag != WAVE_FORMAT_PCM)
    {
        mmioClose(mmio, MMIO_FHOPEN);
        throw "format error";
    }

    // fmt chunk ��ޏo
    mret = mmioAscend(mmio, &chunk, 0);
    if (mret != MMSYSERR_NOERROR)
    {
        mmioClose(mmio, MMIO_FHOPEN);
        throw "mmioAscend(fmt) ret=" + mret;
    }

    // data chunk �֐i��
    chunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
    mret = mmioDescend(mmio, &chunk, &riff_chunk, MMIO_FINDCHUNK);
    if (mret != MMSYSERR_NOERROR)
    {
        mmioClose(mmio, MMIO_FHOPEN);
        throw "mmioDescend(data) ret=" + mret;
    }

    // data chunk �̓ǂݍ���
    out_data->size = chunk.cksize;
    out_data->buffer = new char[chunk.cksize];
    size = mmioRead(mmio, (HPSTR)out_data->buffer, chunk.cksize);
    if (size != chunk.cksize)
    {
        mmioClose(mmio, MMIO_FHOPEN);
        delete[] out_data->buffer;
        throw "mmioRead(data) ret=" + mret;
    }

    // wav�t�@�C�������
    mmioClose(mmio, MMIO_FHOPEN);
}

void Audio::PlayWaveSound(const WaveData& wave_data, bool loop)
{
    WAVEFORMATEX wave_format{};

    // �g�`�t�H�[�}�b�g�̐ݒ�
    memcpy(&wave_format, &wave_data.wav_format, sizeof(wave_data.wav_format));

    // 1�T���v��������̃o�b�t�@�T�C�Y���Z�o
    wave_format.wBitsPerSample = wave_data.wav_format.nBlockAlign * 8 / wave_data.wav_format.nChannels;

    // source voice �̍쐬
    if (FAILED(xaudio_->CreateSourceVoice(&source_voice_, (WAVEFORMATEX*)&wave_format)))
    {
        throw "CreateSourceVoice";
    }

    // �g�`�f�[�^�� source voice �ɓn��
    XAUDIO2_BUFFER buffer{};
    buffer.pAudioData = (BYTE*)wave_data.buffer;
    buffer.Flags = XAUDIO2_END_OF_STREAM;
    buffer.AudioBytes = wave_data.size;
    buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

    source_voice_->SubmitSourceBuffer(&buffer);

    // ����炷
    source_voice_->Start();
}

void Audio::Init()
{
    // COM�R���|�[�l���g�̏�����
    if (FAILED(CoInitializeEx(0, COINIT_MULTITHREADED)))
    {
        throw "CoInitializeEx";
    }

    // XAudio2 �̏�����
    UINT32 flags = 0;
#ifdef _DEBUG
    flags |= XAUDIO2_DEBUG_ENGINE;
#endif

    if (FAILED(XAudio2Create(&xaudio_, flags)))
    {
        throw "XAudio2Create";
    }

    // mastering voice �̍쐬
    if (FAILED(xaudio_->CreateMasteringVoice(&mastering_voice_)))
    {
        throw "CreateMasteringVoice";
    }
}

void Audio::Cleanup()
{
    // source voice �̔j��
    if (source_voice_)
    {
        source_voice_->DestroyVoice();
        source_voice_ = nullptr;
    }

    // mastering voice �̔j��
    if (mastering_voice_)
    {
        mastering_voice_->DestroyVoice();
        mastering_voice_ = nullptr;
    }

    // xaudio �̔j��
    if (xaudio_)
    {
        xaudio_->Release();
        xaudio_ = nullptr;
    }

    CoUninitialize();
}

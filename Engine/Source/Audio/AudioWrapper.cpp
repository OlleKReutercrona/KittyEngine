#include "stdafx.h"
#include "AudioWrapper.h"
#include "XAudio2Helpers.h"
#include "Utility/Logging.h"
#include "Utility/StringUtils.h"
//#include "ComponentSystem/Components/AudioComponent.h"
#include "Engine/Source/Math/KittyMath.h"
#include <cassert>

KE::XAudio2VoiceCallbackInterface::XAudio2VoiceCallbackInterface() : hBufferEndEvent(CreateEvent(nullptr, false, false, nullptr)), myHasFinishedPlayback(false) {}

KE::XAudio2VoiceCallbackInterface::~XAudio2VoiceCallbackInterface() { CloseHandle(hBufferEndEvent); }

void KE::XAudio2VoiceCallbackInterface::OnStreamEnd() noexcept
{
	SetEvent(hBufferEndEvent);
}

void KE::XAudio2VoiceCallbackInterface::OnBufferEnd(void* pBufferContext) noexcept
{
	pBufferContext;
	myHasFinishedPlayback = true;
}

void KE::XAudio2VoiceCallbackInterface::OnBufferStart(void* pBufferContext) noexcept
{
	pBufferContext;
	myHasFinishedPlayback = false;
}



KE::AudioWrapper::AudioWrapper() : mySounds{}, mySoundsStackPtr(0), myIXAudioHandle(nullptr), myMasteringVoiceHandle(nullptr), myMasteringVolume(1.0f), myX3DInstance{}
{
	// Initialize COM.
	HRESULT hr;
	hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		KE_ERROR("Failed to initialize COM.");
		return;
	}

	UINT32 creationFlags;

#ifdef _DEBUG
	creationFlags = XAUDIO2_DEBUG_ENGINE;
#else
	creationFlags = 0;
#endif	

	// Create an instance of the XAudio2 engine.
	if (FAILED(hr = XAudio2Create(&myIXAudioHandle, creationFlags, XAUDIO2_USE_DEFAULT_PROCESSOR)))
	{
		KE_ERROR("Failed to initialize XAudio2-engine.");
		return;
	}

#ifdef _DEBUG

	XAUDIO2_DEBUG_CONFIGURATION debugSettings{};

	debugSettings.LogFileline = true;
	debugSettings.LogThreadID = true;
	debugSettings.LogFunctionName = true;
	debugSettings.LogTiming = true;

	myIXAudioHandle->SetDebugConfiguration(&debugSettings);

#endif

	// Create mastering voice, which in essence encapsulates an audio device.
	if (FAILED(hr = myIXAudioHandle->CreateMasteringVoice(&myMasteringVoiceHandle)))
	{
		KE_ERROR("Failed to initialize audio device.");
		return;
	}

	// Initialize X3DAudio.
	DWORD dwChannelMask;
	myMasteringVoiceHandle->GetChannelMask(&dwChannelMask);

	if (FAILED(hr = X3DAudioInitialize(dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, myX3DInstance)))
	{
		KE_ERROR("Failed to initialize X3DAudio.");
		return;
	}
}

KE::AudioWrapper::~AudioWrapper()
{
	myIXAudioHandle->Release();

	for (unsigned short i = 0; i < mySoundsStackPtr; i++)
	{
		delete[] mySounds[i].myBuffer.pAudioData;
	}
}

bool KE::AudioWrapper::AddSoundFromDisk(const char* aFilePath, bool aSoundShouldLoop, const char* aFileName)
{
	if (!(mySoundsStackPtr < KE_AudioWrapper_MaxSounds))
	{
		KE_ERROR("The audiowrapper's sound stack is full, you cannot add any more sounds. Bump up the hash-define if you want more sounds, but don't get too greedy as each sound eats a lot of memory.");
		return false;
	}

	// CODE COPIED FROM https://learn.microsoft.com/en-us/windows/win32/xaudio2/how-to--load-audio-data-files-in-xaudio2.
	// 1. Declare WAVEFORMATEXTENSIBLE and XAUDIO2_BUFFER structures.
	// Done in constructor, these are members.

	// 2. Open the audio file with CreateFile.
	HANDLE hFile = CreateFileA(
		aFilePath,
		GENERIC_READ,
		FILE_SHARE_READ,
		nullptr,
		OPEN_EXISTING,
		0,
		nullptr
	);

	if (INVALID_HANDLE_VALUE == hFile)
	{
		KE_ERROR("Failed to open file, error code: %u.", GetLastError());
		return false;
	}

	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, nullptr, FILE_BEGIN))
	{
		KE_ERROR("File shenanigans I don't understand, error code: %u.", GetLastError());
		return false;
	}

	// 3. Locate the 'RIFF' chunk in the audio file, and check the file type.
	DWORD dwChunkSize;
	DWORD dwChunkPosition;
	// Check the file type, should be fourccWAVE or 'XWMA'.
	// FourccWave is the standardized version, XWMA is a proprietary Windows variant intended for XAudio2.
	FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);

	DWORD fileType;
	ReadChunkData(hFile, &fileType, sizeof(DWORD), dwChunkPosition);
	if (fileType != fourccWAVE)
	{
		KE_ERROR("File type of file %s is not .wav. We only support wav files as of yet.", aFilePath);
		return false;
	}

	// 4. Locate the 'fmt' chunk, and copy its contents into a WAVEFORMATEXTENSIBLE structure.
	FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
	ReadChunkData(hFile, &mySounds[mySoundsStackPtr].myWFX, dwChunkSize, dwChunkPosition);

	// 5. Locate the 'data' chunk, and read its contents into a buffer.
	// Fill out the audio data buffer with the contents of the fourccDATA chunk.
	FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
	BYTE* pDataBuffer = new BYTE[dwChunkSize];
	ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

	// 6. Populate an XAUDIO2_BUFFER structure.
	mySounds[mySoundsStackPtr].myBuffer.AudioBytes = dwChunkSize; // Size of audio buffer in bytes.
	mySounds[mySoundsStackPtr].myBuffer.pAudioData = pDataBuffer; // We're transferring ownership of resources here. Make sure to delete this in the destructor.
	mySounds[mySoundsStackPtr].myBuffer.Flags = XAUDIO2_END_OF_STREAM; // Tell the source voice not to expect any data after this buffer.

	if (aSoundShouldLoop)
	{
		mySounds[mySoundsStackPtr].myBuffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}

	// Assign name to audio file.
	memcpy(mySounds[mySoundsStackPtr].myName, aFileName, strlen(aFileName));

	// Increase the sound stack pointer to the next free slot.
	mySoundsStackPtr++;

	return true;
}

KE::AudioFile* KE::AudioWrapper::DoesSoundExist(const char* aFileName)
{
	if (!(strlen(aFileName) < KE_AudioFile_FileNameLen))
	{
		KE_WARNING("The file %s cannot be a valid audio file -- it is too long(max %u characters).", aFileName, KE_AudioFile_FileNameLen);
		return nullptr;
	}

	for (unsigned short i = 0; i < KE_AudioWrapper_MaxSounds; i++)
	{
		if (!strcmp(aFileName, mySounds[i].myName))
		{
			return (mySounds + i);
		}
	}

	return nullptr;
}

bool KE::AudioWrapper::PlaySoundFile(IXAudio2SourceVoice* aSourceVoiceHandle, AudioFile* anAudioFile)
{
	// Steps to play a sound: https://learn.microsoft.com/en-us/windows/win32/xaudio2/how-to--play-a-sound-with-xaudio2
	HRESULT hr;

	//float targetVolume = aVolume;
	//aSourceVoiceHandle->GetVolume(&targetVolume);
	myMasteringVoiceHandle->SetVolume(myMasteringVolume);

	// Try and send the source voice an audio buffer which we want it to play.
	if (FAILED(hr = aSourceVoiceHandle->SubmitSourceBuffer(&anAudioFile->myBuffer)))
	{
		KE_ERROR("Failed to submit sound buffer to source voice. HRESULT = %i", hr);
		return false;
	}

	// Finally, we start the sound playback.
	if (FAILED(hr = aSourceVoiceHandle->Start(0, XAUDIO2_COMMIT_NOW)))
	{
		KE_ERROR("Failed to play sound. HRESULT = %i", hr);
		return false;
	}

	return true;
}

bool KE::AudioWrapper::ApplyAcoustics(IXAudio2SourceVoice* aSourceVoiceHandle, X3DAUDIO_VECTOR* emitterTransformArr, X3DAUDIO_VECTOR* listenerTransformArr)
{
	X3DAUDIO_LISTENER listener = {};
	X3DAUDIO_EMITTER emitter = {};
	constexpr unsigned short numChannels = 2u;
	static FLOAT32 channelAzimuths[numChannels] = { -90.0f * KE::DegToRadImmediate, 90.0f * KE::DegToRadImmediate };

	listener.OrientFront = listenerTransformArr[0];
	listener.OrientTop = listenerTransformArr[1];
	listener.Position = listenerTransformArr[2];
	listener.Velocity = listenerTransformArr[3];

	emitter.ChannelCount = numChannels;
	emitter.ChannelRadius = 0.5f;
	emitter.pChannelAzimuths = channelAzimuths;
	emitter.CurveDistanceScaler = 1.0f;
	emitter.DopplerScaler = 1.0f;
	emitter.OrientFront = emitterTransformArr[0];
	emitter.OrientTop = emitterTransformArr[1];
	emitter.Position = emitterTransformArr[2];
	emitter.Velocity = emitterTransformArr[3];


	X3DAUDIO_DSP_SETTINGS DSPSettings;
	XAUDIO2_VOICE_DETAILS voiceDetails;

	aSourceVoiceHandle->GetVoiceDetails(&voiceDetails);
	
	// This guy leads to strange errors when written out to by X3DAudioCalculate if it is too small, even though a size of 2 should suffice.
	// Don't size this guy down plz.
	FLOAT32 matrix[numChannels * 8];

	DSPSettings.SrcChannelCount = numChannels;
	DSPSettings.DstChannelCount = voiceDetails.InputChannels;
	DSPSettings.pMatrixCoefficients = matrix;

	X3DAudioCalculate(myX3DInstance, &listener, &emitter,
		X3DAUDIO_CALCULATE_MATRIX | X3DAUDIO_CALCULATE_DOPPLER | X3DAUDIO_CALCULATE_LPF_DIRECT | X3DAUDIO_CALCULATE_REVERB,
		&DSPSettings
	);

	aSourceVoiceHandle->SetOutputMatrix(myMasteringVoiceHandle, 1, voiceDetails.InputChannels, DSPSettings.pMatrixCoefficients);
	aSourceVoiceHandle->SetFrequencyRatio(DSPSettings.DopplerFactor);

	// Apply a low pass filter direct coefficient.
	XAUDIO2_FILTER_PARAMETERS filterParameters = { XAUDIO2_FILTER_TYPE::LowPassFilter, 2.0f * sinf(X3DAUDIO_PI / 6.0f * DSPSettings.LPFDirectCoefficient), 1.0f };
	aSourceVoiceHandle->SetFilterParameters(&filterParameters);

	return true;
}


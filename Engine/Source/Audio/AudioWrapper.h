#pragma once
#include <Windows.h>
#include <xaudio2.h>
#include <x3daudio.h>
#include <stdio.h>

#include <unordered_map>
#include <string>

// Dependency managed in premake instead.
//#pragma comment(lib, "XAudio2")

/*
  This is an audio engine meant only to play wav sounds.
  We'll use XAudio2, which has it's sound threaded.
  https://learn.microsoft.com/en-us/windows/win32/xaudio2/xaudio2-apis-portal

  https://learn.microsoft.com/en-us/windows/uwp/gaming/working-with-audio-in-your-directx-game
  https://learn.microsoft.com/en-us/windows/win32/xaudio2/how-to--initialize-xaudio2
  https://learn.microsoft.com/en-us/windows/win32/xaudio2/how-to--play-a-sound-with-xaudio2

  We also use X3DAudio for spatial audio playback.
  https://learn.microsoft.com/en-us/windows/win32/xaudio2/x3daudio-overview
  https://learn.microsoft.com/en-us/windows/win32/xaudio2/how-to--integrate-x3daudio-with-xaudio2
*/

namespace KE
{
	class AudioComponent;

	// Callback interface which we use to check if a sound has stopped playing.
	// https://learn.microsoft.com/en-us/windows/win32/xaudio2/how-to--use-source-voice-callbacks
	class XAudio2VoiceCallbackInterface : public IXAudio2VoiceCallback
	{
	public:
		HANDLE hBufferEndEvent;
		bool myHasFinishedPlayback;

		XAudio2VoiceCallbackInterface();
		~XAudio2VoiceCallbackInterface();

		// Called when the voice has just finished playing a contiguous audio stream.
		virtual void OnStreamEnd() noexcept override;

		virtual void OnVoiceProcessingPassEnd() noexcept override final { /* NOT IMPLEMENTED YET */ }
		virtual void OnVoiceProcessingPassStart(UINT32 SamplesRequired) noexcept override final { SamplesRequired; /* NOT IMPLEMENTED YET */ }
		virtual void OnBufferEnd(void* pBufferContext) noexcept override final; /* IMPLEMENTED */
		virtual void OnBufferStart(void* pBufferContext) noexcept override final; /* NOT IMPLEMENTED YET */
		virtual void OnLoopEnd(void* pBufferContext) noexcept override final { pBufferContext; /* NOT IMPLEMENTED YET */ }
		virtual void OnVoiceError(void* pBufferContext, HRESULT Error) noexcept override final { pBufferContext; /* NOT IMPLEMENTED YET */ Error; }
	};

#define KE_AudioFile_FileNameLen (64u)
	struct AudioFile
	{
		WAVEFORMATEXTENSIBLE myWFX;
		XAUDIO2_BUFFER myBuffer;
		char myName[KE_AudioFile_FileNameLen];
	};

	class AudioWrapper
	{
#define KE_AudioWrapper_MaxSounds (64u)
#define KE_AudioWrapper_AudioFilePath "Data/Assets/Audio"
	public:

		// Initializes XAudio2.
		AudioWrapper();
		AudioWrapper(const AudioWrapper&) = delete;
		AudioWrapper(AudioWrapper&&) = delete;
		~AudioWrapper();

		// Allows you to load sounds from file into the sounds-array below. Having the sound cached is ideal, because we do not want to read from disk each time we want to play a sound.
		bool AddSoundFromDisk(const char* aFilePath, bool aSoundShouldLoop, const char* aFileName);

		// Checks if a sound exists within the stack. Returns a pointer to the sound if it exists, returns a nullptr if it doesn't.
		AudioFile* DoesSoundExist(const char* aFileName);

		// Plays a sound by setting up a source voice and then playing through it. Plays the sound globally.
		bool PlaySoundFile(IXAudio2SourceVoice* aSourceVoiceHandle, AudioFile* anAudioFile);

		// Starts treating a sound with 3D spatial effects like reverb and doppler effect.
		// Arrays expect: OrientFront, OrientTop, Position & Velocity.
		bool ApplyAcoustics(IXAudio2SourceVoice* aSourceVoiceHandle, X3DAUDIO_VECTOR* emitterTransformArr, X3DAUDIO_VECTOR* listenerTransformArr);

		inline IXAudio2* GetIXAudioHandle(void) const { return myIXAudioHandle; }

		// Set mastering volume, which is applied to all sounds through -> masteringVolume * individualSoundVolume. Clamped to 0 - 1.
		inline void SetMasteringVolume(float aMasteringVolume = 1.0f)
		{
			aMasteringVolume > 1.0f ? aMasteringVolume = 1.0f : __noop;
			aMasteringVolume < 0.0f ? aMasteringVolume = 0.0f : __noop;
			myMasteringVolume = aMasteringVolume;
			myMasteringVoiceHandle->SetVolume(myMasteringVolume);
		}

	private:


		AudioFile mySounds[KE_AudioWrapper_MaxSounds];

		// Relative pointer into the current free slot of our sounds stack.
		unsigned short mySoundsStackPtr;

		// Handle to the IXAudio2 engine.
		IXAudio2* myIXAudioHandle;

		// Handle to the mastering voice(audio device).
		IXAudio2MasteringVoice* myMasteringVoiceHandle;

		// Mastering volume to easily change the volume of all sounds.
		float myMasteringVolume;

		// Handle to the X3DAudio engine, which we use to play sounds at different locations.
		X3DAUDIO_HANDLE myX3DInstance;
	};
}
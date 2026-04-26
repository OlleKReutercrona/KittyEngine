#pragma once
#include "Engine/Source/Audio/AudioWrapper.h"

#ifndef AudioCompMaxConcurrentSounds
#define AudioCompMaxConcurrentSounds (16u)
#endif

namespace KE
{
	struct Sound
	{
		void PlayAudioFile(void);
		void Stop(void);

		struct VoicePair : IXAudio2VoiceCallback
		{
			inline VoicePair() : mySourceVoice(nullptr), mySubmixVoice(nullptr), myKillSignal(false) {}
			VoicePair(Sound* aParent);

			inline operator bool() const { return mySourceVoice; /* return mySourceVoice && mySubmixVoice; */ }
			inline void Clear() { mySourceVoice = nullptr; mySubmixVoice = nullptr; /* memset(this, 0, sizeof(*this)); */ }

			IXAudio2SourceVoice* mySourceVoice;
			IXAudio2SubmixVoice* mySubmixVoice;
			bool myKillSignal;
			bool myIsPlaying = false;

#pragma region IXAudio2VoiceCallback_Overrides
			virtual void OnStreamEnd() noexcept override { myIsPlaying = false; }
			virtual void OnVoiceProcessingPassEnd() noexcept override final {  }
			virtual void OnVoiceProcessingPassStart(UINT32 SamplesRequired) noexcept override final { SamplesRequired; }
			virtual void OnBufferEnd(void* pBufferContext) noexcept override final;
			virtual void OnBufferStart(void* pBufferContext) noexcept override final { myIsPlaying = true; pBufferContext; }
			virtual void OnLoopEnd(void* pBufferContext) noexcept override final { pBufferContext; }
			virtual void OnVoiceError(void* pBufferContext, HRESULT Error) noexcept override final { pBufferContext; Error; }
#pragma endregion

		} myVoicePairs[AudioCompMaxConcurrentSounds];

		const short GetAnyFreeVoicePairSlot(void) const;

		std::string mySoundFileName;
		AudioFile* myAudioFile;
		bool myShouldLoop;
		float myVolume;
	};



	class AudioPlayer
	{
	public:

		
		void Init(void);
		void Update(void);

		inline Sound* operator[](const char* aSoundName)
		{
			return &mySounds[aSoundName];
		}


	private:

		std::unordered_map<std::string, Sound> mySounds;

	};
}



/*
{
	audioMan["music.wav"]->Play();

	...

	audioMan["music.wav"]->Stop();
}
*/
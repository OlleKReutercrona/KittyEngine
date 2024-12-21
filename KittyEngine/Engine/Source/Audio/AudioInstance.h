#pragma once
#include "Engine/Source/Audio/AudioWrapper.h"
#include <string>

namespace KE
{
	struct AudioInstanceData
	{
		std::string fileName;
		bool shouldLoop;
		bool shouldPerformSpatialPlayback = false;
		float volume = 1.0f;
		float maxVolume = 1.0f;
	};

#define AudioCompMaxConcurrentSounds (16u)

	class AudioInstance/* : public Component*/
	{
		friend class GlobalAudio;
	public:

		AudioInstance();
		~AudioInstance();

		// Debug-only instantiation of a freestanding component. Strictly used for testing purposes.
	    /*AudioInstance(void* dummy);*/

		void SetData(AudioInstanceData& aData);


		///// Update Loop Handling /////

		//// Wake up and smell the ashes.
		//virtual void Awake() override;

		//// Nothing much to do yet.
		//virtual void LateUpdate() override;

		// Update transforms if we're supporting 3D-playback.
		void Update();


		/////// Activity Handling /////

		//// Play my sound. 
		//virtual void OnEnable() override;

		//// Stop playing my sound.
		//virtual void OnDisable() override;

		//// Audiowrapper manages the resources on it's own, so nothing to do here.
		//virtual void OnDestroy() override;

		bool IsPlaying() const { return myVoicePairs[0].myIsPlaying; }

		
		void PlayAudioFile(float aVolScalar = 1.0f);
		void Stop(void);

		// Volume is clamped between 0 and 1. Sets the volume for this sound only.
		void SetVolume(float aVolume = 1.0f);



	private:

		struct VoicePair : IXAudio2VoiceCallback
		{
			inline VoicePair() : mySourceVoice(nullptr), mySubmixVoice(nullptr), myKillSignal(false) {}
			VoicePair(AudioInstance* aParent);

			inline operator bool() const { return mySourceVoice; /* return mySourceVoice && mySubmixVoice; */ }
			inline void Clear() { mySourceVoice = nullptr; mySubmixVoice = nullptr; /* memset(this, 0, sizeof(*this)); */ }

			IXAudio2SourceVoice* mySourceVoice;
			IXAudio2SubmixVoice* mySubmixVoice;
			bool myKillSignal;
			bool myIsPlaying = false;

			virtual void OnStreamEnd() noexcept override { myIsPlaying = false; }
			virtual void OnVoiceProcessingPassEnd() noexcept override final {  }
			virtual void OnVoiceProcessingPassStart(UINT32 SamplesRequired) noexcept override final { SamplesRequired; }
			virtual void OnBufferEnd(void* pBufferContext) noexcept override final;
			virtual void OnBufferStart(void* pBufferContext) noexcept override final { myIsPlaying = true; pBufferContext; }
			virtual void OnLoopEnd(void* pBufferContext) noexcept override final { pBufferContext; }
			virtual void OnVoiceError(void* pBufferContext, HRESULT Error) noexcept override final { pBufferContext; Error; }

		} myVoicePairs[AudioCompMaxConcurrentSounds];

		const short GetAnyFreeVoicePairSlot(void) const;


		std::string mySoundFileName;
		AudioFile* myAudioFile;
		bool myShouldLoop;
		bool myShouldPerformSpatialPlayback;
		float myVolume;

		// Used to calculate velocity.
		X3DAUDIO_VECTOR myEmitterLastFramePos;
		X3DAUDIO_VECTOR myListenerLastFramePos;
	};


}


/*


Play:
Get Free slot -> Play through slot.
if (all slots are full)
	Log warning


Stopped playing:
Mark slot as free.

*/
#include "stdafx.h"
#include "AudioInstance.h"
#include "Engine/Source/Utility/Logging.h"
#include "Engine/Source/Utility/StringUtils.h"
#include "Engine/Source/Utility/Global.h"
#include <cassert>
#include "imgui/imgui.h"
#include "ComponentSystem/Components/Graphics/CameraComponent.h"
#include "Engine\Source\ComponentSystem\GameObjectManager.h"

//extern KE::AudioWrapper g_AudioWrapper;


KE::AudioInstance::AudioInstance()
	: myVoicePairs{}
	, myAudioFile(nullptr)
	, myShouldLoop(false)
	, myShouldPerformSpatialPlayback(false)
	, myVolume(1.0f)
	, myEmitterLastFramePos(0.0f, 0.0f, 0.0f)
	, myListenerLastFramePos(0.0f, 0.0f, 0.0f)
{

}

KE::AudioInstance::~AudioInstance()
{
	Stop();
}


void KE::AudioInstance::SetData(AudioInstanceData& aData)
{
	// Default volume of all sounds to 1.0f.
	mySoundFileName = aData.fileName;
	myShouldLoop = aData.shouldLoop;
	myShouldPerformSpatialPlayback = aData.shouldPerformSpatialPlayback;
	myVolume = aData.volume;

	myAudioFile = KE_GLOBAL::audioWrapper.DoesSoundExist(mySoundFileName.c_str());
	if (myAudioFile == nullptr)
	{
		// Sound file doesn't exist. Add it and reassign the pointer.
		char filePath[MAX_PATH] = {};

#pragma warning(push)
#pragma warning(disable: 4996)
		(void)strcat(filePath, KE_AudioWrapper_AudioFilePath);
		(void)strcat(filePath, "/");
		(void)strcat(filePath, mySoundFileName.c_str());
#pragma warning(pop)

		KE_GLOBAL::audioWrapper.AddSoundFromDisk(filePath, myShouldLoop, mySoundFileName.c_str());
		myAudioFile = KE_GLOBAL::audioWrapper.DoesSoundExist(mySoundFileName.c_str());
	}

	
	//HRESULT hr;
	//IXAudio2* IXAudioHandle = KE_GLOBAL::audioWrapper.GetIXAudioHandle();

	//if (FAILED(hr = IXAudioHandle->CreateSourceVoice(
	//	&mySourceVoice,
	//	(WAVEFORMATEX*)&myAudioFile->myWFX,
	//	0,
	//	XAUDIO2_DEFAULT_FREQ_RATIO
	//)))
	//{
	//	KE_ERROR("Failed to create a source voice to play sound %s.", mySoundFileName.c_str());
	//	return;
	//}


	//// Hook in our 3Dfx submix voice, if there should be one.
	//if (myShouldPerformSpatialPlayback)
	//{
	//	if (FAILED(hr = IXAudioHandle->CreateSubmixVoice(&mySubmixVoice, 1, 44100)))
	//	{
	//		KE_ERROR("Failed to create a submix voice, HRESULT = %i", hr);
	//		return;
	//	}

	//	XAUDIO2_SEND_DESCRIPTOR SFXSend = { 0, mySubmixVoice };
	//	XAUDIO2_VOICE_SENDS SFXSendList = { 1, &SFXSend };
	//	mySourceVoice->SetOutputVoices(&SFXSendList);
	//}
}

void KE::AudioInstance::Update()
{
	for (unsigned short i = 0; i < AudioCompMaxConcurrentSounds; i++)
	{
		if(myVoicePairs[i].mySourceVoice) {
			float volumes[2] = { myVolume, myVolume };
			myVoicePairs[i].mySourceVoice->SetVolume(myVolume);
			myVoicePairs[i].mySourceVoice->SetChannelVolumes(2, volumes);
		}
		//if (myVoicePairs[i].mySubmixVoice) {
		//	myVoicePairs[i].mySubmixVoice->SetVolume(myVolume);
		//}

		if (myVoicePairs[i].myKillSignal)
		{
			myVoicePairs[i].mySourceVoice->DestroyVoice();
			myVoicePairs[i].myKillSignal = false;

			myVoicePairs[i].Clear();
		}
	}
	
	if (!myShouldPerformSpatialPlayback)
	{
		return;
	}

	// Run applyacoustics on all valid voice pairs here.
	//for (unsigned short i = 0; i < AudioCompMaxConcurrentSounds; i++)
	//{
	//	if (myVoicePairs[i])
	//	{
	//		KE_GLOBAL::audioWrapper.ApplyAcoustics(myVoicePairs[i].mySourceVoice, emitterTransform, listenerTransform);
	//	}
	//}
}



#pragma region Activity_Handling
void KE::AudioInstance::PlayAudioFile(float aVolScalar)
{
	if (myAudioFile == nullptr)
	{
		KE_ERROR("Failed to play sound %s.", mySoundFileName.c_str());
		return;
	}
	
	short nextInLine = GetAnyFreeVoicePairSlot();
	if (nextInLine == -1)
	{
		KE_WARNING("This audio component has reached the concurrency limit of %i, no more instances can play at the same time.", AudioCompMaxConcurrentSounds);
		return;
	}
	
	// Create a new voice at the given slot.
	VoicePair* newVoicePair = new (&myVoicePairs[nextInLine]) VoicePair(this);

	// Set volume.
	const float channelVolumes[2] = { myVolume * aVolScalar, myVolume * aVolScalar };

	//newVoicePair->mySourceVoice->SetVolume(myVolume);
	newVoicePair->mySourceVoice->SetChannelVolumes(2, channelVolumes);
	//newVoicePair->mySubmixVoice->SetVolume(myVolume);

	KE_GLOBAL::audioWrapper.PlaySoundFile(newVoicePair->mySourceVoice, myAudioFile);
}

void KE::AudioInstance::Stop(void)
{
	for (unsigned short i = 0; i < AudioCompMaxConcurrentSounds; i++)
	{
		if (myVoicePairs[i])
		{
			myVoicePairs[i].mySourceVoice->Stop();
			myVoicePairs[i].myKillSignal = true;
			myVoicePairs[i].myIsPlaying = false;
		}
	}
}

void KE::AudioInstance::SetVolume(float aVolume)
{
	// Clamp the volume.
	myVolume = std::clamp(aVolume, 0.0f, 1.0f);
}

#pragma endregion // Activity_Handling

KE::AudioInstance::VoicePair::VoicePair(AudioInstance* aParent) : mySourceVoice(nullptr), mySubmixVoice(nullptr), myKillSignal(false)
{
	HRESULT hr;

	IXAudio2* IXAudioHandle = KE_GLOBAL::audioWrapper.GetIXAudioHandle();

	if (FAILED(hr = IXAudioHandle->CreateSourceVoice(
		&mySourceVoice,
		(WAVEFORMATEX*)&aParent->myAudioFile->myWFX,
		0,
		XAUDIO2_DEFAULT_FREQ_RATIO,
		this
	)))
	{
		KE_ERROR("Failed to create a source voice to play sound %s.", aParent->mySoundFileName.c_str());
		return;
	}


	// Hook in our 3Dfx submix voice, if there should be one.
	if (aParent->myShouldPerformSpatialPlayback)
	{
		if (FAILED(hr = IXAudioHandle->CreateSubmixVoice(&mySubmixVoice, 1, 44100)))
		{
			KE_ERROR("Failed to create a submix voice, HRESULT = %i", hr);
			return;
		}

		XAUDIO2_SEND_DESCRIPTOR SFXSend = { 0, mySubmixVoice };
		XAUDIO2_VOICE_SENDS SFXSendList = { 1, &SFXSend };
		mySourceVoice->SetOutputVoices(&SFXSendList);
	}
}

#pragma region VOICE_PAIR_CALLBACKS

void KE::AudioInstance::VoicePair::OnBufferEnd(void* pBufferContext) noexcept
{
	// Extremely important to remove ourselves from the XAudio graph, but we may not do it from within a callback.
	//mySourceVoice->DestroyVoice();
	myKillSignal = true;
	myIsPlaying = false;
	//Clear();
}

#pragma endregion

const short KE::AudioInstance::GetAnyFreeVoicePairSlot(void) const
{
	for (unsigned short i = 0; i < AudioCompMaxConcurrentSounds; i++)
	{
		if (!myVoicePairs[i])
		{
			return i;
		}
	}

	return -1;
}
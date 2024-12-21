#include "stdafx.h"
#include "AudioPlayer.h"
#include <filesystem>

void KE::Sound::PlayAudioFile(void)
{
	if (myAudioFile == nullptr)
	{
		KE_ERROR("Failed to play sound %s.", mySoundFileName.c_str());
		return;
	}

	short nextInLine = GetAnyFreeVoicePairSlot();
	if (nextInLine == -1)
	{
		KE_WARNING("This sound has reached the concurrency limit of %i, no more instances can play at the same time.", AudioCompMaxConcurrentSounds);
		return;
	}

	// Create a new voice at the given slot.
	VoicePair* newVoicePair = new (myVoicePairs + nextInLine) VoicePair(this);

	// Set volume.
	const float channelVolumes[2] = { myVolume, myVolume };

	//newVoicePair->mySourceVoice->SetVolume(myVolume);
	newVoicePair->mySourceVoice->SetChannelVolumes(2, channelVolumes);


	KE_GLOBAL::audioWrapper.PlaySoundFile(newVoicePair->mySourceVoice, myAudioFile);
}

void KE::Sound::Stop(void)
{
	for (unsigned short i = 0; i < AudioCompMaxConcurrentSounds; i++)
	{
		if (myVoicePairs[i])
		{
			myVoicePairs[i].mySourceVoice->Stop();
			myVoicePairs[i].myKillSignal = true;
		}
	}
}

KE::Sound::VoicePair::VoicePair(Sound* aParent) : mySourceVoice(nullptr), mySubmixVoice(nullptr), myKillSignal(false)
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
}

void KE::Sound::VoicePair::OnBufferEnd(void* pBufferContext) noexcept
{
	// Extremely important to remove ourselves from the XAudio graph, but we may not do it from within a callback.
	myKillSignal = true;
	myIsPlaying = false;
}

const short KE::Sound::GetAnyFreeVoicePairSlot(void) const
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




void KE::AudioPlayer::Init(void)
{
	namespace fs = std::filesystem;

	// Start file crawler and add all sounds in the sounds folder.
	const fs::path audioFPath(KE_AudioWrapper_AudioFilePath);

	if (!fs::exists(audioFPath))
	{
		KE_ERROR("You are missing the audio folder on disk. Make sure to copy over your assets from Unity!");
		return;
	}

	for (const auto& dirEntry : fs::directory_iterator(audioFPath))
	{
		std::string fName(dirEntry.path().filename().string());

		std::cout << "Loading audio file: " << dirEntry.path() << "\nFilename: " << fName << std::endl;

		Sound newSound;

		newSound.myShouldLoop = false;
		newSound.mySoundFileName = fName;
		newSound.myVolume = 1.0f;

		newSound.myAudioFile = KE_GLOBAL::audioWrapper.DoesSoundExist(newSound.mySoundFileName.c_str());

		if (newSound.myAudioFile == nullptr)
		{
			KE_GLOBAL::audioWrapper.AddSoundFromDisk(dirEntry.path().string().c_str(), newSound.myShouldLoop, newSound.mySoundFileName.c_str());
			newSound.myAudioFile = KE_GLOBAL::audioWrapper.DoesSoundExist(newSound.mySoundFileName.c_str());
		}

		mySounds[fName] = newSound;
	}
}

void KE::AudioPlayer::Update(void)
{
	// Check for stale voices to kill.
	for (auto& pair : mySounds)
	{
		auto& sound = pair.second;
		
		for (unsigned short i = 0; i < AudioCompMaxConcurrentSounds; i++)
		{
			if (sound.myVoicePairs[i].myKillSignal)
			{
				sound.myVoicePairs[i].mySourceVoice->DestroyVoice();
				sound.myVoicePairs[i].myKillSignal = false;

				sound.myVoicePairs[i].Clear();
			}
		}
	}
}

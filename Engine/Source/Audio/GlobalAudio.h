#pragma once
#include "Engine/Source/Audio/AudioInstance.h"

namespace sound
{
	enum class SFX {

		BallParry,
		BallCollision,
		BallImpact,
		BallBounce,

		PortalTeleport,
		DeathzoneTick,

		PowerupShieldActivate,
		PowerupShieldDeActivate,
		PowerupPickup,
		PowerupTeleport,
		PowerupSplitBall,
		PowerupExplosiveBall,
		PowerupTelekinesis,

		PlayerThrow,
		PlayerDash,
		PlayerFalling,
		PlayerMeele,

		PlayerTauntCandy,	
		PlayerTauntDew,		
		PlayerTauntCinder,	
		PlayerTauntMal,		
		PlayerTauntGod,		

		PlayerDeathCandy,
		PlayerDeathDew,
		PlayerDeathCinder,
		PlayerDeathMal,
		PlayerDeathGod,

		PlayerSelectCandy,
		PlayerSelectDew,
		PlayerSelectCinder,
		PlayerSelectMal,
		PlayerSelectGod,

		AnnouncerGo,
		AnnouncerReady,
		AnnouncerWinner,
		AnnouncerChampion,
		Vignette,

		MenuToggle,
		MenuSelect,
		ScoreboardPoint,

		GodWin,
		GodLose,
		GodActive,

		Count
	};

	enum class Music {

		Space,
		Lilypad,
		SpoopyMansion,
		Menu,
		Winscreen,

		Count
	};

	enum class Ambient {

		Space,
		Lilypad,
		SpoopyMansion,

		Count
	};
}

namespace KE
{
	class GlobalAudio
	{
	public:
		static void Init();
		static void Update();

		static void PlaySFX(sound::SFX aType, float aVolScalar = 1.0f);
		static void PlayMusic(sound::Music aType);
		static void PlayAmbient(sound::Ambient aType);
		static bool IsSFXPlaying(sound::SFX aType);
		static void StopSFX(sound::SFX aType);

		static void LoadSoundSettings();
		static void SetMasterVolume(float aVolume);

		static const char* EnumParser(sound::SFX aType);
		static const char* EnumParser(sound::Music aType);
		static const char* EnumParser(sound::Ambient aType);


		inline static std::unordered_map<sound::Music, AudioInstance> musicInstances;
		inline static std::unordered_map<sound::SFX, AudioInstance> sfxInstances;
		inline static std::unordered_map<sound::Ambient, AudioInstance> ambientInstances;

		inline static std::array<AudioInstanceData, static_cast<int>(sound::SFX::Count)> sfxData = {};
		inline static std::array<AudioInstanceData, static_cast<int>(sound::Music::Count)> musicData = {};
		inline static std::array<AudioInstanceData, static_cast<int>(sound::Ambient::Count)> ambientData = {};

		inline static sound::Music currentMusic = sound::Music::Count;
		inline static sound::Music previousMusic = sound::Music::Count;
		inline static sound::Ambient currentAmbient = sound::Ambient::Count;
		inline static sound::Ambient previousAmbient = sound::Ambient::Count;

		inline static bool initialized = false;
		inline static float masterVolume = 1.0f;

		inline static float myTimer = 0.0f;
		inline static float myDuration = 2.0f;
		inline static bool myIncrease = true;

		inline static bool myStartFade = false;

		inline static float myMusicMaxVolume = 1.0f;
		inline static float myAmbientMaxVolume = 1.0f;
	};
}
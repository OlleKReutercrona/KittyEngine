#include "stdafx.h"
#include "GlobalAudio.h"
#include "Engine/Source/Utility/Global.h"
#include <nlohmann/json.hpp>

using namespace sound;

#define Volume(x) (x > 1.0f ? 1.0f : (x < 0.0f ? 0.0f : x))
#define MaxVolume(x) (x > 1.0f ? 1.0f : (x < 0.0f ? 0.0f : x))
constexpr const char* soundSettingsFile = "Data/Settings/SoundSettings.json";

// TODO
// - Lägg in vignetten, ska spelas när vi går från character select -> game samt game -> Select / winscreen.
// - Lägg in Winscreen_music, ska spelas när vi går från game -> winscreen.
// - Lägg in nytt Telekinesis ljud.


namespace KE
{
	using LOOPING = bool;

	void GlobalAudio::Init()
	{
		if (initialized) { return; }

		bool LOOP = true;
		bool NO_LOOP = false;
		bool NO_SPATIAL = false;
		
		// -------> BALL <------- //
		sfxData[(int)SFX::BallParry]				= { "SFX/Ball/Ball_parry.wav"					, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::BallBounce]				= { "SFX/Ball/Ball_wall_bounce.wav"				, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::BallImpact]				= { "SFX/Ball/Ball_impact1.wav"					, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::BallCollision]			= { "SFX/Ball/Ball_collision.wav"				, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };

		// -------> PORTAL <------- //
		sfxData[(int)SFX::PortalTeleport]			= { "SFX/Portal/portal_teleport.wav"			, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::DeathzoneTick]			= { "SFX/Warning_Sizzle.wav"					, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };

		// -------> POWER UP <------- //
		sfxData[(int)SFX::PowerupShieldActivate]	= { "SFX/Powerup/shield_activate1.wav"			, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PowerupShieldDeActivate]	= { "SFX/Powerup/shield_deactivate1.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PowerupTeleport]			= { "SFX/Powerup/ball_teleport.wav"				, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PowerupExplosiveBall]		= { "SFX/Powerup/explosive_ball.wav"			, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PowerupPickup]			= { "SFX/Powerup/Powerup_pickup.wav"			, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PowerupSplitBall]			= { "SFX/Powerup/Powerup_split_ball.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PowerupTelekinesis]		= { "SFX/Powerup/Powerup_telekenesis_bubble.wav", NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };

		// -------> PLAYER <------- //
		sfxData[(int)SFX::PlayerDash]				= { "SFX/Player/Dash_1.wav"						, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PlayerThrow]				= { "SFX/Player/Ball_throw2.wav"				, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PlayerFalling]			= { "SFX/Player/player_fall1.wav"				, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PlayerMeele]				= { "SFX/Player/player_Meele_swoosh2.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
																																	
		sfxData[(int)SFX::PlayerTauntCandy]			= { "SFX/Player/VoiceOver/Candy_Taunt.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PlayerTauntDew]			= { "SFX/Player/VoiceOver/Dew_Taunt.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PlayerTauntCinder]		= { "SFX/Player/VoiceOver/Cinder_Taunt.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PlayerTauntMal]			= { "SFX/Player/VoiceOver/Mal_Taunt.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PlayerTauntGod]			= { "SFX/Player/VoiceOver/God_Taunt.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };

		sfxData[(int)SFX::PlayerDeathCandy]			= { "SFX/Player/VoiceOver/Candy_Death.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PlayerDeathDew]			= { "SFX/Player/VoiceOver/Dew_Death.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PlayerDeathCinder]		= { "SFX/Player/VoiceOver/Cinder_Death.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PlayerDeathMal]			= { "SFX/Player/VoiceOver/Mal_Death.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PlayerDeathGod]			= { "SFX/Player/VoiceOver/God_Death.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };

		sfxData[(int)SFX::PlayerSelectCandy]		= { "SFX/Player/VoiceOver/Candy_Select.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PlayerSelectDew]			= { "SFX/Player/VoiceOver/Dew_Select.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PlayerSelectCinder]		= { "SFX/Player/VoiceOver/Cinder_Select.wav"	, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PlayerSelectMal]			= { "SFX/Player/VoiceOver/Mal_Select.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::PlayerSelectGod]			= { "SFX/Player/VoiceOver/God_Select_2.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };

		// -------> ANNOUNCER <------- //
		sfxData[(int)SFX::AnnouncerGo]				= { "SFX/Announcer/Announcer_Go.wav"						, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::AnnouncerReady]			= { "SFX/Announcer/Announcer_Ready.wav"						, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::AnnouncerWinner]			= { "SFX/Announcer/Announcer_Winner.wav"					, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::AnnouncerChampion]		= { "SFX/Announcer/Announcer_Intergalactic_Champion.wav"	, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		
		// -------> VIGNETTE <------- //
		sfxData[(int)SFX::Vignette]					= { "SFX/Transition/transitional_vignett.wav"	, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };

		// -------> MENU <------- //
		sfxData[(int)SFX::MenuToggle]				= { "SFX/Menu/Menu_movement.wav"			, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::MenuSelect]				= { "SFX/Menu/Menu_select.wav"				, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::ScoreboardPoint]			= { "SFX/Menu/Scoreboard_Blop.wav"			, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };

		// -------> MUSIC <------- //
		musicData[(int)Music::Space]				= { "Music/music_Arena_Space.wav"			, LOOP, NO_SPATIAL, Volume(0), MaxVolume(1) };
		musicData[(int)Music::Lilypad]				= { "Music/music_Arena_Lilypad.wav"			, LOOP, NO_SPATIAL, Volume(0), MaxVolume(1) };
		musicData[(int)Music::SpoopyMansion]		= { "Music/music_Arena_Spoopy_Mansion.wav"	, LOOP, NO_SPATIAL, Volume(0), MaxVolume(1) };
		musicData[(int)Music::Menu]					= { "Music/music_menu.wav"					, LOOP, NO_SPATIAL, Volume(0), MaxVolume(1) };
		musicData[(int)Music::Winscreen]			= { "Music/music_Winner_Announce.wav"		, LOOP, NO_SPATIAL, Volume(0), MaxVolume(1) };

		// -------> AMBIENT <------- //
		ambientData[(int)Ambient::Space]			= { "Ambient/Amb_Space.wav"					, LOOP, NO_SPATIAL, Volume(0), MaxVolume(1) };
		ambientData[(int)Ambient::Lilypad]			= { "Ambient/Amb_Lilypad_Forest.wav"		, LOOP, NO_SPATIAL, Volume(0), MaxVolume(1) };
		ambientData[(int)Ambient::SpoopyMansion]	= { "Ambient/Amb_spoopy_mansion.wav"		, LOOP, NO_SPATIAL, Volume(0), MaxVolume(1) };


		// Cheat
		sfxData[(int)SFX::GodWin] = { "SFX/Player/VoiceOver/God_Win.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::GodLose] = { "SFX/Player/VoiceOver/God_Lose.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };
		sfxData[(int)SFX::GodActive] = { "SFX/Player/VoiceOver/God_Active.wav"		, NO_LOOP, NO_SPATIAL, Volume(1), MaxVolume(1) };

		for (int i = 0; i < static_cast<int>(Music::Count); i++)
		{
			Music type = static_cast<Music>(i);
			musicInstances[type] = KE::AudioInstance();
			musicInstances[type].SetData(musicData[i]);
		}
		for (int i = 0; i < static_cast<int>(SFX::Count); i++)
		{
			SFX type = static_cast<SFX>(i);
			sfxInstances[type] = KE::AudioInstance();
			sfxInstances[type].SetData(sfxData[i]);
		}
		for (int i = 0; i < static_cast<int>(Ambient::Count); i++)
		{
			Ambient type = static_cast<Ambient>(i);
			ambientInstances[type] = KE::AudioInstance();
			ambientInstances[type].SetData(ambientData[i]);
		}

		LoadSoundSettings();

		initialized = true;
	}

	void GlobalAudio::Update()
	{
		for (int i = 0; i < static_cast<int>(SFX::Count); i++)
		{
			SFX type = static_cast<SFX>(i);
			float volume = sfxData[(int)type].volume;
			sfxInstances[type].SetVolume(volume);
			sfxInstances[type].Update();
		}

		for (int i = 0; i < static_cast<int>(Music::Count); i++)
		{
			Music type = static_cast<Music>(i);
			float volume = std::clamp(musicData[(int)type].volume, 0.0f, myMusicMaxVolume);

			musicInstances[type].SetVolume(volume);
			musicInstances[type].Update();
		}

		for (int i = 0; i < static_cast<int>(Ambient::Count); i++)
		{
			Ambient type = static_cast<Ambient>(i);
			ambientInstances[type].SetVolume(ambientData[(int)type].volume);
			ambientInstances[type].Update();
		}
		
		if (myStartFade) {
		
			myTimer += KE_GLOBAL::deltaTime;
			float volume = myTimer / myDuration;
			float percentage = myTimer / myDuration;

			float currentVolume = myMusicMaxVolume * percentage;
			float previousVolume = myMusicMaxVolume - currentVolume;

			if (currentMusic != Music::Count) {
				musicData[(int)currentMusic].volume = std::clamp(currentVolume, 0.0f, myMusicMaxVolume);
			}
			if (previousMusic != sound::Music::Count) {
				musicData[(int)previousMusic].volume = std::clamp(previousVolume, 0.0f, myMusicMaxVolume);
			}

			if (currentAmbient != Ambient::Count) {
				ambientData[(int)currentAmbient].volume = std::clamp(currentVolume, 0.0f, myAmbientMaxVolume);
			}
			if (previousAmbient != sound::Ambient::Count) {
				ambientData[(int)previousAmbient].volume = std::clamp(previousVolume, 0.0f, myAmbientMaxVolume);
			}

			if(volume >= 1.0f) {

				myStartFade = false;
				myTimer = 0.0f;
			}
		}
	}

	void GlobalAudio::SetMasterVolume(float aVolume)
	{
		KE_GLOBAL::audioWrapper.SetMasteringVolume(aVolume);
	}

	// <---[OBS]--->
	// aVolScalar wont work since every AudioInstance set their volume in their update.
	// This means they assign volumes to the whole AudioInstance and not voice pair.
	// If we play a ball bounce sound with lower volume it will be set for all ball bounces.
	// To solve this volume should not be set in AudioInstance but in the voice pair.
	// _____________________________________________________________________________________
	void GlobalAudio::PlaySFX(SFX aType, float /*aVolScalar*/)
	{
		if (sfxInstances.find(aType) == sfxInstances.end())
		{
			KE_WARNING("SFX not found");
			return;
		}

		sfxInstances[aType].SetVolume(sfxData[(int)aType].volume);
		sfxInstances[aType].PlayAudioFile();
	}

	void GlobalAudio::PlayMusic(Music aType)
	{
		if (musicInstances.find(aType) == musicInstances.end())
		{
			KE_WARNING("Music not found");
			return;
		}

		// First time we play music, start all of them.
		if (currentMusic == Music::Count) {
		
			for (int i = 0; i < static_cast<int>(Music::Count); i++)
			{
				sound::Music type = static_cast<sound::Music>(i);
				musicInstances[type].PlayAudioFile();
			}
		}

		if (aType != currentMusic) {

			myTimer = 0.0f;
			previousMusic = currentMusic;
			currentMusic = aType;
			myStartFade = true;
		}
	}

	void GlobalAudio::PlayAmbient(sound::Ambient aType)
	{
		if (ambientInstances.find(aType) == ambientInstances.end() && aType != Ambient::Count)
		{
			KE_WARNING("Ambient not found");
			return;
		}
		
		if (currentAmbient == Ambient::Count && previousAmbient == Ambient::Count) {

			for (int i = 0; i < static_cast<int>(Ambient::Count); i++)
			{
				sound::Ambient type = static_cast<sound::Ambient>(i);
				ambientInstances[type].PlayAudioFile();
			}
		}

		if (aType != currentAmbient) {

			myTimer = 0.0f;
			previousAmbient = currentAmbient;
			currentAmbient = aType;
			myStartFade = true;
		}
	}

	bool GlobalAudio::IsSFXPlaying(sound::SFX aType)
	{
		if (sfxInstances.find(aType) == sfxInstances.end())
		{
			KE_WARNING("SFX not found");
			return false;
		}

		return sfxInstances[aType].IsPlaying();
	}

	void GlobalAudio::StopSFX(sound::SFX aType)
	{
		if (sfxInstances.find(aType) == sfxInstances.end())
		{
			KE_WARNING("SFX not found");
			return;
		}

		sfxInstances[aType].Stop();
	}

	void GlobalAudio::LoadSoundSettings()
	{
		std::ifstream ifs(soundSettingsFile);

		if (!std::filesystem::exists(soundSettingsFile)) return;
		if (!ifs.good()) return;
		nlohmann::json obj = nlohmann::json::parse(ifs);
		ifs.close();

		if (!obj["master"].empty())
		{
			nlohmann::json& master = obj["master"];

			KE::GlobalAudio::myMusicMaxVolume = master["music"];
		}

		if (!obj["sfx"].empty())
		{
			nlohmann::json& sfx = obj["sfx"];
			for (int i = 0; i < (int)sound::SFX::Count; i++)
			{
				const char* name = EnumParser(static_cast<sound::SFX>(i));

				if (sfx[name].empty()) { continue; }

				KE::GlobalAudio::sfxData[i].volume = sfx[name];
			}
		}
	}

	const char* GlobalAudio::EnumParser(sound::SFX aType)
	{
		const char* name = "";

		switch (aType)
		{
		case sound::SFX::BallParry:
			name = "BallParry";
			break;
		case sound::SFX::BallCollision:
			name = "Ball Collision";
			break;
		case sound::SFX::BallImpact:
			name = "Ball Impact";
			break;
		case sound::SFX::BallBounce:
			name = "Ball Bounce";
			break;

		case sound::SFX::PortalTeleport:
			name = "Portal Teleport";
			break;
		case sound::SFX::DeathzoneTick:
			name = "DeathzoneTick";
			break;

		case sound::SFX::PowerupShieldActivate:
			name = "Shield Activate";
			break;
		case sound::SFX::PowerupShieldDeActivate:
			name = "Shield DeActivate";
			break;
		case sound::SFX::PowerupPickup:
			name = "Powerup Pickup";
			break;
		case sound::SFX::PowerupTeleport:
			name = "Ball Teleport";
			break;
		case sound::SFX::PowerupSplitBall:
			name = "Split Ball";
			break;
		case sound::SFX::PowerupExplosiveBall:
			name = "Explosive Ball";
			break;
		case sound::SFX::PowerupTelekinesis:
			name = "Telekinesis";
			break;

		case sound::SFX::PlayerThrow:
			name = "Player Throw";
			break;
		case sound::SFX::PlayerDash:
			name = "Player Dash";
			break;
		case sound::SFX::PlayerFalling:
			name = "Player Falling";
			break;
		case sound::SFX::PlayerMeele:
			name = "Player Meele";
			break;

		case sound::SFX::PlayerTauntCandy:
			name = "PlayerTauntCandy";
			break;
		case sound::SFX::PlayerTauntDew:
			name = "PlayerTauntDew";
			break;
		case sound::SFX::PlayerTauntCinder:
			name = "PlayerTauntCinder";
			break;
		case sound::SFX::PlayerTauntMal:
			name = "PlayerTauntMal";
			break;
		case sound::SFX::PlayerTauntGod:
			name = "PlayerTauntGod";
			break;

		case sound::SFX::PlayerDeathCandy:
			name = "PlayerDeathCandy";
			break;
		case sound::SFX::PlayerDeathDew:
			name = "PlayerDeathDew";
			break;
		case sound::SFX::PlayerDeathCinder:
			name = "PlayerDeathCinder";
			break;
		case sound::SFX::PlayerDeathMal:
			name = "PlayerDeathMal";
			break;
		case sound::SFX::PlayerDeathGod:
			name = "PlayerDeathGod";
			break;

		case sound::SFX::PlayerSelectCandy:
			name = "PlayerSelectCandy";
			break;
		case sound::SFX::PlayerSelectDew:
			name = "PlayerSelectDew";
			break;
		case sound::SFX::PlayerSelectCinder:
			name = "PlayerSelectCinder";
			break;
		case sound::SFX::PlayerSelectMal:
			name = "PlayerSelectMal";
			break;
		case sound::SFX::PlayerSelectGod:
			name = "PlayerSelectGod";
			break;

		case sound::SFX::AnnouncerGo:
			name = "AnnouncerGo";
			break;
		case sound::SFX::AnnouncerReady:
			name = "AnnouncerReady";
			break;
		case sound::SFX::AnnouncerWinner:
			name = "AnnouncerWinner";
			break;
		case sound::SFX::AnnouncerChampion:
			name = "AnnouncerChampion";
			break;
		case sound::SFX::Vignette:
			name = "Vignette";
			break;



		case sound::SFX::MenuToggle:
			name = "Menu Toggle";
			break;
		case sound::SFX::MenuSelect:
			name = "Menu Select";
			break;
		case sound::SFX::ScoreboardPoint:
			name = "ScoreboardPoint";
			break;


		default:
			name = "Unknown";
			break;
		}

		return name;
	}
	const char* GlobalAudio::EnumParser(sound::Music aType)
	{
		const char* name = "";

		switch (aType)
		{
		case sound::Music::Space:
			name = "Space";
			break;
		case sound::Music::Lilypad:
			name = "Lilypad";
			break;
		case sound::Music::SpoopyMansion:
			name = "SpoopyMansion";
			break;
		case sound::Music::Menu:
			name = "Menu";
			break;
		case sound::Music::Winscreen:
			name = "Winscreen";
			break;

		default:
			name = "Unknown";
			break;
		}

		return name;
	}
	const char* GlobalAudio::EnumParser(sound::Ambient aType)
	{
		const char* name = "";

		switch (aType)
		{
		case sound::Ambient::SpoopyMansion:
			name = "SpoopyMansion";
			break;
		case sound::Ambient::Lilypad:
			name = "Lilypad";
			break;
		case sound::Ambient::Space:
			name = "Space";
			break;

		default:
			name = "Unknown";
			break;
		}

		return name;
	}
}

#undef Volume
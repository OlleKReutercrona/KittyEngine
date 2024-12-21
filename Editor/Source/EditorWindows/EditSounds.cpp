#include "stdafx.h"
#ifndef KITTYENGINE_NO_EDITOR

#include "EditSounds.h"
#include <nlohmann/json.hpp>
#include "Editor/Source/Editor.h"

constexpr const char* soundSettings = "Data/Settings/SoundSettings.json";

namespace KE_EDITOR
{
	void EditSounds::Init()
	{
		Load();
	}

	void EditSounds::Update()
	{

	}

	void EditSounds::Render()
	{
		float min = 0.0f;
		float max = 100.0f;

		if (ImGui::TreeNode("Mastering"))
		{
			ImGui::PushItemWidth(100.0f);

			const char* name = "Music Volume";

			float musicVolume = KE::GlobalAudio::myMusicMaxVolume * 100;
			if (ImGui::SliderFloat(name, &musicVolume, min, max))
			{
				KE::GlobalAudio::myMusicMaxVolume = musicVolume * 0.01f;
			}

			name = "Ambient Volume";
			float ambientVolume = KE::GlobalAudio::myAmbientMaxVolume * 100;
			if (ImGui::SliderFloat(name, &ambientVolume, min, max))
			{
				KE::GlobalAudio::myAmbientMaxVolume = ambientVolume * 0.01f;
			}

			ImGui::PopItemWidth();
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("SFX"))
		{
			ImGui::PushItemWidth(100.0f);
			for (int i = 0; i < (int)sound::SFX::Count; i++)
			{
				const char* name = KE::GlobalAudio::EnumParser(static_cast<sound::SFX>(i));
				int volume = static_cast<int>(KE::GlobalAudio::sfxData[i].volume * 100);
				
				if (ImGui::SliderInt(name, &volume, static_cast<int>(min), static_cast<int>(max)))
				{
					KE::GlobalAudio::sfxData[i].volume = volume * 0.01f;
				}
			}
			ImGui::PopItemWidth();
			ImGui::TreePop();
		}
		//if (ImGui::TreeNode("Music"))
		//{
		//	ImGui::PushItemWidth(100.0f);
		//	for (int i = 0; i < (int)sound::Music::Count; i++)
		//	{
		//		const char* name = EnumParser(static_cast<sound::Music>(i));
		//		int volume = KE::GlobalAudio::musicData[i].volume * 100;
		//
		//		ImGui::SliderInt(name, &volume, myMin, myMax);
		//		KE::GlobalAudio::musicData[i].volume = volume * 0.01f;
		//	}
		//	ImGui::PopItemWidth();
		//	ImGui::TreePop();
		//}
		//if (ImGui::TreeNode("Ambient"))
		//{
		//	ImGui::PushItemWidth(100.0f);
		//	for (int i = 0; i < (int)sound::Ambient::Count; i++)
		//	{
		//		const char* name = EnumParser(static_cast<sound::Ambient>(i));
		//		int volume = KE::GlobalAudio::ambientData[i].volume * 100;
		//
		//		ImGui::SliderInt(name, &volume, myMin, myMax);
		//		KE::GlobalAudio::ambientData[i].volume = volume * 0.01f;
		//	}
		//	ImGui::PopItemWidth();
		//	ImGui::TreePop();
		//}

		if (ImGui::Button("Save"))
		{
			Save();
		}
	}
	void EditSounds::Save()
	{
		nlohmann::ordered_json obj;

		obj["master"]["music"] = KE::GlobalAudio::myMusicMaxVolume;
		obj["master"]["ambient"] = KE::GlobalAudio::myAmbientMaxVolume;

		for (int i = 0; i < (int)sound::SFX::Count; i++)
		{
			const char* name = KE::GlobalAudio::EnumParser(static_cast<sound::SFX>(i));
			obj["sfx"][name] = KE::GlobalAudio::sfxData[i].volume;
		}
		//for (int i = 0; i < (int)sound::Music::Count; i++)
		//{
		//	const char* name = EnumParser(static_cast<sound::Music>(i));
		//	obj["sfx"][name] = KE::GlobalAudio::musicData[i].volume;
		//}
		//for (int i = 0; i < (int)sound::Ambient::Count; i++)
		//{
		//	const char* name = EnumParser(static_cast<sound::Ambient>(i));
		//	obj["ambient"][name] = KE::GlobalAudio::ambientData[i].volume;
		//}
		
		// Save to file.
		if (
			std::filesystem::exists(soundSettings) &&
			std::filesystem::is_regular_file(soundSettings) &&
			std::filesystem::status(soundSettings).permissions() == std::filesystem::perms::_File_attribute_readonly
			)
		{
			std::filesystem::permissions(soundSettings, std::filesystem::perms::owner_write, std::filesystem::perm_options::replace);
		}
		
		// Write to file.
		std::string jsonStr = obj.dump(4);
		std::ofstream file(soundSettings);
		file << jsonStr;
		file.close();
	}
	void EditSounds::Load()
	{
		std::ifstream ifs(soundSettings);
		
		if (!std::filesystem::exists(soundSettings)) return;
		if (!ifs.good()) return;
		nlohmann::json obj = nlohmann::json::parse(ifs);
		ifs.close();

		if(!obj["master"].empty())
		{
			nlohmann::json& master = obj["master"];

			KE::GlobalAudio::myMusicMaxVolume = master["music"];
		}

		if (!obj["sfx"].empty()) 
		{
			nlohmann::json& sfx = obj["sfx"];
			for (int i = 0; i < (int)sound::SFX::Count; i++)
			{
				const char* name = KE::GlobalAudio::EnumParser(static_cast<sound::SFX>(i));

				if (sfx[name].empty()) { continue; }

				KE::GlobalAudio::sfxData[i].volume = sfx[name];
			}
		}
		//if (!obj["music"].empty())
		//{
		//	nlohmann::json& music = obj["music"];
		//	for (int i = 0; i < (int)sound::Music::Count; i++)
		//	{
		//		const char* name = EnumParser(static_cast<sound::Music>(i));
		//		KE::GlobalAudio::musicData[i].volume = music[name];
		//	}
		//}
		//if (!obj["ambient"].empty())
		//{
		//	nlohmann::json& ambient = obj["ambient"];
		//	for (int i = 0; i < (int)sound::Ambient::Count; i++)
		//	{
		//		const char* name = EnumParser(static_cast<sound::Ambient>(i));
		//		KE::GlobalAudio::ambientData[i].volume = ambient[name];
		//	}
		//}
	}
	//const char* EditSounds::EnumParser(sound::SFX aType)
	//{
	//	const char* name = "";

	//	switch (aType)
	//	{
	//	case sound::SFX::BallParry:
	//		name = "BallParry";
	//		break;
	//	case sound::SFX::BallCollision:
	//		name = "Ball Collision";
	//		break;
	//	case sound::SFX::BallImpact:
	//		name = "Ball Impact";
	//		break;
	//	case sound::SFX::BallBounce:
	//		name = "Ball Bounce";
	//		break;

	//	case sound::SFX::PortalTeleport:
	//		name = "Portal Teleport";
	//		break;

	//	case sound::SFX::PowerupShieldActivate:
	//		name = "Shield Activate";
	//		break;
	//	case sound::SFX::PowerupShieldDeActivate:
	//		name = "Shield DeActivate";
	//		break;
	//	case sound::SFX::PowerupPickup:
	//		name = "Powerup Pickup";
	//		break;
	//	case sound::SFX::PowerupTeleport:
	//		name = "Ball Teleport";
	//		break;
	//	case sound::SFX::PowerupSplitBall:
	//		name = "Split Ball";
	//		break;
	//	case sound::SFX::PowerupExplosiveBall:
	//		name = "Explosive Ball";
	//		break;

	//	case sound::SFX::PlayerThrow:
	//		name = "Player Throw";
	//		break;
	//	case sound::SFX::PlayerDash:
	//		name = "Player Dash";
	//		break;
	//	case sound::SFX::PlayerFalling:
	//		name = "Player Falling";
	//		break;
	//	case sound::SFX::PlayerMeele:
	//		name = "Player Meele";
	//		break;

	//	case sound::SFX::PlayerTauntCandy:
	//		name = "PlayerTauntCandy";
	//		break;
	//	case sound::SFX::PlayerTauntDew:
	//		name = "PlayerTauntDew";
	//		break;
	//	case sound::SFX::PlayerTauntCinder:
	//		name = "PlayerTauntCinder";
	//		break;
	//	case sound::SFX::PlayerTauntMal:
	//		name = "PlayerTauntMal";
	//		break;

	//	case sound::SFX::PlayerDeathCandy:
	//		name = "PlayerDeathCandy";
	//		break;
	//	case sound::SFX::PlayerDeathDew:
	//		name = "PlayerDeathDew";
	//		break;
	//	case sound::SFX::PlayerDeathCinder:
	//		name = "PlayerDeathCinder";
	//		break;
	//	case sound::SFX::PlayerDeathMal:
	//		name = "PlayerDeathMal";
	//		break;

	//	case sound::SFX::PlayerSelectCandy:
	//		name = "PlayerSelectCandy";
	//		break;
	//	case sound::SFX::PlayerSelectDew:
	//		name = "PlayerSelectDew";
	//		break;
	//	case sound::SFX::PlayerSelectCinder:
	//		name = "PlayerSelectCinder";
	//		break;
	//	case sound::SFX::PlayerSelectMal:
	//		name = "PlayerSelectMal";
	//		break;

	//	case sound::SFX::AnnouncerGo:
	//		name = "AnnouncerGo";
	//		break;
	//	case sound::SFX::AnnouncerReady:
	//		name = "AnnouncerReady";
	//		break;
	//	case sound::SFX::AnnouncerWinner:
	//		name = "AnnouncerWinner";
	//		break;
	//	case sound::SFX::AnnouncerChampion:
	//		name = "AnnouncerChampion";
	//		break;



	//	case sound::SFX::MenuToggle:
	//		name = "Menu Toggle";
	//		break;
	//	case sound::SFX::MenuSelect:
	//		name = "Menu Select";
	//		break;


	//	default:
	//		name = "Unknown";
	//		break;
	//	}

	//	return name;
	//}
	//const char* EditSounds::EnumParser(sound::Music aType)
	//{
	//	const char* name = "";

	//	switch (aType)
	//	{
	//	case sound::Music::Space:
	//		name = "Space";
	//		break;
	//	case sound::Music::Lilypad:
	//		name = "Lilypad";
	//		break;
	//	case sound::Music::SpoopyMansion:
	//		name = "SpoopyMansion";
	//		break;
	//	case sound::Music::Menu:
	//		name = "Menu";
	//		break;

	//	default:
	//		name = "Unknown";
	//		break;
	//	}

	//	return name;
	//}
	//const char* EditSounds::EnumParser(sound::Ambient aType)
	//{
	//	const char* name = "";

	//	switch (aType)
	//	{
	//	case sound::Ambient::SpoopyMansion:
	//		name = "SpoopyMansion";
	//		break;
	//	case sound::Ambient::Lilypad:
	//		name = "Lilypad";
	//		break;
	//	case sound::Ambient::Space:
	//		name = "Space";
	//		break;

	//	default:
	//		name = "Unknown";
	//		break;
	//	}

	//	return name;
	//}
}
#endif
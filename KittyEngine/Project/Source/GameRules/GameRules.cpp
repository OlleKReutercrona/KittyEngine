#include "stdafx.h"
#include "GameRules.h"

P8::GameRules* P8::GameRules::ourGameRulesInstance;

P8::GameRules::GameRules()
{
	ourGameRulesInstance = this;
}

P8::GameRules::~GameRules()
{
}

P8::GameRules* P8::GameRules::GetInstance()
{
	return ourGameRulesInstance;
}

void P8::GameRules::CreateInstance()
{
	if (ourGameRulesInstance != nullptr)
	{
		std::cout << "More than one gamerule created!\n";
		return;
	}
	
	ourGameRulesInstance = new GameRules;
	ourGameRulesInstance->ResetGameRules();
}

void P8::GameRules::ChangeGameRules(GameRuleSet* aGameRuleSet)
{
	myCurrentGameRuleSet = aGameRuleSet;
	SaveGameRules();
}

void P8::GameRules::SaveGameRules()
{
	nlohmann::basic_json settings;

	settings["PlayerMovementSpeed"] = myCurrentGameRuleSet->PlayerMovementSpeed;
	settings["PlayerThrowingDistance"] = myCurrentGameRuleSet->PlayerThrowDistance;
	settings["ItemSpawn"] = myCurrentGameRuleSet->ItemSpawn;

	std::ofstream o("Data/Settings/GameRuleSet.json");
	o << std::setw(4) << settings << std::endl;
}

void P8::GameRules::ResetGameRules()
{
	myCurrentGameRuleSet = new GameRuleSet;
}

void P8::GameRules::Init()
{
	//GameRuleSet* gameRuleSet = new GameRuleSet;
	//
	//std::string settingsPath = "Data/Settings/GameRuleSet.json";
	//nlohmann::json settings = GetJsonObj(settingsPath);
	//
	//if (settings.empty())
	//{
	//	std::cout << "GameRuleSet empty!\n";
	//	ChangeGameRules(gameRuleSet);
	//	return;
	//}
	//
	//gameRuleSet->PlayerMovementSpeed = settings["PlayerMovementSpeed"].get<float>();
	//gameRuleSet->PlayerThrowDistance = settings["PlayerThrowingDistance"].get<float>();
	//gameRuleSet->ItemSpawn = settings["ItemSpawn"].get<bool>();
	//
	//ChangeGameRules(gameRuleSet);
}

P8::GameRuleSet* P8::GameRules::GetGameRules()
{
	return myCurrentGameRuleSet;
}

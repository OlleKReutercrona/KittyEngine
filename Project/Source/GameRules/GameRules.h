#pragma once
#include <External/Include/nlohmann/json.hpp>
#include <string>

namespace P8
{
	struct GameRuleSet
	{
		float PlayerMovementSpeed = 1.f;
		float PlayerThrowDistance = 1.f;
		bool ItemSpawn = true;
	};

	class GameRules
	{
	public:

		GameRules();
		~GameRules();

		void ChangeGameRules(GameRuleSet* aGameRuleSet);
		void SaveGameRules();
		void ResetGameRules();
		void Init();

		inline nlohmann::json GetJsonObj(std::string& aFilePath);

		GameRuleSet* GetGameRules();

	public:
		// Static functions
		static void CreateInstance();
		static GameRules* GetInstance();

	private:

		GameRuleSet* myCurrentGameRuleSet;

	private:
		// Static instance of the game rules.
		static GameRules* ourGameRulesInstance;
	};

	inline nlohmann::json GameRules::GetJsonObj(std::string& aFilePath)
	{
		std::ifstream ifs(aFilePath);

		nlohmann::json obj;
		if (ifs.good())
		{
			obj = nlohmann::json::parse(ifs);
		}

		ifs.close();

		return obj;
	}
}
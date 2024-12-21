#pragma once
#include <string>
#include <unordered_map>
#include <chrono>

namespace KE
{
	struct LoggedVar
	{
		LoggedVar(const std::string& aName, LoggedVar* aParent = nullptr) : myName(aName), myParent(aParent) {};

		std::string myName = "";
		std::chrono::high_resolution_clock::time_point myBeginTime;
		float myTime = 0.0f;

		LoggedVar* myParent = nullptr;
		std::unordered_map<std::string, LoggedVar> myChildren = {};

	};

	class DebugTimeLogger
	{
	public:
		static void BeginLogVar(const std::string& aString);
		static void EndLogVar(const std::string& aString);

		static void LogFPS();
		static int GetAvarageFPS();

		static inline std::unordered_map<std::string, LoggedVar> myHeads;
	private:
		static inline LoggedVar* myCurrentParent;
	};

	inline void DebugTimeLogger::BeginLogVar(const std::string& aString)
	{
		if (myCurrentParent == nullptr)
		{
			// if new Var, create
			if (myHeads.count(aString) == 0)
			{
				myHeads.insert(std::pair(aString, LoggedVar(aString)));
			}

			LoggedVar* var = &myHeads.at(aString);
			var->myBeginTime = std::chrono::high_resolution_clock::now();

			myCurrentParent = var;
			return;
		}


		if (myCurrentParent->myChildren.count(aString) == 0)
		{
			myCurrentParent->myChildren.insert(std::pair(aString, LoggedVar(aString, myCurrentParent)));
		}
		LoggedVar* var = &myCurrentParent->myChildren.at(aString);

		var->myBeginTime = std::chrono::high_resolution_clock::now();
		
		myCurrentParent = var;
	}

	inline void DebugTimeLogger::EndLogVar(const std::string& aString)
	{
		if (myCurrentParent != nullptr && myCurrentParent->myName == aString)
		{
			// LogVar is parent node

			myCurrentParent->myTime = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - myCurrentParent->myBeginTime).count() * 1000.0f;

			myCurrentParent = myCurrentParent->myParent;

			return;
		}

		// LogVar is not parent node

		LoggedVar* var = &myCurrentParent->myChildren.at(aString);
		var->myTime = std::chrono::duration_cast<std::chrono::duration<float>>(std::chrono::high_resolution_clock::now() - var->myBeginTime).count() * 1000.0f;
		myCurrentParent = myCurrentParent->myParent;
	}
}
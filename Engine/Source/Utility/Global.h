#pragma once
#include <any>

#include "Engine/Source/Audio/AudioWrapper.h"
#include "Engine/Source/Audio/AudioPlayer.h"
//#include <Engine/Source/ComponentSystem/Components/AudioComponent.h>

//remove this!
#include <typeindex>
namespace KE_GLOBAL
{
	inline float gameDeltaTime = 0.0f;
	inline float deltaTime = 0.0f;
	inline float trueDeltaTime = 0.0f;
	inline float totalTime = 0.0f;
	inline KE::AudioWrapper audioWrapper;
	inline KE::AudioPlayer audioPlayer;
	inline const Vector2i* resolution;

	class Blackboard
	{
	private:
		std::unordered_map<std::type_index, void*> myData;

	public:
		template<typename T>
		T* Get(const std::string& aKey = "")
		{
			auto iter = myData.find(typeid(T));
			return iter != myData.end() ? static_cast<T*>(iter->second) : nullptr;

			//auto iter = myData.find(aKey);
			//if (iter == myData.end())
			//{
			//	return nullptr;
			//}
			//return static_cast<T*>(iter->second);
		}

		template<typename T>
		void Register(const std::string& aKey = "", T* aData = nullptr)
		{
			//myData[aKey] = aData;
			myData[typeid(T)] = aData;
		}

		template<typename T>
		void Register(T* aData)
		{
			myData[typeid(T)] = aData;
		}

		//void Deregister(const std::string& aKey)
		//
		//	if (myData.count(aKey) > 0)
		//	{
		//		myData.erase(aKey);
		//	}
		//}

		//template<typename T>
		//T* operator[](const std::string& aKey)
		//{
		//	return Get<T>(aKey);
		//}
	};

	inline Blackboard blackboard;
}
#pragma once

#ifndef KITTYENGINE_NO_EDITOR

namespace KE_SER
{
	struct Data;
}

namespace KE_EDITOR
{
	using InspectionFunction = std::function<bool(const char* variableName, void* variable)>;


	class EditorInspectionSystem
	{
	private:
		std::unordered_map<std::string, InspectionFunction> myInspectionFunctions;
		std::unordered_map<std::string, std::string> beautifiedMemberNames;


	public:
		EditorInspectionSystem();
		~EditorInspectionSystem();

		template<typename T>
		void AddType(InspectionFunction aFunction);

		void Inspect(void* anObject, KE_SER::Data& aSerializationData);

		const std::string& BeautifyMemberName(const std::string& aMemberName);
	};

	template <typename T>
	void EditorInspectionSystem::AddType(InspectionFunction aFunction)
	{
		myInspectionFunctions[typeid(T).name()] = aFunction;
	}
}

#endif
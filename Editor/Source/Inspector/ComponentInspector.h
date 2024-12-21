#pragma once
#include <string>

#include "imgui/imgui_internal.h"

#ifndef KITTYENGINE_NO_EDITOR
#define PURR(var) #var, &var

namespace KE
{
	class SphereColliderComponent;
}

namespace KE_EDITOR
{
	class ComponentInspector
	{
	private:


	public:
		ComponentInspector();
		~ComponentInspector();

		std::string StructureData(const char* aDirtyData, KE::SphereColliderComponent* aColliderComponent);
		void Inspect(const char* aDirtyData, KE::SphereColliderComponent* aColliderComponent);
	};



}

#endif
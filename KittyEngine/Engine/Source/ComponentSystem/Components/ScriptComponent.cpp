#include "stdafx.h"
#include "Script/Script.h"
#include "Script/ScriptExecution.h"
#include "ScriptComponent.h"

#include "Utility/DebugTimeLogger.h"

KE::ScriptComponent::ScriptComponent(GameObject& aParentGameObject) : Component(aParentGameObject)
{
	
}

KE::ScriptComponent::~ScriptComponent()
{
}

void KE::ScriptComponent::SetData(void* aDataObject)
{
	myScript = ((ScriptComponentData*)aDataObject)->script;
}

void KE::ScriptComponent::DefaultData()
{
	
}

void KE::ScriptComponent::Awake()
{
	myScriptRuntime.Init(myScript,&myGameObject);

	myScriptRuntime.TriggerEntryPoint(EntryPointType::Awake);
}

void KE::ScriptComponent::Update()
{
	KE::DebugTimeLogger::BeginLogVar("Script Update");
	myScriptRuntime.TriggerEntryPoint(EntryPointType::Update);
	KE::DebugTimeLogger::EndLogVar("Script Update");
}

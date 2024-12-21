#include "stdafx.h"
#include "Engine/Source/Graphics/Renderers/ShellTexturedRenderer.h"
#include "ShellTexturingComponent.h"

#include <filesystem>

#include "ComponentSystem/GameObject.h"
#include "ComponentSystem/GameObjectManager.h"
#include "Graphics/DebugRenderer.h"
#include "Graphics/Graphics.h"
//#include "imgui/imgui.h"
#include "Project/Source/Player/Player.h"
#include "Project/Source/Boomerang/BoomerangComponent.h"
#include "Project/Source/Managers/BallManager.h"
#include "SceneManagement/Scene.h"

KE::ShellTexturingComponent::ShellTexturingComponent(GameObject& aParentGameObject) : Component(aParentGameObject)
{
	myGraphics = KE_GLOBAL::blackboard.Get<Graphics>();

}

void KE::ShellTexturingComponent::SetData(void* aDataObject)
{
	auto* data = static_cast<ShellTexturingComponentData*>(aDataObject);
	shellModelData = data->shellModelData;

	if (shellModelData->effectsRT)
	{
		float clear[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		shellModelData->effectsRT->Clear(clear, true);
		/*ClearDisplacement();*/
	}

	liveUpdate = data->liveUpdate;
}

void KE::ShellTexturingComponent::DefaultData()
{

}

void KE::ShellTexturingComponent::Refresh()
{
	otherShellTexturingComponents.clear();
	myDisplacementObjects.clear();

	const auto& shellTexturingComponentObjs = myGameObject.GetManager().GetGameObjectsWithComponent<ShellTexturingComponent>();
	for (const auto& obj : shellTexturingComponentObjs)
	{
		ShellTexturingComponent& shellTexturingComponent = obj->GetComponent<ShellTexturingComponent>();
		if (&shellTexturingComponent == this) { continue; }
		
		shellTexturingComponent.primary = primary && shellTexturingComponent.primary ? false : shellTexturingComponent.primary;
		otherShellTexturingComponents.push_back(&shellTexturingComponent);
	}

	if (!primary) { return; }

	/*const auto& players = myGameObject.GetManager().GetGameObjectsWithComponent<P8::Player>();
	const auto& boomers = myGameObject.GetManager().GetGameObjectsWithComponent<P8::BoomerangComponent>();

	for (const auto& player : players)
	{
		myDisplacementObjects.push_back({ player, {0.5f, 0.5f, 0.5f} });
	}

	for (const auto& boomer : boomers)
	{
		myDisplacementObjects.push_back({ boomer, {0.5f, 1.0f, 0.5f} });
	}*/

	//myDisplacementObjects.push_back({myGameObject.GetManager().GetGameObject(4), {1.0f, 1.0f, 1.0f} });

}

void KE::ShellTexturingComponent::CalculateBounds()
{
	const Vector3f& tScl = myGameObject.myTransform.GetScale();

	Vector3f ownMinBounds = myGameObject.myTransform.GetPosition() - tScl/2.0f;
	Vector3f ownMaxBounds = myGameObject.myTransform.GetPosition() + tScl/2.0f;

	Vector3f sharedMinimumBounds = ownMinBounds;
	Vector3f sharedMaximumBounds = ownMaxBounds;

	for (const auto& shellTexturingComponent : otherShellTexturingComponents)
	{
		const Vector3f& pos = shellTexturingComponent->myGameObject.myTransform.GetPosition();
		const Vector3f& scl = shellTexturingComponent->myGameObject.myTransform.GetScale();
		sharedMinimumBounds = {
			std::min(sharedMinimumBounds.x, pos.x - scl.x/2.0f),
			std::min(sharedMinimumBounds.y, pos.y - scl.y/2.0f),
			std::min(sharedMinimumBounds.z, pos.z - scl.z/2.0f)
		};
		sharedMaximumBounds = {
			std::max(sharedMaximumBounds.x, pos.x + scl.x/2.0f),
			std::max(sharedMaximumBounds.y, pos.y + scl.y/2.0f),
			std::max(sharedMaximumBounds.z, pos.z + scl.z/2.0f)
		};
	}

	sharedBounds.min = sharedMinimumBounds;
	sharedBounds.max = sharedMaximumBounds;


	float minXProgress = (ownMinBounds.x - sharedMinimumBounds.x) / (sharedMaximumBounds.x - sharedMinimumBounds.x);
	float minZProgress = (ownMinBounds.z - sharedMinimumBounds.z) / (sharedMaximumBounds.z - sharedMinimumBounds.z);
	float maxXProgress = (ownMaxBounds.x - sharedMinimumBounds.x) / (sharedMaximumBounds.x - sharedMinimumBounds.x);
	float maxZProgress = (ownMaxBounds.z - sharedMinimumBounds.z) / (sharedMaximumBounds.z - sharedMinimumBounds.z);
	

	shellModelData->attributes.textureRegionMin = { minXProgress, minZProgress };
	shellModelData->attributes.textureRegionMax = { maxXProgress, maxZProgress };
}

void KE::ShellTexturingComponent::RenderDisplacement()
{

	static float baseDisplace = 0.6f;
	static float heightDiv = 7.0f;

	//ImGui::Begin("Displacement Settings");
	//ImGui::DragFloat("Base Displacement", &baseDisplace, 0.01f, 0.0f, 1.0f);
	//ImGui::DragFloat("Height Divisor", &heightDiv, 0.01f, 0.0f, 10.0f);
	//ImGui::End();

	std::vector<ShellTextureDisplacement> displacements;

	if (!hasSetlevelDisplacement && shellModelData->effectsRT)
	{
		SetDisplacementForLevel();
		hasSetlevelDisplacement = true;
	}

	//for (const auto& displacement : myDisplacementObjects)
	//{
	//	displacements.push_back({ displacement.gameObject->myWorldSpaceTransform.GetPosition(), displacement.displacementScale });
	//}

	myGraphics->GetShellTexturedRenderer()->FadeDisplacement(shellModelData, 1.0f, 1.0f);

	if (!liveUpdate) { return; }

	auto& ballMGR = myGameObject.GetManager().GetGameObject(eGameSystemManager)->GetComponent<P8::BallManager>();
	displacements.clear();
	for (const auto& boomer : ballMGR.GetBalls())
	{
		if (!boomer->GetGameObject().IsActive()) { continue; }
		Vector3f scl = boomer->GetGameObject().myWorldSpaceTransform.GetScale();
		if (boomer->GetGameObject().myWorldSpaceTransform.GetPosition().LengthSqr() <= 0.001f) { continue; } 

		displacements.push_back({ 
			boomer->GetGameObject().myWorldSpaceTransform.GetPosition(),
			Vector3f(
				1.0f * scl.x,
				baseDisplace + (shellModelData->attributes.totalHeight / heightDiv),
				1.0f * scl.z
			)
		});
	}

	for (const auto& player : ballMGR.GetPlayers())
	{
		if (!player->GetGameObject().IsActive()) { continue; }

		Vector3f scl = player->GetGameObject().myWorldSpaceTransform.GetScale();
		if (player->GetGameObject().myWorldSpaceTransform.GetPosition().LengthSqr() <= 0.001f) { continue; }

		displacements.push_back({ 
			player->GetGameObject().myWorldSpaceTransform.GetPosition(),
			Vector3f(
				0.5f * scl.x,
			baseDisplace + (shellModelData->attributes.totalHeight / heightDiv),
				0.5f * scl.z
			)
		});
	}

	//if (displacements.empty()) { return;  }
	CalculateBounds();
	myGraphics->GetShellTexturedRenderer()->RenderDisplacement(
		shellModelData,
		displacements,
		sharedBounds.min,
		sharedBounds.max,
		nullptr
	);
}

void KE::ShellTexturingComponent::SetDisplacementForLevel()
{
	std::string folder = "Data/InternalAssets/ShellDisplacements/";
	std::string fileName = std::format(
		"Displacement_{}.dds",
		myGameObject.GetManager().GetScene()->sceneName
	);
	std::string path = std::format("{}{}", folder, fileName);
	std::string zeroPath = std::format("{}{}", folder, "ZeroDisplacement.dds");

	if (!std::filesystem::exists(path))
	{

		//check if file is read only
		if (std::filesystem::exists(path) && 
			std::filesystem::is_regular_file(path) && 
			std::filesystem::status(path).permissions() == std::filesystem::perms::_File_attribute_readonly)
		{
			std::filesystem::permissions(path, std::filesystem::perms::owner_write, std::filesystem::perm_options::replace);
		}

		std::filesystem::copy(zeroPath, path, std::filesystem::copy_options::overwrite_existing);

	}

	Texture* displacementTexture = myGraphics->GetTextureLoader().GetTextureFromPath(path, false, true);
	shellModelData->displacementTexture = displacementTexture;
}

void KE::ShellTexturingComponent::ClearDisplacement()
{
	std::string folder = "Data/InternalAssets/ShellDisplacements/";
	std::string fileName = std::format(
		"Displacement_{}.dds",
		myGameObject.GetManager().GetScene()->sceneName
	);
	std::string path = std::format("{}{}", folder, fileName);

	std::string zeroPath = std::format("{}{}", folder, "ZeroDisplacement.dds");

	myGraphics->GetShellTexturedRenderer()->ClearDisplacement(shellModelData);

	//check if file is read only
	if (std::filesystem::exists(path) && 
		std::filesystem::is_regular_file(path) && 
		std::filesystem::status(path).permissions() == std::filesystem::perms::_File_attribute_readonly)
	{
		std::filesystem::permissions(path, std::filesystem::perms::owner_write, std::filesystem::perm_options::replace);
	}

	std::filesystem::copy(zeroPath, path, std::filesystem::copy_options::overwrite_existing);
	shellModelData->displacementTexture = myGraphics->GetTextureLoader().GetTextureFromPath(path, false, true);
}

void KE::ShellTexturingComponent::Awake()
{

}

void KE::ShellTexturingComponent::LateUpdate()
{
	if (!primary) { return; }

	RenderDisplacement();
}

void KE::ShellTexturingComponent::Update()
{
	refreshTimer += KE_GLOBAL::deltaTime;
	if (refreshTimer > refreshTime)
	{
		refreshTimer = 0.0f;
		Refresh();
		CalculateBounds();
	}
}

void KE::ShellTexturingComponent::OnEnable()
{
	
}

void KE::ShellTexturingComponent::OnDisable()
{

}

void KE::ShellTexturingComponent::OnDestroy()
{
	
}

void KE::ShellTexturingComponent::DrawDebug(KE::DebugRenderer& aDbg)
{
	Vector3f pos = sharedBounds.min + (sharedBounds.max - sharedBounds.min) * 0.5f;
	Vector3f scale = sharedBounds.max - sharedBounds.min;
	aDbg.RenderCube(pos, scale);
}

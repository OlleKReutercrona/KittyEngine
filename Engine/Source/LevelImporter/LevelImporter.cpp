#include "stdafx.h"
#include "LevelImporter.h"

#include "Graphics/Graphics.h"
#include "Graphics/ModelLoader.h"
#include "Graphics/Texture/TextureLoader.h"
#include "Graphics/ShaderLoader.h"
#include "Graphics/Lighting/LightManager.h"

#include "Engine/Source/ComponentSystem/GameObjectManager.h"
#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/SceneManagement/SceneManager.h"
#include "Engine/Source/SceneManagement/Scene.h"
#include "Engine/Source/Math/KittyMath.h"
#include "Engine/Source/Utility/Logging.h"
#include "Engine/Source/Collision/Collider.h"
#include "Engine/Source/Collision/Layers.h"
#include "Engine/Source/Utility/Timer.h"
#include "Engine/Source/Files/LeMeowFile.h"
#include "Engine/Source/LevelImporter/NavmeshBuilder.h"
#include "Engine/Source/AI/BehaviourTree/BehaviourTreeBuilder.h"
#include "Engine/Source/AI/BehaviourTree/BehaviourTree.h"
#include "Engine/Source/Files/KittyMesh.h"
#include "Engine/Source/Audio/GlobalAudio.h"
#include "Script/ScriptManager.h"
#include "Audio/AudioInstance.h"

// <[ENGINE COMPONENTS]> //
#include "ComponentSystem/Components/LightComponent.h"
#include "ComponentSystem/Components/Graphics/ModelComponent.h"
#include "ComponentSystem/Components/Graphics/SkeletalModelComponent.h"
#include "ComponentSystem/Components/Collider/BoxColliderComponent.h"
#include "ComponentSystem/Components/Collider/SphereColliderComponent.h"
#include "ComponentSystem/Components/Collider/CapsuleColliderComponent.h"
#include "ComponentSystem/Components/Graphics/CameraComponent.h"
#include "ComponentSystem/Components/Graphics/PostProcessingComponent.h"
#include "ComponentSystem/Components/Graphics/VFXComponent.h"
#include "ComponentSystem/Components/SceneTransitionComponent.h"
#include "ComponentSystem/Components/Graphics/DecalComponent.h"
#include "ComponentSystem/Components/ScriptComponent.h"
#include "ComponentSystem/Components/TestComponent.h"
#include "ComponentSystem/Components/Graphics/ShellTexturingComponent.h"

// <[GAMEPLAY COMPONENTS]> //
#include "Project/Source/Player/Player.h"
#include "Project/Source/Boomerang/BoomerangPhysxController.h"
#include "Project/Source/Boomerang/BoomerangComponent.h"
#include "Project/Source/Camera/ActionCameraComponent.h"
#include "Project/Source/Managers/BallManager.h"
#include "Project/Source/Managers/TimeManager.h"
#include "Project/Source/Managers/GameManager.h"
#include "Project/Source/GameSystems/LobbySystem.h"
#include "Project/Source/MovingObject/MovingObject.h"
#include "Project/Source/MovingObject/MovingObjectTrigger.h"
#include "Project/Source/Player/SpawnPointComponent.h"
#include "Project/Source/Portal/Portal.h"
#include "Project/Source/Powerups/PowerupAreaComponent.h"
#include "Project/Source/Powerups/PowerupPickupComponent.h"
#include "Project/Source/BattleArea/BattleArea.h"
#include "Project/Source/Water/WaterComponent.h"

namespace KE
{
	static char playerIndex = -1;


	void LevelImporter::Init(Graphics* aGraphics, SceneManager* aSceneManager)
	{
		myGraphics = aGraphics;
		mySceneManager = aSceneManager;
	}

	bool LevelImporter::LoadLevel(Scene& aScene, std::string& aFileName)
	{
		KE::Timer timer;

		std::string levelPath = myLevelsDir + aFileName;
		nlohmann::json level = GetJsonObj(levelPath);

		if (!level.contains("game_objects"))
		{
			KE_ERROR("Level does not contain game_objects, file %s", aFileName.c_str());
			return false;
		}

		// [Clear] //
		myNavmeshBuilderData.clear();
		myGraphics->GetDeferredLightManager().Reset();
		myGraphics->GetShellTexturedRenderer()->Reset();

		// [Init] //
		LoadLevelSettings(aScene);
		aScene.gameObjectManager.ReserveSpace(level["game_objects"].size());

		playerIndex = -1; //lol

		// [Engine Components created by their own ] //
		{
			std::string name = "";

			name = "Main Camera";
			{
				aScene.AddGameObject(KE::eMainCamera, name);

				CameraComponentData cameraComponentData;
				cameraComponentData.camera = myGraphics->GetCameraManager().GetMainCamera();

				float width = (float)myGraphics->GetRenderWidth();
				float height = (float)myGraphics->GetRenderHeight();

				cameraComponentData.camera->SetPerspective(width, height, KE::DegToRad(90.0f), 0.01f, 1000.0f);
				cameraComponentData.rotation.x = 0.0f;
				cameraComponentData.rotation.y = 0.0f;
				cameraComponentData.rotation.z = 0.0f;
				aScene.AddComponentToLast<CameraComponent>(&cameraComponentData);

				//TEMP: disable camera for select screen
				//TODO: add a component to handle this instead
				//if (aScene.sceneName.find("Select") == std::string::npos)
				//{
				aScene.AddComponentToLast<P8::ActionCameraComponent>(nullptr);
				//}
			}


			name = "Directional Light";
			aScene.AddGameObject(KE::eDirectionalLight, name);

			name = "Post Processing";
			aScene.AddGameObject(KE::ePostProcessing, name);
			PostProcessingComponentData postProcessingComponentData;
			postProcessingComponentData.postProcessing = myGraphics->GetPostProcessing();
			aScene.AddComponentToLast<PostProcessingComponent>(&postProcessingComponentData);

			name = "Game System Manager";
			// Check if GO already exists from earlier level
			if (!aScene.GetGameObjectManager().GetGameObject(KE::eGameSystemManager))
			{
				aScene.AddGameObject(KE::eGameSystemManager, name);
				AddGameSystemComponents(*aScene.GetGameObjectManager().GetLatestGameObject());
			}

			name = "Skybox";
			aScene.AddGameObject(KE::eSkybox, name);
			auto go = aScene.GetGameObjectManager().GetLatestGameObject();

			// todo: add a skybox component to handle this instead
			ModelData& skybox = *myGraphics->CreateModelData("Data/EngineAssets/Cube.fbx");
			skybox.myTransform = &go->myTransform.GetMatrix();
			skybox.myIsInstanced = false;
			auto& rr = skybox.myRenderResources.back();

			rr.myMaterial = myGraphics->GetTextureLoader().GetDefaultMaterial();
			rr.myVertexShader = myGraphics->GetShaderLoader().GetVertexShader(SHADER_LOAD_PATH "Skybox_VS.cso");
			rr.myPixelShader = myGraphics->GetShaderLoader().GetPixelShader(SHADER_LOAD_PATH "Skybox_PS.cso");

			myGraphics->GetRenderLayer(eRenderLayers::Back)->AddModelDataIndex(myGraphics->GetModelData().size() - 1);
		}

		// [Unity Navmesh] //
		if (level.contains("navmesh"))
		{
			std::string navmeshObj = std::string("Data/Navmesh/") + level["navmesh"].get<std::string>();

			aScene.RegisterNavmesh(navmeshObj);
		}

		// [GameSystemData] //
		if (!LoadGameSystemData(aScene, aFileName))
		{
			KE_LOG("One or more GameSystems failed to load data from json");
		}

		// [NEW LOAD GAMEOBJECTS] //
		for (auto& obj : level["game_objects"])
		{
			KE::GameObject& GO = AddGameObject(aScene, obj);

			if (obj.contains("components"))
			{
				AddComponents(mySceneManager->GetCurrentScene(), GO, obj["components"]);
			}
		}

		// TODO This should be turned on when the rotations in transform is fixed
		aScene.AssignAdoptedChildren();

		// [If no Unity DirLight -> Create one] //
		auto go = aScene.gameObjectManager.GetGameObject(1);
		if (go && !go->HasComponent<LightComponent>())
		{
			DirectionalLightData* data = static_cast<DirectionalLightData*>(myGraphics->GetDeferredLightManager().
				CreateLightData(eLightType::Directional));

			data->myColour = { 1.0f, 1.0f, 1.0f };
			data->myDirection = { 0.0f, 0.0f, 0.0f };
			data->myDirectionalLightIntensity = 1.0f;
			data->myAmbientLightIntensity = 1.0f;

			LightComponentData lightComponentData;
			lightComponentData.myLightData = data;
			lightComponentData.myLightType = eLightType::Directional;
			lightComponentData.myLightTypeName = "Directional Light";

			go->AddComponent<LightComponent>();
			go->GetComponent<LightComponent>().SetData(&lightComponentData);

			Transform& t = go->myTransform;
			t.SetDirection({ 0.0f, -0.815f, 0.579f });
		}

		// [Kitty GRID Navmesh] //
		KE::KittyMesh mesh = NavmeshBuilder::Build(myNavmeshBuilderData);
		aScene.myNavmesh.LoadKittyMesh(mesh);
		aScene.myPathfinder.Init(aScene.myNavmesh);

		myGraphics->GetRenderLayer(eRenderLayers::Back)->GenerateInstancingData(myGraphics->GetModelData());
		myGraphics->GetRenderLayer(eRenderLayers::Main)->GenerateInstancingData(myGraphics->GetModelData());
		myGraphics->GetRenderLayer(eRenderLayers::Front)->GenerateInstancingData(myGraphics->GetModelData());
		myGraphics->GetRenderLayer(eRenderLayers::UI)->GenerateInstancingData(myGraphics->GetModelData());

		KE_LOG_CHANNEL("LevelImport", "Obj import took: %f", timer.UpdateDeltaTime());

		return true;
	}

	void KE::LevelImporter::LoadLevelSettings(const Scene& aScene)
	{
		KE::LevelSettingsMeowFile file;

		std::string fileName(mySceneManager->GetCurrentScene()->sceneName);
		fileName.erase(fileName.begin() + fileName.find_last_of("."), fileName.end());
		fileName += ".LeMeow";

		if (file.Load(fileName))
		{
			myGraphics->GetPostProcessing()->SetAttributes(file.myPPData);
		}
	}

	KE::GameObject& LevelImporter::AddGameObject(Scene& aScene, nlohmann::json& aGameObject)
	{
		std::string name = aGameObject["name"];
		int id = aGameObject["id"];
		bool isStatic = false;
		if (aGameObject.contains("isStatic"))
		{
			isStatic = aGameObject["isStatic"];
		}

		if (aGameObject.contains("parentID"))
		{
			int parentID = aGameObject["parentID"];
			if (parentID != INT_MAX)
			{
				aScene.AddChildParentPair(parentID, id);
			}
		}

		// Transform import WIP
		if (aGameObject.contains("transform"))
		{
			auto go = aGameObject["transform"];

			Matrix4x4f mat;
			mat(1, 1) = go["m00"];
			mat(1, 2) = go["m01"];
			mat(1, 3) = go["m02"];
			mat(1, 4) = go["m03"];
			mat(2, 1) = go["m10"];
			mat(2, 2) = go["m11"];
			mat(2, 3) = go["m12"];
			mat(2, 4) = go["m13"];
			mat(3, 1) = go["m20"];
			mat(3, 2) = go["m21"];
			mat(3, 3) = go["m22"];
			mat(3, 4) = go["m23"];
			mat(4, 1) = go["m30"];
			mat(4, 2) = go["m31"];
			mat(4, 3) = go["m32"];
			mat(4, 4) = go["m33"];

			transformData.SetMatrix(mat);
		}
		else
		{
			Vector3f pos = {
				aGameObject["position"]["x"],
				aGameObject["position"]["y"],
				aGameObject["position"]["z"]
			};
			transformData.SetPosition(pos);

			Vector3f rot = {
				KE::DegToRad(aGameObject["rotation"]["x"]),
				KE::DegToRad(aGameObject["rotation"]["y"]),
				KE::DegToRad(aGameObject["rotation"]["z"])
			};

			Vector3f scl = {
				aGameObject["scale"]["x"],
				aGameObject["scale"]["y"],
				aGameObject["scale"]["z"]
			};

			transformData.SetRotation(rot);
			transformData.SetScale(scl);
		}

		KE::GameObject& gameObject = *aScene.AddGameObject(id, name, transformData, isStatic);

		if (aGameObject.contains("layer"))
		{
			int layer = aGameObject["layer"];
			layer = 1 << (layer - 5);

			gameObject.myLayer = static_cast<KE::Collision::Layers>(layer);
		}

		return gameObject;
	}

	inline nlohmann::json LevelImporter::GetJsonObj(std::string& aFilePath)
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


#pragma region OLD_COMPONENT_IMPORTER
//	void LevelImporter::AddComponents(Scene& aScene, nlohmann::json& someComponents)
//	{
//
//
//		GameObject* gameObject = aScene.GetGameObjectManager().GetLatestGameObject();
//
//		for (auto& component : someComponents.items())
//		{
//
//#pragma region EngineComponents
//			//---[CAMERA COMPONENT]--- //
//			if (component.key() == "main_camera")
//			{
//				aScene.gameObjectManager.GetGameObject(0)->myTransform = gameObject->myTransform;
//
//				aScene.gameObjectManager.DestroyGameObject(gameObject->myID);
//			}
//
//			// ---[MODEL COMPONENT]--- //
//			else if (component.key() == "model")
//			{
//				std::string modelPath = std::string("Data/Assets/") + std::string(component.value()["path"]);
//
//				KE::ModelComponentData modelComponentData;
//				modelComponentData.modelData = myGraphics->CreateModelData(modelPath);
//				modelComponentData.modelData->myTransform = &aScene.gameObjectManager.GetLatestGameObject()->myWorldSpaceTransform.GetMatrix();
//
//				eRenderLayers layer = eRenderLayers::Main;
//				if (someComponents.contains("renderLayer"))
//				{
//					layer = static_cast<eRenderLayers>(someComponents["renderLayer"]["layer"]);
//				}
//
//				myGraphics->GetRenderLayer(layer)->AddModelDataIndex(myGraphics->GetModelData().size() - 1);
//
//				aScene.AddComponentToLast<ModelComponent>(&modelComponentData);
//			}
//
//			// ---[COOLER MODEL COMPONENT]--- //
//			else if (component.key() == "skeletal_model")
//			{
//				std::string modelPath = std::string("Data/Assets/") + std::string(component.value()["path"]);
//
//				KE::SkeletalModelComponentData skeletalModelComponentData;
//				skeletalModelComponentData.skeletalModelData = myGraphics->CreateSkeletalModelData(modelPath);
//				skeletalModelComponentData.skeletalModelData->myTransform = &gameObject->myWorldSpaceTransform.GetMatrix();
//
//				// TODO This should be handled properly by FBX Importer vvvvvvv
//				//DirectX::XMVECTOR translation, rotation, scale;
//				//DirectX::XMMatrixDecompose(&scale, &rotation, &translation, *skeletalModelComponentData.skeletalModelData->myTransform);
//				//scale = DirectX::XMVectorScale(scale, 0.01f);
//				//*skeletalModelComponentData.skeletalModelData->myTransform = DirectX::XMMatrixAffineTransformation(scale, DirectX::XMVectorZero(), rotation, translation);
//				// TODO This should be handled properly by FBX Importer ^^^^^^^^
//
//				for (auto& animation : component.value()["animationPaths"])
//				{
//					std::string animationPath = std::string("Data/Assets/") + std::string(animation);
//
//					myGraphics->GetModelLoader().LoadAnimation(*skeletalModelComponentData.skeletalModelData, animationPath);
//					auto* animationClip = myGraphics->GetModelLoader().GetAnimationClip(animationPath);
//
//					if (animationClip != nullptr)
//					{
//						skeletalModelComponentData.skeletalModelData->myAnimationClipMap[animationClip->name] = animationClip;
//					}
//				}
//
//				eRenderLayers layer = eRenderLayers::Main;
//				if (someComponents.contains("renderLayer"))
//				{
//					layer = static_cast<eRenderLayers>(someComponents["renderLayer"]["layer"]);
//				}
//				myGraphics->GetRenderLayer(layer)->AddSkeletalModelDataIndex(myGraphics->GetSkeletalModelData().size() - 1);
//
//				//aScene.AddComponentToLast<SkeletalModelComponent>(&skeletalModelComponentData);
//				aScene.GetGameObjectManager().AddComponentToObject<SkeletalModelComponent>(gameObject, &skeletalModelComponentData);
//			}
//
//			// ---[BOXCOLLIDER COMPONENT]--- //
//			else if (component.key() == "box_collider")
//			{
//				auto worldMat = component.value()["world_transform"];
//
//
//				Matrix4x4f worldMatrix;
//				worldMatrix(1, 1) = worldMat["m00"];
//				worldMatrix(1, 2) = worldMat["m01"];
//				worldMatrix(1, 3) = worldMat["m02"];
//				worldMatrix(1, 4) = worldMat["m03"];
//				worldMatrix(2, 1) = worldMat["m10"];
//				worldMatrix(2, 2) = worldMat["m11"];
//				worldMatrix(2, 3) = worldMat["m12"];
//				worldMatrix(2, 4) = worldMat["m13"];
//				worldMatrix(3, 1) = worldMat["m20"];
//				worldMatrix(3, 2) = worldMat["m21"];
//				worldMatrix(3, 3) = worldMat["m22"];
//				worldMatrix(3, 4) = worldMat["m23"];
//				worldMatrix(4, 1) = worldMat["m30"];
//				worldMatrix(4, 2) = worldMat["m31"];
//				worldMatrix(4, 3) = worldMat["m32"];
//				worldMatrix(4, 4) = worldMat["m33"];
//
//
//				Vector3f offset;
//				offset.x = component.value()["offset"]["x"];
//				offset.y = component.value()["offset"]["y"];
//				offset.z = component.value()["offset"]["z"];
//
//				Vector3f size;
//				size.x = component.value()["size"]["x"];
//				size.y = component.value()["size"]["y"];
//				size.z = component.value()["size"]["z"];
//
//				Transform tm;
//				tm.SetMatrix(worldMatrix);
//				tm.TranslateLocal(offset);
//
//				bool isTrigger = component.value()["trigger"];
//
//				bool isStatic = gameObject->IsStatic();
//
//				KE::BoxColliderComponentData boxColliderData(*aScene.myCollisionHandler.CreateBox(tm, offset, size, gameObject->myLayer, isStatic));
//
//				boxColliderData.offset = offset;
//
//				boxColliderData.size = size;
//
//				boxColliderData.isTrigger = isTrigger;
//
//				boxColliderData.graphics = myGraphics;
//
//				aScene.AddComponentToLast<BoxColliderComponent>(&boxColliderData);
//			}
//
//			// ---[SPHERECOLLIDER COMPONENT]--- //
//			else if (component.key() == "sphere_collider")
//			{
//				Vector3f offset;
//				offset.x = component.value()["offset"]["x"];
//				offset.y = component.value()["offset"]["y"];
//				offset.z = component.value()["offset"]["z"];
//
//				Vector3f position = gameObject->myTransform.GetPosition();
//
//				Transform tm;
//
//				tm.SetPosition(position + offset);
//
//				float radius = component.value()["radius"];
//
//				bool trigger = component.value()["trigger"];
//
//				bool isStatic = gameObject->IsStatic();
//
//				KE::SphereColliderComponentData sphereColliderData(*aScene.myCollisionHandler.CreateSphere(tm, offset, radius, gameObject->myLayer, isStatic));
//
//				sphereColliderData.offset = offset;
//
//				sphereColliderData.radius = radius;
//
//				sphereColliderData.isTrigger = trigger;
//
//				sphereColliderData.graphics = myGraphics;
//
//				aScene.AddComponentToLast<SphereColliderComponent>(&sphereColliderData);
//			}
//
//			// ---[CAPSULE COLLIDER]---
//			else if (component.key() == "capsule_collider")
//			{
//				Vector3f offset;
//				offset.x = component.value()["offset"]["x"];
//				offset.y = component.value()["offset"]["y"];
//				offset.z = component.value()["offset"]["z"];
//
//				Vector3f position = gameObject->myTransform.GetPosition();
//
//				Transform tm;
//
//				tm.SetPosition(position + offset);
//
//				float radius = component.value()["radius"];
//
//				float length = component.value()["length"];
//
//				bool trigger = component.value()["trigger"];
//
//				bool isStatic = gameObject->IsStatic();
//
//				KE::CapsuleColliderData capsuleColData(*aScene.myCollisionHandler.CreateCapsule(tm, offset, radius, length / 2.0f, gameObject->myLayer, isStatic));
//
//				capsuleColData.offset = offset;
//
//				capsuleColData.length = length;
//
//				capsuleColData.radius = radius;
//
//				capsuleColData.isTrigger = trigger;
//
//				aScene.AddComponentToLast<CapsuleColliderComponent>(&capsuleColData);
//			}
//
//			// ---[MESH COLLIDER]---
//			else if (component.key() == "mesh_collider")
//			{
//				std::string modelPath = std::string("Data/Assets/") + std::string(component.value()["mesh"]);
//
//				KE::ModelData* data = myGraphics->CreateModelData(modelPath);
//
//				if (!data->myMeshList)
//				{
//					KE_ERROR("Mesh Collider failed to load, file %s", aScene.sceneName.c_str());
//					continue;
//				}
//
//				if (data->myMeshList->myMeshes.size() > 0)
//				{
//					KE::Mesh& mesh = data->myMeshList->myMeshes[0];
//					bool isStatic = true;
//
//					aScene.myCollisionHandler.CreateMeshCollider(
//						gameObject->myWorldSpaceTransform,
//						mesh.myVertices,
//						mesh.myIndices,
//						gameObject->myLayer,
//						isStatic
//					);
//				}
//			}
//
//			// ---[AUDIO COMPONENT]--- //
//			else if (component.key() == "audio")
//			{
//				AudioComponentData data{};
//
//				data.fileName = component.value()["fileName"];
//				data.shouldLoop = component.value()["shouldLoop"];
//				data.shouldPerformSpatialPlayback = component.value()["shouldPerformSpatialPlayback"];
//
//				aScene.AddComponentToLast<AudioComponent>(&data);
//			}
//
//			// ---[LIGHT COMPONENT]--- //
//			else if (component.key() == "directional_light")
//			{
//				auto go = aScene.gameObjectManager.GetGameObject(1);
//
//				if (!go->HasComponent<LightComponent>())
//				{
//					go->AddComponent<LightComponent>();
//					go->myTransform = gameObject->myTransform;
//
//					DirectionalLightData* data = static_cast<DirectionalLightData*>(myGraphics->GetDeferredLightManager().
//						CreateLightData(eLightType::Directional));
//
//					data->myColour = {
//						component.value()["color"]["r"],
//						component.value()["color"]["g"],
//						component.value()["color"]["b"]
//					};
//					data->myDirection = gameObject->myTransform.GetForward();
//					data->myDirectionalLightIntensity = static_cast<float>(component.value()["intensity"]);
//					data->myAmbientLightIntensity = 1.0f;
//
//					LightComponentData lightComponentData;
//					lightComponentData.myLightData = data;
//					lightComponentData.myLightType = eLightType::Directional;
//					lightComponentData.myLightTypeName = "Directional Light";
//
//					go->GetComponent<LightComponent>().SetData(&lightComponentData);
//				}
//				else
//				{
//					KE_ERROR("Scene already has a directional light, file %s", aScene.sceneName.c_str());
//				}
//				aScene.gameObjectManager.DestroyGameObject(gameObject->myID);
//			}
//
//			// ---[POINTLIGHT COMPONENT]--- //
//			else if (component.key() == "point_light")
//			{
//				PointLightData* data = static_cast<PointLightData*>(myGraphics->GetDeferredLightManager().
//					CreateLightData(eLightType::Point));
//
//				data->myColour = {
//					component.value()["color"]["r"],
//					component.value()["color"]["g"],
//					component.value()["color"]["b"]
//				};
//				data->myRange = static_cast<float>(component.value()["range"]);
//				data->myIntensity = static_cast<float>(component.value()["intensity"]);
//				data->myPosition = { 0.0f, 0.0f, 0.0f };
//				data->isActive = true;
//
//				LightComponentData lightComponentData;
//				lightComponentData.myLightData = data;
//				lightComponentData.myLightType = eLightType::Point;
//				lightComponentData.myLightTypeName = "Point Light";
//
//				aScene.AddComponentToLast<LightComponent>(&lightComponentData);
//			}
//
//			// ---[SPOTLIGHT COMPONENT]--- //
//			else if (component.key() == "spot_light")
//			{
//				SpotLightData* data = static_cast<SpotLightData*>(myGraphics->GetDeferredLightManager().CreateLightData(
//					eLightType::Spot));
//
//				data->myColour = {
//					component.value()["color"]["r"],
//					component.value()["color"]["g"],
//					component.value()["color"]["b"]
//				};
//				data->myRange = static_cast<float>(component.value()["range"]);
//				data->myIntensity = static_cast<float>(component.value()["intensity"]);
//				data->myPosition = { 0.0f, 0.0f, 0.0f };
//				data->myDirection = { 0.0f, 0.0f, 0.0f };
//				data->myOuterAngle = DegToRad(component.value()["outerAngle"]);
//				data->myInnerAngle = DegToRad(data->myOuterAngle - static_cast<float>(component.value()["innerAngle"]));
//				data->isActive = true;
//
//				LightComponentData lightComponentData;
//				lightComponentData.myLightData = data;
//				lightComponentData.myLightType = eLightType::Spot;
//				lightComponentData.myLightTypeName = "Spot Light";
//
//				aScene.AddComponentToLast<LightComponent>(&lightComponentData);
//			}
//
//			// ---[VFX COMPONENT]--- //
//			else if (component.key() == "vfx")
//			{
//				VFXComponentData data;
//				data.myVFXManager = &myGraphics->GetVFXManager();
//
//				for (auto& sqName : component.value()["sequence_names"])
//				{
//					data.myAutoPlay = component.value()["autoplay"];
//					data.myVFXNames.push_back(sqName);
//				}
//
//				aScene.GetGameObjectManager().AddComponentToObject<VFXComponent>(gameObject, &data);
//
//			}
//
//			// ---[SCENE TRANSITION TRIGGER]--- //
//			else if (component.key() == "scene_trigger")
//			{
//				SceneTransitionComponentData data;
//
//				data.sceneIndex = component.value()["sceneIndex"];
//
//				aScene.AddComponentToLast<SceneTransitionComponent>(&data);
//			}
//
//			// ---[SCRIPT!!!!]
//			else if (component.key() == "script")
//			{
//				ScriptComponentData data;
//
//				auto* scriptManager = KE_GLOBAL::blackboard.Get<ScriptManager>("scriptManager");
//				const std::string path = component.value()["scriptName"];
//				data.script = scriptManager->GetOrLoadScript(path);
//
//
//				aScene.AddComponentToLast<KE::ScriptComponent>(&data);
//			}
//
//			else if (component.key() == "decal")
//			{
//				DecalComponentData data;
//
//				const std::string& decalAlbedo = component.value()["material"]["albedo"];
//				const std::string& decalNormal = component.value()["material"]["normal"];
//				const std::string& decalMaterial = component.value()["material"]["material"];
//				const std::string& decalEffects = component.value()["material"]["effects"];
//
//				data.myDecalIndex = myGraphics->GetDecalManager().CreateDecal(
//					myGraphics->GetTextureLoader().GetCustomMaterial(
//						decalAlbedo,
//						decalNormal,
//						decalMaterial,
//						decalEffects
//					),
//					aScene.GetGameObjectManager().GetLatestGameObject()->myWorldSpaceTransform
//				);
//
//				auto renderLayerToUse = eRenderLayers::Main;
//				if (component.value().contains("renderLayer"))
//				{
//					int layer = component.value()["renderLayer"];
//					if (layer < static_cast<int>(eRenderLayers::Count) && layer >= 0)
//					{
//						renderLayerToUse = static_cast<eRenderLayers>(layer);
//					}
//				}
//				myGraphics->GetRenderLayer(renderLayerToUse)->AddDecalIndex(data.myDecalIndex);
//
//				aScene.AddComponentToLast<DecalComponent>(&data);
//			}
//
//			else if (component.key() == "shellTexturing")
//			{
//				auto& shaderLoader = myGraphics->GetShaderLoader();
//				auto& textureLoader = myGraphics->GetTextureLoader();
//				auto& modelLoader = myGraphics->GetModelLoader();
//
//				ShellModelData shellData;
//				ShellTexturingComponentData data = {
//					myGraphics->GetShellTexturedRenderer()->AddShellModel(shellData)
//				};
//
//				auto& val = component.value();
//				const std::string modelPath = std::string("Data/Assets/") + std::string(val["mesh"]);
//
//				auto& sAtr = data.shellModelData->attributes;
//				sAtr.totalHeight = val.contains("totalHeight") ? (float)val["totalHeight"] : sAtr.totalHeight;
//				sAtr.shellCount = val.contains("shellCount") ? (int)val["shellCount"] : sAtr.shellCount;
//				sAtr.thickness = val.contains("thickness") ? (float)val["thickness"] : sAtr.thickness;
//				sAtr.density = val.contains("density") ? (float)val["density"] : sAtr.density;
//				sAtr.noiseMin = val.contains("noiseMin") ? (float)val["noiseMin"] : sAtr.noiseMin;
//				sAtr.noiseMax = val.contains("noiseMax") ? (float)val["noiseMax"] : sAtr.noiseMax;
//				sAtr.aoExp = val.contains("aoExp") ? (float)val["aoExp"] : sAtr.aoExp;
//
//				sAtr.bottomColour[0] = val.contains("bottomColour") ? (float)val["bottomColour"]["r"] : sAtr.bottomColour[0];
//				sAtr.bottomColour[1] = val.contains("bottomColour") ? (float)val["bottomColour"]["g"] : sAtr.bottomColour[1];
//				sAtr.bottomColour[2] = val.contains("bottomColour") ? (float)val["bottomColour"]["b"] : sAtr.bottomColour[2];
//
//				sAtr.topColour[0] = val.contains("topColour") ? (float)val["topColour"]["r"] : sAtr.topColour[0];
//				sAtr.topColour[1] = val.contains("topColour") ? (float)val["topColour"]["g"] : sAtr.topColour[1];
//				sAtr.topColour[2] = val.contains("topColour") ? (float)val["topColour"]["b"] : sAtr.topColour[2];
//
//				auto& shellMD = data.shellModelData->modelData;
//				shellMD.myTransform = &gameObject->myTransform.GetMatrix();
//				auto& rrb = shellMD.myRenderResources.emplace_back();
//
//				rrb.myVertexShader = shaderLoader.GetVertexShader(SHADER_LOAD_PATH "ShellTexturing_VS.cso");
//				rrb.myPixelShader = shaderLoader.GetPixelShader(SHADER_LOAD_PATH "ShellTexturing_PS.cso");
//				rrb.myMaterial = textureLoader.GetDefaultMaterial();
//
//				shellMD.myMeshList = &modelLoader.Load(modelPath);
//				data.shellModelData->effectsRT = myGraphics->GetRenderTarget(10);
//
//				aScene.GetGameObjectManager().AddComponentToObject<ShellTexturingComponent>(gameObject, &data);
//			}
//
//#pragma endregion
//
//
//#pragma region GameComponents
//
//			else if (component.key() == "player")
//			{
//
//				playerIndex++;
//
//				const int boomerangID = -9999 + playerIndex;
//
//				P8::PlayerData data;
//
//				// UserData //
//				CharacterControllerUserData playerUserData = {};
//				playerUserData.gameObject = gameObject;
//				playerUserData.objType = CharacterControllerUserData::Type::Player;
//				playerUserData.team = playerIndex;
//
//				// PhysxController //
//				PhysxControllerData controllerData;
//				controllerData.stepOffset = 0.01f;
//
//
//				data.controller = aScene.myCollisionHandler.CreateController(controllerData, playerUserData);
//
//				auto& val = component.value();
//
//				// Player Unity Data //
//				data.maxSpeed = component.value()["maxSpeed"];
//				data.maxRotation = component.value()["maxRotation"];
//				data.movementSpeed = component.value()["movementSpeed"];
//				data.rotationSpeed = component.value()["rotationSpeed"];
//				data.boomerangGoID = boomerangID;
//				data.playerIndex = playerIndex;
//
//
//				if (val.contains("characterIndex"))
//				{
//					data.characterIndex = static_cast<int>(val["characterIndex"]);
//				}
//				int playerCharacterIndex = data.characterIndex;
//
//				aScene.AddComponentToLast<P8::Player>(&data);
//				P8::Player& playerComponent = aScene.GetGameObjectManager().GetLatestGameObject()->GetComponent<P8::Player>();
//
//				// ---Temporary way of adding a boomerang to the player--- //
//				{
//					const std::string objName = "Boomerang";
//					aScene.GetGameObjectManager().CreateGameObject(boomerangID, &objName, Transform(), false);
//
//					auto* obj = aScene.GetGameObjectManager().GetGameObject(boomerangID);
//					obj->myLayer = KE::Collision::Layers::Projectile;
//
//
//					obj->myTransform.SetScale({ 0.25f,0.25f,0.25f });
//					const float radius = 0.25f;
//					controllerData.halfHeight = 0.01f;
//					controllerData.radius = radius;
//
//					P8::BoomerangComponentData boomerangData;
//					P8::BoomerangPhysicsController* boomPhys = new P8::BoomerangPhysicsController();
//
//
//					CharacterControllerUserData boomUserData = {};
//					boomUserData.gameObject = obj;
//					boomUserData.objType = CharacterControllerUserData::Type::Boomerang;
//					boomUserData.team = playerIndex;
//
//					const auto& boomCtrl = boomPhys->Create(
//						aScene.myCollisionHandler.GetPhysics(),
//						aScene.myCollisionHandler.GetControllerManager(),
//						controllerData,
//						boomUserData
//					);
//
//					aScene.myCollisionHandler.RegisterPhysicsObject(boomCtrl);
//					boomerangData.controller = boomPhys;
//					boomerangData.player = &playerComponent;
//
//					aScene.AddComponentToLast<P8::BoomerangComponent>(&boomerangData);
//
//					//also add a model
//					{
//						std::string modelPath = std::string("Data/EngineAssets/Sphere.fbx");
//						KE::ModelComponentData modelComponentData;
//						modelComponentData.modelData = myGraphics->CreateModelData(modelPath);
//						modelComponentData.modelData->myTransform = &aScene.gameObjectManager.GetLatestGameObject()->myWorldSpaceTransform.GetMatrix();
//						modelComponentData.modelData->myRenderResources.back().myMaterial = myGraphics->GetTextureLoader().GetMaterialFromPath(
//							"../../EngineAssets/KittyEnginePattern"
//						);
//
//						myGraphics->GetRenderLayer(eRenderLayers::Main)->AddModelDataIndex(myGraphics->GetModelData().size() - 1);
//
//						aScene.AddComponentToLast<ModelComponent>(&modelComponentData);
//					}
//
//					//and a vfx component
//					{
//						KE::VFXComponentData vfxData;
//						vfxData.myVFXNames.push_back(std::format("BallVFX0{}", playerCharacterIndex));
//
//						vfxData.myVFXManager = &myGraphics->GetVFXManager();
//						aScene.AddComponentToLast<VFXComponent>(&vfxData);
//
//					}
//				}
//
//
//			}
//#pragma endregion
//		}
//	}
#pragma endregion

	void LevelImporter::AddComponents(KE::Scene* aCurrentScene, KE::GameObject& aGameObject, nlohmann::json& someComponents)
	{
		auto& GOManager = aCurrentScene->GetGameObjectManager();

		auto& collisionHandler = aCurrentScene->myCollisionHandler;

		for (auto& component : someComponents.items())
		{
#pragma region EngineComponents
			//---[CAMERA COMPONENT]--- //
			if (component.key() == "main_camera")
			{

				auto* camObj = GOManager.GetGameObject(0);
				camObj->myTransform = aGameObject.myTransform;


				GOManager.DestroyGameObject(aGameObject.myID);
			}



			// ---[MODEL COMPONENT]--- //
			else if (component.key() == "model")
			{
				std::string modelPath = std::string("Data/Assets/") + std::string(component.value()["path"]);

				KE::ModelComponentData modelComponentData;
				modelComponentData.modelData = myGraphics->CreateModelData(modelPath);
				modelComponentData.modelData->myTransform = &aGameObject.myWorldSpaceTransform.GetMatrix();

				eRenderLayers layer = eRenderLayers::Main;
				if (someComponents.contains("renderLayer"))
				{
					layer = static_cast<eRenderLayers>(someComponents["renderLayer"]["layer"]);
				}

				myGraphics->GetRenderLayer(layer)->AddModelDataIndex(myGraphics->GetModelData().size() - 1);

				aGameObject.AddComponent<ModelComponent>(&modelComponentData);

				if (modelPath.find("SM_WaterPlane") != std::string::npos)
				{
					for (auto& rr : modelComponentData.modelData->myRenderResources)
					{
						//std::cout << "\nSetting water shader!";
						rr.myPixelShader = myGraphics->GetShaderLoader().GetPixelShader(SHADER_LOAD_PATH "Water_PS.cso");
						rr.myVertexShader = myGraphics->GetShaderLoader().GetVertexShader(SHADER_LOAD_PATH "Water_VS.cso");
					}
				}

				if (someComponents.contains("overrideMaterial"))
				{
					auto& material = someComponents["overrideMaterial"];
					std::string materialName = material["materialName"];
					Vector3f albedo = {
						material["albedo"]["r"],
						material["albedo"]["g"],
						material["albedo"]["b"]
					};

					float roughness = material["roughness"];
					float metallic = material["metallic"];
					float emission = material["emission"];

					std::string albedoTexName = material["albedoTex"];

					DataMaterial mat;
					mat.myName = materialName;
					mat.albedo = albedo;
					mat.metallic = metallic;
					mat.roughness = roughness;
					mat.emission = emission;

					if (!albedoTexName.empty())
					{
						mat.albedoMap = std::format("Data/Assets/{}", albedoTexName);

						if (albedoTexName.ends_with("_c.dds"))
						{
							mat.albedoMap = std::format("Data/Assets/{}", albedoTexName);
							mat.normalMap = std::format("Data/Assets/{}", albedoTexName.substr(0, albedoTexName.size() - 6) + "_n.dds");
							mat.materialMap = std::format("Data/Assets/{}", albedoTexName.substr(0, albedoTexName.size() - 6) + "_m.dds");
							mat.effectsMap = std::format("Data/Assets/{}", albedoTexName.substr(0, albedoTexName.size() - 6) + "_fx.dds");
						}

					}

					DataMaterial* matToUse = myGraphics->GetTextureLoader().GetDataMaterial(mat);

					for (auto& rr : modelComponentData.modelData->myRenderResources)
					{
						rr.myMaterial = matToUse;
					}

			
				}
			}

			// ---[COOLER MODEL COMPONENT]--- //
			else if (component.key() == "skeletal_model")
			{
				std::string modelPath = std::string("Data/Assets/") + std::string(component.value()["path"]);

				KE::SkeletalModelComponentData skeletalModelComponentData;
				skeletalModelComponentData.skeletalModelData = myGraphics->CreateSkeletalModelData(modelPath);
				skeletalModelComponentData.skeletalModelData->myTransform = &aGameObject.myWorldSpaceTransform.GetMatrix();

				// TODO This should be handled properly by FBX Importer vvvvvvv
				//DirectX::XMVECTOR translation, rotation, scale;
				//DirectX::XMMatrixDecompose(&scale, &rotation, &translation, *skeletalModelComponentData.skeletalModelData->myTransform);
				//scale = DirectX::XMVectorScale(scale, 0.01f);
				//*skeletalModelComponentData.skeletalModelData->myTransform = DirectX::XMMatrixAffineTransformation(scale, DirectX::XMVectorZero(), rotation, translation);
				// TODO This should be handled properly by FBX Importer ^^^^^^^^

				std::vector<std::string> animations;
				auto& modelLoader = myGraphics->GetModelLoader();
				for (auto& animation : component.value()["animationPaths"])
				{
					std::string animationPath = std::string("Data/Assets/") + std::string(animation);
					animations.push_back(animationPath);
				}

				eRenderLayers layer = eRenderLayers::Main;
				if (someComponents.contains("renderLayer"))
				{
					layer = static_cast<eRenderLayers>(someComponents["renderLayer"]["layer"]);
				}
				myGraphics->GetRenderLayer(layer)->AddSkeletalModelDataIndex(myGraphics->GetSkeletalModelData().size() - 1);

				SkeletalModelComponent* skModel = aGameObject.AddComponent<SkeletalModelComponent>(&skeletalModelComponentData);
				skModel->GetAnimationPlayer().AssignAnimations(modelLoader, animations);
			}

			// ---[BOXCOLLIDER COMPONENT]--- //
			else if (component.key() == "box_collider")
			{
				auto worldMat = component.value()["world_transform"];


				Matrix4x4f worldMatrix;
				worldMatrix(1, 1) = worldMat["m00"];
				worldMatrix(1, 2) = worldMat["m01"];
				worldMatrix(1, 3) = worldMat["m02"];
				worldMatrix(1, 4) = worldMat["m03"];
				worldMatrix(2, 1) = worldMat["m10"];
				worldMatrix(2, 2) = worldMat["m11"];
				worldMatrix(2, 3) = worldMat["m12"];
				worldMatrix(2, 4) = worldMat["m13"];
				worldMatrix(3, 1) = worldMat["m20"];
				worldMatrix(3, 2) = worldMat["m21"];
				worldMatrix(3, 3) = worldMat["m22"];
				worldMatrix(3, 4) = worldMat["m23"];
				worldMatrix(4, 1) = worldMat["m30"];
				worldMatrix(4, 2) = worldMat["m31"];
				worldMatrix(4, 3) = worldMat["m32"];
				worldMatrix(4, 4) = worldMat["m33"];


				Vector3f offset;
				offset.x = component.value()["offset"]["x"];
				offset.y = component.value()["offset"]["y"];
				offset.z = component.value()["offset"]["z"];

				Vector3f size;
				size.x = component.value()["size"]["x"];
				size.y = component.value()["size"]["y"];
				size.z = component.value()["size"]["z"];

				Transform tm;
				tm.SetMatrix(worldMatrix);
				tm.TranslateLocal(offset);

				bool isTrigger = component.value()["trigger"];

				bool isStatic = aGameObject.IsStatic();

				KE::BoxColliderComponentData boxColliderData(
					*collisionHandler.CreateBox(tm, offset, size, aGameObject.myLayer, isStatic)
				);

				boxColliderData.offset = offset;

				boxColliderData.size = size;

				boxColliderData.isTrigger = isTrigger;

				boxColliderData.graphics = myGraphics;

				aGameObject.AddComponent<BoxColliderComponent>(&boxColliderData);
			}

			// ---[SPHERECOLLIDER COMPONENT]--- //
			else if (component.key() == "sphere_collider")
			{
				Vector3f offset;
				offset.x = component.value()["offset"]["x"];
				offset.y = component.value()["offset"]["y"];
				offset.z = component.value()["offset"]["z"];

				Vector3f position = aGameObject.myTransform.GetPosition();

				Transform tm;

				tm.SetPosition(position + offset);

				float radius = component.value()["radius"];

				bool trigger = component.value()["trigger"];

				bool isStatic = aGameObject.IsStatic();

				KE::SphereColliderComponentData sphereColliderData(*collisionHandler.CreateSphere(tm, offset, radius, aGameObject.myLayer, isStatic));

				sphereColliderData.offset = offset;

				sphereColliderData.radius = radius;

				sphereColliderData.isTrigger = trigger;

				sphereColliderData.graphics = myGraphics;

				aGameObject.AddComponent<SphereColliderComponent>(&sphereColliderData);
			}

			// ---[CAPSULE COLLIDER]---
			else if (component.key() == "capsule_collider")
			{
				Vector3f offset;
				offset.x = component.value()["offset"]["x"];
				offset.y = component.value()["offset"]["y"];
				offset.z = component.value()["offset"]["z"];

				Vector3f position = aGameObject.myTransform.GetPosition();

				Transform tm;

				tm.SetPosition(position + offset);

				float radius = component.value()["radius"];

				float length = component.value()["length"];

				bool trigger = component.value()["trigger"];

				bool isStatic = aGameObject.IsStatic();

				KE::CapsuleColliderData capsuleColData(*collisionHandler.CreateCapsule(tm, offset, radius, length / 2.0f, aGameObject.myLayer, isStatic));

				capsuleColData.offset = offset;

				capsuleColData.length = length;

				capsuleColData.radius = radius;

				capsuleColData.isTrigger = trigger;

				aGameObject.AddComponent<CapsuleColliderComponent>(&capsuleColData);
			}

			// ---[MESH COLLIDER]---
			else if (component.key() == "mesh_collider")
			{
				std::string modelPath = std::string("Data/Assets/") + std::string(component.value()["mesh"]);

				KE::ModelData* data = myGraphics->CreateModelData(modelPath);

				if (!data->myMeshList)
				{
					KE_ERROR("Mesh Collider failed to load, file %s", aCurrentScene->sceneName.c_str());
					continue;
				}

				if (data->myMeshList->myMeshes.size() > 0)
				{
					KE::Mesh& mesh = data->myMeshList->myMeshes[0];
					bool isStatic = true;

					*collisionHandler.CreateMeshCollider(
						aGameObject.myWorldSpaceTransform,
						mesh.myVertices,
						mesh.myIndices,
						aGameObject.myLayer,
						isStatic
					);
				}
			}

			// ---[LIGHT COMPONENT]--- //
			else if (component.key() == "directional_light")
			{
				auto go = GOManager.GetGameObject(1);

				if (!go->HasComponent<LightComponent>())
				{
					go->AddComponent<LightComponent>();
					go->myTransform = aGameObject.myTransform;

					DirectionalLightData* data = static_cast<DirectionalLightData*>(myGraphics->GetDeferredLightManager().
						CreateLightData(eLightType::Directional));

					data->myColour = {
						component.value()["color"]["r"],
						component.value()["color"]["g"],
						component.value()["color"]["b"]
					};

					data->myDirection = aGameObject.myTransform.GetForward();
					data->myDirectionalLightIntensity = static_cast<float>(component.value()["intensity"]);
					data->myAmbientLightIntensity = 1.0f;


					if (someComponents.contains("lightSettings"))
					{
						auto& lightSettings = someComponents["lightSettings"];
						data->myAmbientLightIntensity = lightSettings["ambientIntensity"];
					}


					LightComponentData lightComponentData;
					lightComponentData.myLightData = data;
					lightComponentData.myLightType = eLightType::Directional;
					lightComponentData.myLightTypeName = "Directional Light";

					go->GetComponent<LightComponent>().SetData(&lightComponentData);
				}
				else
				{
					KE_ERROR("Scene already has a directional light, file %s", aCurrentScene->sceneName.c_str());
				}

				GOManager.DestroyGameObject(aGameObject.myID);
			}

			// ---[POINTLIGHT COMPONENT]--- //
			else if (component.key() == "point_light")
			{
				auto* lightManager = &myGraphics->GetDeferredLightManager();
				PointLightData* data = static_cast<PointLightData*>(lightManager->CreateLightData(eLightType::Point));

				data->myColour = {
					component.value()["color"]["r"],
					component.value()["color"]["g"],
					component.value()["color"]["b"]
				};
				data->myRange = static_cast<float>(component.value()["range"]);
				data->myIntensity = static_cast<float>(component.value()["intensity"]);
				data->myPosition = { 0.0f, 0.0f, 0.0f };
				data->isActive = true;

				LightComponentData lightComponentData;
				lightComponentData.myLightData = data;
				lightComponentData.myLightType = eLightType::Point;
				lightComponentData.myLightTypeName = "Point Light";
				lightComponentData.myLightManager = lightManager;

				aGameObject.AddComponent<LightComponent>(&lightComponentData);
			}

			// ---[SPOTLIGHT COMPONENT]--- //
			else if (component.key() == "spot_light")
			{
				auto* lightManager = &myGraphics->GetDeferredLightManager();
				SpotLightData* data = static_cast<SpotLightData*>(lightManager->CreateLightData(eLightType::Spot));

				data->myColour = {
					component.value()["color"]["r"],
					component.value()["color"]["g"],
					component.value()["color"]["b"]
				};
				data->myRange = static_cast<float>(component.value()["range"]);
				data->myIntensity = static_cast<float>(component.value()["intensity"]);
				data->myPosition = { 0.0f, 0.0f, 0.0f };
				data->myDirection = { 0.0f, 0.0f, 0.0f };
				data->myOuterAngle = DegToRad(component.value()["outerAngle"]);
				data->myInnerAngle = DegToRad(data->myOuterAngle - static_cast<float>(component.value()["innerAngle"]));
				data->isActive = true;

				LightComponentData lightComponentData;
				lightComponentData.myLightData = data;
				lightComponentData.myLightType = eLightType::Spot;
				lightComponentData.myLightTypeName = "Spot Light";
				lightComponentData.myLightManager = lightManager;

				aGameObject.AddComponent<LightComponent>(&lightComponentData);
			}

			// ---[VFX COMPONENT]--- //
			else if (component.key() == "vfx")
			{
				VFXComponentData data;
				data.myVFXManager = &myGraphics->GetVFXManager();

				for (auto& sqName : component.value()["sequence_names"])
				{
					data.myAutoPlay = component.value()["autoplay"];
					data.myVFXNames.push_back(sqName);
				}

				aGameObject.AddComponent<VFXComponent>(&data);
			}

			// ---[SCENE TRANSITION TRIGGER]--- //
			else if (component.key() == "scene_trigger")
			{
				SceneTransitionComponentData data;

				data.sceneIndex = component.value()["sceneIndex"];

				aGameObject.AddComponent<SceneTransitionComponent>(&data);
			}

			// ---[SCRIPT!!!!]
			else if (component.key() == "script")
			{
				ScriptComponentData data;

				auto* scriptManager = KE_GLOBAL::blackboard.Get<ScriptManager>("scriptManager");
				const std::string path = component.value()["scriptName"];
				data.script = scriptManager->GetOrLoadScript(path);

				aGameObject.AddComponent<KE::ScriptComponent>(&data);
			}

			else if (component.key() == "decal")
			{
				DecalComponentData data;

				const std::string& decalAlbedo = component.value()["material"]["albedo"];
				const std::string& decalNormal = component.value()["material"]["normal"];
				const std::string& decalMaterial = component.value()["material"]["material"];
				const std::string& decalEffects = component.value()["material"]["effects"];

				data.myDecalIndex = myGraphics->GetDecalManager().CreateDecal(
					myGraphics->GetTextureLoader().GetCustomMaterial(
						decalAlbedo,
						decalNormal,
						decalMaterial,
						decalEffects
					),
					aGameObject.myWorldSpaceTransform
				);

				auto renderLayerToUse = eRenderLayers::Main;
				if (component.value().contains("renderLayer"))
				{
					int layer = component.value()["renderLayer"];
					if (layer < static_cast<int>(eRenderLayers::Count) && layer >= 0)
					{
						renderLayerToUse = static_cast<eRenderLayers>(layer);
					}
				}
				myGraphics->GetRenderLayer(renderLayerToUse)->AddDecalIndex(data.myDecalIndex);

				aGameObject.AddComponent<DecalComponent>(&data);
			}

			else if (component.key() == "shellTexturing")
			{
				auto& shaderLoader = myGraphics->GetShaderLoader();
				auto& textureLoader = myGraphics->GetTextureLoader();
				auto& modelLoader = myGraphics->GetModelLoader();

				ShellModelData shellData;
				ShellTexturingComponentData data = {
					myGraphics->GetShellTexturedRenderer()->AddShellModel(shellData)
				};

				auto& val = component.value();
				const std::string modelPath = std::string("Data/Assets/") + std::string(val["mesh"]);

				auto& sAtr = data.shellModelData->attributes;
				sAtr.totalHeight = val.contains("totalHeight") ? (float)val["totalHeight"] : sAtr.totalHeight;
				sAtr.shellCount = val.contains("shellCount") ? (int)val["shellCount"] : sAtr.shellCount;
				sAtr.thickness = val.contains("thickness") ? (float)val["thickness"] : sAtr.thickness;
				sAtr.density = val.contains("density") ? (float)val["density"] : sAtr.density;
				sAtr.noiseMin = val.contains("noiseMin") ? (float)val["noiseMin"] : sAtr.noiseMin;
				sAtr.noiseMax = val.contains("noiseMax") ? (float)val["noiseMax"] : sAtr.noiseMax;
				sAtr.aoExp = val.contains("aoExp") ? (float)val["aoExp"] : sAtr.aoExp;

				sAtr.bottomColour[0] = val.contains("bottomColour") ? (float)val["bottomColour"]["r"] : sAtr.bottomColour[0];
				sAtr.bottomColour[1] = val.contains("bottomColour") ? (float)val["bottomColour"]["g"] : sAtr.bottomColour[1];
				sAtr.bottomColour[2] = val.contains("bottomColour") ? (float)val["bottomColour"]["b"] : sAtr.bottomColour[2];

				sAtr.topColour[0] = val.contains("topColour") ? (float)val["topColour"]["r"] : sAtr.topColour[0];
				sAtr.topColour[1] = val.contains("topColour") ? (float)val["topColour"]["g"] : sAtr.topColour[1];
				sAtr.topColour[2] = val.contains("topColour") ? (float)val["topColour"]["b"] : sAtr.topColour[2];

				

				auto& shellMD = data.shellModelData->modelData;
				shellMD.myTransform = &aGameObject.myTransform.GetMatrix();
				auto& rrb = shellMD.myRenderResources.emplace_back();

				rrb.myVertexShader = shaderLoader.GetVertexShader(SHADER_LOAD_PATH "ShellTexturing_VS.cso");
				rrb.myPixelShader = shaderLoader.GetPixelShader(SHADER_LOAD_PATH "ShellTexturing_PS.cso");
				rrb.myMaterial = textureLoader.GetDefaultMaterial();

				shellMD.myMeshList = &modelLoader.Load(modelPath);
				data.shellModelData->effectsRT = myGraphics->GetRenderTarget(10);

				aGameObject.AddComponent<ShellTexturingComponent>(&data);
			}

#pragma endregion

#pragma region GameComponents
			else if (component.key() == "player")
			{
				const int boomerangID = aCurrentScene->gameObjectManager.GenerateUniqueID();

				P8::PlayerData data;

				// UserData //
				CharacterControllerUserData playerUserData = {};
				playerUserData.gameObject = &aGameObject;
				playerUserData.objType = CharacterControllerUserData::Type::Player;
				playerUserData.team = playerIndex;

				// PhysxController //
				PhysxControllerData controllerData;
				controllerData.stepOffset = 0.01f;
				controllerData.radius = 0.2f;

				data.controller = aCurrentScene->myCollisionHandler.CreateController(controllerData, playerUserData);

				auto& val = component.value();

				// Player Unity Data //
				data.maxSpeed = component.value()["maxSpeed"];
				data.maxRotation = component.value()["maxRotation"];
				data.movementSpeed = component.value()["movementSpeed"];
				data.rotationSpeed = component.value()["rotationSpeed"];
				data.boomerangGoID = boomerangID;

				if (val.contains("characterIndex"))
				{
					data.characterIndex = static_cast<int>(val["characterIndex"]);
				}
				
				P8::Player& playerComponent = *aGameObject.AddComponent<P8::Player>(&data);
			}

			else if (component.key() == "boomerang")
			{
				aGameObject.myLayer = KE::Collision::Layers::Projectile;

				const float radius = 0.05f;
				PhysxControllerData controllerData;
				//controllerData.stepOffset = 0.01f;
				controllerData.halfHeight = 0.20f;
				controllerData.radius = radius;
				controllerData.skinWidth = 0.0001f;




				P8::BoomerangComponentData boomerangData;
				P8::BoomerangPhysicsController* boomPhys = new P8::BoomerangPhysicsController();


				CharacterControllerUserData boomUserData = {};
				boomUserData.gameObject = &aGameObject;
				boomUserData.objType = CharacterControllerUserData::Type::Boomerang;
				boomUserData.team = playerIndex;

				const auto& boomCtrl = boomPhys->Create(
					collisionHandler.GetPhysics(),
					collisionHandler.GetControllerManager(),
					controllerData,
					boomUserData
				);
							

				collisionHandler.RegisterPhysicsObject(boomCtrl);
				boomerangData.controller = boomPhys;

				auto* boomC = aGameObject.AddComponent<P8::BoomerangComponent>(&boomerangData);
				boomC->SetSize(0.25f);

				////also add a model
				//{
				//	std::string modelPath = std::string("Data/EngineAssets/Sphere.fbx");
				//	KE::ModelComponentData modelComponentData;
				//	modelComponentData.modelData = myGraphics->CreateModelData(modelPath);
				//	modelComponentData.modelData->myTransform = &aGameObject.myWorldSpaceTransform.GetMatrix();
				//	modelComponentData.modelData->myRenderResources.back().myMaterial = myGraphics->GetTextureLoader().GetMaterialFromPath(
				//		"../../EngineAssets/KittyEnginePattern"
				//	);

				//	myGraphics->GetRenderLayer(eRenderLayers::Main)->AddModelDataIndex(myGraphics->GetModelData().size() - 1);

				//	boomerangObj->AddComponent<ModelComponent>(&modelComponentData);
				//}

				////and a vfx component
				//{
				//	KE::VFXComponentData vfxData;
				//	vfxData.myVFXNames.push_back(std::format("BallVFX0{}", playerCharacterIndex));

				//	vfxData.myVFXManager = &myGraphics->GetVFXManager();
				//	boomerangObj->AddComponent<VFXComponent>(&vfxData);
				//}
			}

			if (component.key() == "cameraOverride")
			{
				auto* camObj = GOManager.GetGameObject(KE::eMainCamera);

				auto& camComp = camObj->GetComponent<CameraComponent>();

				//"aspectRatio": 0
				//"projectionType": 1
				//"FOV": 22.0
				//"ortographicWidth": 5.8
				//"nearPlane": 0.3
				//"farPlane": 1000.0
				//"disableActionCamera": true



				int aspectRatio = component.value()["aspectRatio"];
				int projType = component.value()["projectionType"];
				float fov = component.value()["FOV"];
				float ortographicWidth = component.value()["ortographicWidth"];
				float nearPlane = component.value()["nearPlane"];
				float farPlane = component.value()["farPlane"];
				bool disableActionCamera = component.value()["disableActionCamera"];

				float aspect = aspectRatio == 0 ? 16.0f / 9.0f : 4.0f / 3.0f;

				switch(projType)
				{
					case static_cast<int>(KE::ProjectionType::Perspective):
					{
						camComp.GetCamera()->SetPerspective(
							static_cast<float>(myGraphics->GetRenderWidth()),
							static_cast<float>(myGraphics->GetRenderHeight()),
							fov * KE::DegToRadImmediate,
							nearPlane,
							farPlane
						);
					}
					case static_cast<int>(KE::ProjectionType::Orthographic):
					{
						camComp.GetCamera()->SetOrthographic(
							ortographicWidth,
							ortographicWidth / aspect,
							nearPlane,
							farPlane
						);		
					}
				}

				if (disableActionCamera)
				{
					if (P8::ActionCameraComponent* actionComp; camObj->TryGetComponent<P8::ActionCameraComponent>(actionComp))
					{
						actionComp->SetActive(false);
					}
				}

			}

			else if (component.key() == "spawnPoint")
			{
				int index = component.value().contains("index") ? (int)component.value()["index"] : -1;
				aGameObject.AddComponent<P8::SpawnPointComponent>(&index);
			}

			else if (component.key() == "powerupPickup")
			{
				aGameObject.AddComponent<P8::PowerupPickupComponent>();
			}

			else if (component.key() == "powerupArea")
			{
				P8::PowerupAreaData data;
				data.min = {
					component.value()["min"]["x"],
					component.value()["min"]["y"],
					component.value()["min"]["z"]
				};

				data.max = {
					component.value()["max"]["x"],
					component.value()["max"]["y"],
					component.value()["max"]["z"]
				};

				aGameObject.AddComponent<P8::PowerupAreaComponent>(&data);
			}

			// [ Moving Object ] //
			else if (component.key() == "movingObject")
			{
				P8::MovingObjectData data;
				data.timeToTarget = component.value()["travelTime"];
				data.endPosition.x = component.value()["endPosition"]["x"];
				data.endPosition.y = component.value()["endPosition"]["y"];
				data.endPosition.z = component.value()["endPosition"]["z"];

				aGameObject.AddComponent<P8::MovingObject>(&data);
			}

			// [ Moving Object - TRIGGER] //
			else if (component.key() == "movingObjectTrigger")
			{
				P8::MovingObjectTriggerData data;
				

				nlohmann::json objs = component.value()["movingObjects"];

				for (auto& obj : objs)
				{
					int id = obj;
					data.movingObjects.push_back(id);
				}

				aGameObject.AddComponent<P8::MovingObjectTrigger>(&data);
			}

			// Portal //
			else if (component.key() == "portal")
			{
				P8::PortalData data;

				data.linkedPortalID = component.value()["linkedPortal"];

				aGameObject.AddComponent<P8::Portal>(&data);
			}

			// Water Plane //
			else if (component.key() == "waterPlane")
			{
				P8::WaterBufferData data;

				auto& val = component.value();

				data.waterFogColour[0] = val.contains("fogColour") ? (float)val["fogColour"]["r"] : data.waterFogColour[0];
				data.waterFogColour[1] = val.contains("fogColour") ? (float)val["fogColour"]["g"] : data.waterFogColour[1];
				data.waterFogColour[2] = val.contains("fogColour") ? (float)val["fogColour"]["b"] : data.waterFogColour[2];
				data.waterFogDensity = val.contains("fogDensity") ? (float)val["fogDensity"] : data.waterFogDensity;

				data.causticColour[0] = val.contains("causticColour") ? (float)val["causticColour"]["r"] : data.causticColour[0];
				data.causticColour[1] = val.contains("causticColour") ? (float)val["causticColour"]["g"] : data.causticColour[1];
				data.causticColour[2] = val.contains("causticColour") ? (float)val["causticColour"]["b"] : data.causticColour[2];
				data.causticStrength = val.contains("causticStrength") ? (float)val["causticStrength"] : data.causticStrength;

				data.waterFoamColour[0] = val.contains("foamColour") ? (float)val["foamColour"]["r"] : data.waterFoamColour[0];
				data.waterFoamColour[1] = val.contains("foamColour") ? (float)val["foamColour"]["g"] : data.waterFoamColour[1];
				data.waterFoamColour[2] = val.contains("foamColour") ? (float)val["foamColour"]["b"] : data.waterFoamColour[2];
				data.waterFoamStrength = val.contains("foamStrength") ? (float)val["foamStrength"] : data.waterFoamStrength;

				aGameObject.AddComponent<P8::WaterPlane>(&data);
			}

			// Battle Area //
			else if (component.key() == "battleArea")
			{
				P8::BattleAreaData data;

				data.shouldShrink = component.value()["shrink"];
				data.shrinkDelay = component.value()["shrinkDelay"];
				data.shrinkDuration = component.value()["shrinkDuration"];

				aGameObject.AddComponent<P8::BattleArea>(&data);
			}

			else if (component.key() == "cameraAngleOverride")
			{
				KE::GameObject* mainCamera = aCurrentScene->GetGameObjectManager().GetGameObject(ReservedGameObjects::eMainCamera);

				P8::ActionCameraComponent* comp = &aCurrentScene->GetGameObjectManager().GetGameObjectWithComponent<P8::ActionCameraComponent>()->GetComponent<P8::ActionCameraComponent>();

				if (comp)
				{
					auto& settings = comp->GetSettings();

					if (component.value().contains("direction"))
					{
						settings.cameraDirection.x = component.value()["direction"]["x"];
						settings.cameraDirection.y = component.value()["direction"]["y"];
						settings.cameraDirection.z = component.value()["direction"]["z"];
					}
				}
			}

#pragma endregion
		}
	}

	bool LevelImporter::LoadGameSystemData(Scene& aScene, std::string& aFileName)
	{
		if (auto* gameSystems = aScene.GetGameObjectManager().GetGameObject(eGameSystemManager))
		{
			return true;
		}

		return false;
	}

	void LevelImporter::AddGameSystemComponents(KE::GameObject& aGameObject)
	{
		aGameObject.AddComponent<P8::TimeManager>();
		aGameObject.AddComponent<P8::LobbySystem>();
		aGameObject.AddComponent<P8::BallManager>();
		aGameObject.AddComponent<P8::PowerupManager>();
		aGameObject.AddComponent<P8::GameManager>();
		//aGameObject.AddComponent<P8::BattleArea>();
	}
}

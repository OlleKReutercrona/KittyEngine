#pragma once

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <any>
#include <Windows.h>


#include <External/Include/imgui/imgui.h>

#include "Editor/Source/EditorUtils.h"
#include "Editor/Source/EditorGraphics.h"

#include <vector>
#include <unordered_map>

#include "Engine/Source/ComponentSystem/Components/Graphics/ShellTexturingComponent.h"
#include "Engine/Source/ComponentSystem/Components/Graphics/DecalComponent.h"


struct ID3D11Device;
struct ID3D11DeviceContext;

namespace KE
{
	class VFXComponent;
	class GameObject;
	class Component;
	class ModelComponent;
	class SkeletalModelComponent;
	class LightComponent;
	class AudioComponent;
	class CameraComponent;
	class BoxColliderComponent;
	class SphereColliderComponent;
	class PostProcessingComponent;
	class DebugRenderer;
	class RenderTarget;
	class Camera;
	class Navmesh;

	struct Texture;
	struct Material;
	struct VFXSequence;
}

namespace P8
{
	class Player;
	class ActionCameraComponent;
	class GameManager;
	class LobbySystem;
}

class Transform;

namespace KE_EDITOR
{
	

	struct ImGuiWindowData
	{
		float aTopLeftX;
		float aTopLeftY;
		float aBottomRightX;
		float aBottomRightY;

		int aCameraIndex;
	};

	//struct WindowInstanceData
	//{
	//	void* myData;
	//	void Open(void* aData) { myData = aData; }
	//	void Close() { myData = nullptr; }

	//	bool IsOpen() { return myData != nullptr; }
	//};
	//		

	//struct WindowOpenData
	//{
	//	std::unordered_map<std::string, WindowInstanceData> myWindowData = {};

	//	bool IsOpen(const std::string& aWindowName) { return myWindowData[aWindowName].IsOpen(); }
	//	void RegisterWindow(const std::string& aWindowName) { myWindowData[aWindowName] = {}; }
	//};

	struct ArbitraryGizmo
	{
		enum class Type
		{
			VECTOR3,
			VECTOR4,
			TRANSFORM,
			NONE
		};

		Type myType = Type::NONE;
		std::vector<void*> myDatas = { nullptr };
		std::vector<Transform> myTransforms = {};

		ArbitraryGizmo() = default;
		ArbitraryGizmo(Type aType, void* aData) : myType(aType), myDatas({ aData }) {}

		void Clear() { myDatas.clear(); myTransforms.clear(); myType = Type::NONE; }

		void Attach(bool aKeepOld, Type aType, void* aData, const Transform& aTransform)
		{
			if (!aKeepOld) { Clear(); }
			if (myType != aType) { Clear(); }

			myType = aType;
			
			//make sure we don't add the same data twice
			for (auto& data : myDatas)
			{
				if (data == aData) { return; }
			}

			myDatas.push_back(aData);
			myTransforms.push_back(aTransform);
		}
	};

	struct CustomHandle
	{		
		EditorIcon* myIcon = nullptr;
		ArbitraryGizmo::Type myGizmoType = ArbitraryGizmo::Type::NONE;
		void* myData = nullptr;
		Transform myTransform;
		std::string myText = "";
	};

	enum class DragDropType
	{
		Texture,
		Material,
		Model,
		Audio,

		Count
	};

	struct DragDropData
	{
		std::any myData;
		DragDropType myType = DragDropType::Count;
	};

	class Editor;

	class ImGuiHandler
	{
		friend class Editor;
		friend class EditorTexture;
		friend class EditorAssetBrowser;
	private:
		inline static KE::DebugRenderer* myDebugRenderer = nullptr;
		inline static std::vector<KittyFlags> flagStack = {};
		inline static Editor* editor = nullptr;
		inline static std::unordered_map<ImGuiID, bool> windowDockedMap = {};

		inline static std::unordered_map<std::string, std::string> beautifiedComponentNameMap = {};

		bool WillWindowDock(ImGuiID aWindowID);
		void SetWindowDock(ImGuiID aWindowID, bool aCondition) { windowDockedMap[aWindowID] = aCondition; }

		inline static KE::GameObject* inspectedGameObject = nullptr;

		//inline static std::vector<ArbitraryGizmo> gizmos = {};

		inline static DragDropData dragDropData = {};

		inline static int vfxEditorActiveSequenceIndex = -1;

		//inline static WindowOpenData myWindowOpenData = {};
		
	public:
		inline static std::unordered_map<int, ImGuiWindowData> windowData = {};
		inline static ArbitraryGizmo arbitraryGizmo = {};
		inline static std::vector<CustomHandle> customHandles = {};

		static void Init(
			HWND hwnd,
			ID3D11Device* aDevice,
			ID3D11DeviceContext* aDeviceContext,
			Editor* aEditor
		);

		inline static DragDropData& GetDragDropData() { return dragDropData; }

		inline static void RegenerateGameObjectCache() { /*gameObjectCacheDirty = true;*/ }

		inline static void DisplayProgressBar(float aProgress);

		static void BeginFrame();
		static void EndFrame();
		static void Shutdown();
		//

		static bool TrySameLine(float aWidth = 0.0f, float aSpacing = 16.0f);

		static bool TryBeginWindow(const char* aString, bool* aCondition = nullptr, ImGuiWindowFlags aWindowFlags = 0);

		//

		static bool PerformFlagsBegin(const char* aString, KittyFlags aFlags, bool* aOpenFlag = nullptr, int aInputFlags = 0);
		static void PerformFlagsEnd();

		static void DisplayMainMenuBar();
		static void BeginFullscreenWindow();

		//

		// Object display
		static void DisplaySkeleton(KE::GameObject* aGameObject, KittyFlags aFlags = 0);
		static void DisplayNavmesh(KE::Navmesh* aNavmesh, KE::Camera* aCamera, ImVec4 aViewportBounds);
			
		//static void CreateGameObjectCache(const std::vector<KE::GameObject*>& aGameObjectList);
		//static void CalculateGameObjectFilter(const std::vector<KE::GameObject*>& aGameObjectList, const std::string& aNameFilter);
		//static void DisplayObjectList(const std::vector<KE::GameObject*>& aGameObjectList, KittyFlags aFlags = 0);
		//static void DisplayObjectListEntry(KE::GameObject* aGameObject);

		//static void DisplayGameObject(KE::GameObject* aGameObject, KittyFlags aFlags = 0);
		static void DisplayTransform(Transform* aTransform, KittyFlags aFlags = 0);
		
		static bool DisplayCustomHandle(CustomHandle& aCustomHandle, KE::Camera* aCamera, ImVec4 aViewportBounds);
		static bool DisplayCustomHandles(KE::Camera* aCamera, ImVec4 aViewportBounds);
		static void DisplayBoundingBox(const Vector3f& aWorldMin, const Vector3f& aWorldMax, KE::Camera* aCamera, ImVec4 aViewportBounds);
		static bool DisplayArbitraryGizmo(ArbitraryGizmo* aGizmo, KE::Camera* aCamera, ImVec4 aViewportBounds);
		static void DisplayGizmo(Transform* aTransform, KE::Camera* aCamera, ImVec4 aViewportBounds, void* anOutDeltaMatrix = nullptr);

		// Component Display
		static bool BeginDisplayComponent(const std::string& aComponentName, KE::Component* aComponent);
		static void EndDisplayComponent();

		static void ComponentDisplayDispatch(KE::Component* aComponent);
		static void DisplayDecalComponent(KE::DecalComponent* aDecalComponent, KittyFlags aFlags);
		static void DisplayModelComponent(KE::ModelComponent* aModelComponent, KittyFlags aFlags = 0);
		static bool DisplayComponentHeader(const std::string& aComponentName, KE::Component* aComponent);
		static void DisplaySkeletalModelComponent(KE::SkeletalModelComponent* aSkeletalModelComponent, KittyFlags aFlags = 0);
		static void DisplayShadowRendering(KE::DeferredLightManager* aLightManager);
		static void DisplayLightComponent(KE::LightComponent* aLightComponent, KittyFlags aFlags = 0);
		//static void DisplayAudioComponent(KE::AudioComponent* aAudioComponent, KittyFlags aFlags = 0);
		static void DisplayCameraComponent(KE::CameraComponent* aCameraComponent, KittyFlags aFlags = 0);
		static void DisplayBoxColliderComponent(KE::BoxColliderComponent* aBoxColliderComponent, KittyFlags aFlags = 0);
		static void DisplaySphereColliderComponent(KE::SphereColliderComponent* aSphereColliderComponent, KittyFlags aFlags = 0);
		static void DisplayPostProcessingComponent(KE::PostProcessingComponent* aPostProcessingComponent, KittyFlags aFlags = 0);
		static void DisplayVFXComponent(KE::VFXComponent* aVFXComponent, KittyFlags aFlags = 0);

		static void DisplayPlayerComponent(P8::Player* aPlayer, KittyFlags aFlags = 0);
		static void DisplayShellTexturingComponent(KE::ShellTexturingComponent* aShellComponent, KittyFlags aFlags);
		static void DisplayActionCameraComponent(P8::ActionCameraComponent* aComponent, KittyFlags aFlags = 0);
		static void DisplayGameManager(P8::GameManager* aComponent, KittyFlags aFlags);
		static void DisplayLobbySystem(P8::LobbySystem* aComponent, KittyFlags aFlags);
		// Resource Display

		static void DisplayParticleEmitter(KE::ParticleEmitter* anEmitter, KittyFlags aFlags = 0);
		static void DisplayPostProcessing(KE::PostProcessing* aPostProcessing, KittyFlags aFlags = 0);
		static void DisplayShaderList();
		static void DisplayShader(KE::Shader** aShader, bool aDragSource, bool aDragTarget);
		static void DisplayModelData(KE::ModelData* aModelData, KittyFlags aFlags = 0);
		static void DisplaySkeletalModelData(KE::SkeletalModelData* aModelData, KittyFlags aFlags = 0);
		static void DisplayTexture(KE::Texture** aTexture, ImVec2 aSize, KittyFlags aFlags = 0);
		static void DisplayMaterial(KE::Material* aMaterial, KittyFlags aFlags = 0);
		
		static bool DisplayViewport(const std::string& aViewportName, KE::RenderTarget* aRenderTarget, int anIndex, KE::Camera* aCamera, KittyFlags aFlags = 0);

		static void DisplayVFXSequence(KittyFlags aFlags = 0);

		static std::string BeautifyComponentName(const std::string& aUGLYName);

		//

		static void EditTextStyling(KE::TextStyling& aTextStyling)
		{
			ImGui::ColorEdit4("Color", &aTextStyling.text.colour.x);

			DisplayEnumSlider(
				"Horizontal Align",
				&aTextStyling.text.horizontalAlign,
				KE::TextAlign::RightOrBottom
			);

			ImGui::ColorEdit4("Stroke Colour", &aTextStyling.stroke.colour.x);
			ImGui::DragFloat("Stroke Width", &aTextStyling.stroke.width);
			ImGui::DragFloat("Stroke Softness", &aTextStyling.stroke.softness);


		}

		template <typename EnumType>
		inline static void DisplayEnumSlider(const char* aLabel, int* aStep, EnumType aMax)
		{
			ImGui::SliderInt(aLabel, aStep, 0, static_cast<int>(aMax)-1, EnumToString(static_cast<EnumType>(*aStep)));
		}

		template <typename EnumType>
		inline static void DisplayEnumSlider(const char* aLabel, EnumType* aStep, EnumType aMax)
		{
			ImGui::SliderInt(aLabel, (int*)(aStep), 0, static_cast<int>(aMax)-1, EnumToString(static_cast<EnumType>(*aStep)));
		}
	};

	//struct ComponentInspectionHelper
	//{
	//	bool opened = false;
	//	ComponentInspectionHelper(const std::string& aComponentName, KE::Component* aComponent)
	//	{
	//		opened = ImGuiHandler::BeginDisplayComponent(aComponentName, aComponent);
	//		if (!opened) { ImGuiHandler::EndDisplayComponent(); }
	//	}

	//	~ComponentInspectionHelper()
	//	{
	//		if(opened) { ImGuiHandler::EndDisplayComponent(); }
	//	}

	//	bool IsOpen() const { return opened; }
	//};

}

#define COMPONENT_NAME(component) (KE_EDITOR::ImGuiHandler::BeautifyComponentName(typeid(*(component)).name()))

#define BEGIN_DISPLAY_COMPONENT(component) if (!ImGuiHandler::BeginDisplayComponent(COMPONENT_NAME(component), component)) { return; }
#define END_DISPLAY_COMPONENT() ImGuiHandler::EndDisplayComponent();
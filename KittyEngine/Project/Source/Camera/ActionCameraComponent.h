#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include "Engine/Source/Utility/EventSystem.h"

namespace KE
{
	class GameObject;
	class DebugRenderer;
	class CameraComponent;
}

namespace ES
{
	struct Event;
}

namespace P8 
{
	struct CameraSettingsData
	{
		float minDistanceFromGround = 15.0f;
		float maxDistanceFromGround = 100.0f;
		float currentDistanceFromGround = 40.0f;
		float outerBorderDistancePercentage = 10.0f; // Percentage from screen dimension borders
		float innerBorderDistancePercentage = 10.0f; // Percentage from screen dimension borders
		float horizontalMovementSpeed = 1.0f;
		float verticalMovementSpeed = 2.5f;


		float cameraFOV = 45;
		Vector3f cameraDirection = { 65.0f, 45.0f, 0.0f };
	};

	class ActionCameraComponent : public KE::Component, public ES::IObserver
	{
		KE_EDITOR_FRIEND
	public:
		ActionCameraComponent(KE::GameObject& aGameObject);
		~ActionCameraComponent();

		void Awake() override;

		void LateUpdate() override;

		void SetData(void* someData) override;

		void AddTarget(const KE::GameObject& aTarget);
		void RemoveTarget(const KE::GameObject& aTarget);

		inline CameraSettingsData& GetSettings() { return mySettings; }

		// Inherited via IObserver
		void OnReceiveEvent(ES::Event& aEvent) override;
		void OnInit() override;
		void OnDestroy() override;
	private:
		Vector3f CalculateTargetPosition();

		Vector3f CalculateMovement(const Vector3f& aTargetPosition);

		Vector3f CalculateAndApplyDimensions(const Vector3f& aPossiblePosition);

		Vector3f CalculateHeight(const float aDistanceFromGround);

		bool IsInValidCameraZone(const Vector2f& aOuterBox, const Vector2f& aInnerBox, const Vector2f& aMin, const Vector2f& aMax, bool& isOutside);

		Vector3f CalculateStartPosition();

		void ApplyLoadedSettings();
		void DrawScreenSpaceLines();

		void GetWPTargetbounds(OUT Vector3f& aMin, OUT Vector3f& aMax);

		const Vector3f ShakeCamera();
	private:
		std::vector<const KE::GameObject*> myTargets;

		CameraSettingsData mySettings;
		KE::CameraComponent* myCameraComponent;

		KE::DebugRenderer* myDBGRenderer;

		bool isPaused = false;

		bool hasSetStartPosition = false;

		bool isDebugCamera = false;

		bool useTargetPosition = false;
		Vector3f debugTargetPos;


		// Camera Shake variables
		Vector3f cachedPosition = {};
		bool isCameraShaking = false;
		float myCameraShakeTime = 1.0f;
		float myTime = 0.0f;
		float myShakeFactor = 0.05f;
	};
}


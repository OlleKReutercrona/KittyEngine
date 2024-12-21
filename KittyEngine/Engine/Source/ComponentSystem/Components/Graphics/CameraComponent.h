#pragma once

#include "Engine/Source/Graphics/Camera.h"
#include "Engine/Source/ComponentSystem/Components/Component.h"

namespace KE
{
	struct CameraComponentData
	{
		Camera* camera;

		// Need this due to set the GameObjects rotation based on CamerComponent rotation. 
		// Suprisingly GameObject rotation and CameraComponent Rotation is set differently in export. /DR
		Vector3f rotation; 
	};

#pragma region CameraComponentMode
	enum class CameraComponentMode
	{
		Lead, // Component will update the Camera's transform to match the GameObject's transform.
		Follow, // Component will update the GameObject's transform to match the Camera's transform. //might cause transform hierarchy issues
		Count
	};

	inline const char* EnumToString(const CameraComponentMode aMode)
	{
		switch (aMode)
		{
		case CameraComponentMode::Lead: return "Lead";
		case CameraComponentMode::Follow: return "Follow";
		default: return "Unknown";
		}
	}

#pragma endregion

	class GameObject;

	class CameraComponent : public Component
	{
		KE_EDITOR_FRIEND
	public:
		CameraComponent(GameObject& aGameObject);
		~CameraComponent() override;

		void Awake() override {};
		//void LateUpdate() override {};
		void Update() override;
		
		void SetData(void* aDataObject = nullptr) override;

		void SetMode(CameraComponentMode aMode) { myMode = aMode; }

		Camera* GetCamera();
		const DirectX::XMMATRIX GetViewMatrix() const;
		const DirectX::XMMATRIX& GetProjectionMatrix() const;
	private:
		CameraComponentMode myMode = CameraComponentMode::Lead;
		Camera* myCamera;
	};
}

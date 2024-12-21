#pragma once

#include "Engine/Source/ComponentSystem/Components/Component.h"

namespace KE
{
	class VFXManager;
	struct VFXRenderInput;
	struct VFXComponentData
	{
		std::vector<std::string> myVFXNames;
		VFXManager* myVFXManager;
		bool myAutoPlay = false;
	};

	class GameObject;

	class VFXComponent : public Component
	{
	public:
		VFXComponent(GameObject& aGameObject);
		~VFXComponent() override;

		void Awake() override;
		void LateUpdate() override {};
		void Update() override;

		void OnDestroy() override;

		void OnEnable() override {  }
		void OnDisable() override {  }

		void TriggerAllVFX(bool aLooping = false, bool aStationary = false);
		void StopAllVFX();
		//Don't create looping stationary VFXs unless you know what you're doing, they can not be stopped.
		void TriggerVFX(int anIndex, bool aLooping = false, bool aStationary = false);
		void TriggerVFXAt(int anIndex, const Vector3f& aPosition, bool aLooping = false);
		void TriggerVFXCustom(int anIndex, const VFXRenderInput& anInput);
		void StopVFX(int anIndex);
		void StopVFXCustom(int anIndex, const VFXRenderInput& anInput);

		void SetData(void* aDataObject = nullptr) override;

		inline int GetVFXSequenceIndex(int anIndex) const { return myVFXSequenceIndices[anIndex]; }
		inline int GetVFXSequenceCount() const { return (int)myVFXSequenceIndices.size(); }

		void SetSequences(const std::vector<std::string>& someVFXSequenceNames);

	protected:
		void OnSceneChange() override;

	private:
		std::vector<int> myVFXSequenceIndices;
		bool shouldAutoPlay = false;
		VFXManager* myVFXManager;
	};
}
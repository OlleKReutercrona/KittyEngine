#pragma once
#include <functional>

#include "Engine/Source/Utility/EventSystem.h"
#include "Engine/Source/Graphics/ModelData.h"
#include "Engine/Source/Graphics/ModelLoader.h"

namespace KE
{
	struct Animation
	{
		AnimationClip* myClip = nullptr;
		float myTime = 0.0f;
		float mySpeed = 1.0f;
		bool isPlaying = false;
		bool isLooping = true;

		std::vector<DirectX::XMMATRIX> myCombinedTransforms = {};
		std::vector<DirectX::XMMATRIX> myFinalTransforms = {};


		inline bool Finished() const
		{
			if (!myClip) return true;
			return (!isLooping) && myTime >= myClip->duration;
		}

		bool IsOnFrame(const int aFrame) const
		{
			const float frameRate = 1.0f / myClip->fps;
			const float result = myTime / frameRate;
			const int frame = (int)std::floor(result); // Current frame
			return frame == aFrame;
		}

		bool HasPassedFrame(const int aFrame) const
		{
			const float frameRate = 1.0f / myClip->fps;
			const float result = myTime / frameRate;
			const int frame = (int)std::floor(result); // Current frame
			return frame > aFrame;
		}

		void UpdateEvents(const unsigned aFrame) const
		{
			if (!myClip) return;
			for (const auto& event : myClip->myEvents)
			{
				if (event->myIsTriggeredThisFrame)
				{
					continue;
				}
				if (event->myFrameIndex > aFrame)
				{
					event->myIsTriggeredThisFrame = true;
					ES::EventSystem::GetInstance().SendEvent(*event);
					std::cout << "\nEvent [" << event->myName << "] triggered at frame " << event->myFrameIndex;
				}
			}
		}

		void ResetEvents() const
		{
			if (!myClip) return;
			for (const auto& event : myClip->myEvents)
			{
				event->myIsTriggeredThisFrame = false;
				std::cout << "\nResetting events!";
			}
		}
	};

	class AnimationPlayer
	{
		KE_EDITOR_FRIEND
	public:
		AnimationPlayer() = default;
		~AnimationPlayer() = default;

		void Init(SkeletalModelData* aModelData);
		void Animate(float aDeltaTime);
		bool UpdateAnimation(const float aDeltaTime, Animation& aOutAnimation) const;
		void BlendPoses(Animation& aFromAnimation, Animation& aToAnimation, const float aBlendFactor) const;

		void PlayAnimation(const std::string& aAnimationName, bool aShouldLoop = true, float aSpeed = 1.0f);
		bool StartAnimation(const std::string& aAnimationName, Animation& aAnimation, bool aShouldLoop = true, float aSpeed = 1.0f);
		void PlayAnimationBlend(const std::string& aAnimationName, bool aShouldLoop = true, float aSpeed = 1.0f, float aBlendTime = 0.2f);
		void ForcePlayAnimationBlend(const std::string& aAnimationName, bool aShouldLoop = true, float aSpeed = 1.0f, float aBlendTime = 0.2f);
		void PauseAnimation();
		void ResumeAnimation();
		void StopAnimation();
		void SetAnimationShouldLoop(bool aShouldLoop);
		void SetAnimationSpeed(float aSpeed);
		void SetBlendTime(float aBlendTime);
		void UpdateWorldSpaceJoints();

		const Animation& GetCurrentAnimation() const { return currentAnimation; };
		const Animation& GetBlendAnimation()   const { return blendAnimation; };
		AnimationClip* GetAnimationClip(const std::string& aAnimationName) const { return modelData->myAnimationClipMap[aAnimationName]; }
		bool IsBlending() const { return isBlending; }

		void AssignAnimations(ModelLoader& aModelLoader, const std::vector<std::string>& aAnimationNames, bool aKeepExisting = false);

	private:
		SkeletalModelData* modelData = nullptr;
		Animation currentAnimation;
		Animation blendAnimation;

		int currentAnimationIndex;
		int blendAnimationIndex;

		bool isAnimationPlaying = false;
		bool isBlending = false;
		bool useBlendTree = false;

		float blendTimer = 0.0f;
		float blendTime = 0.2f;
		float blendFactor = 0.0f;
	};
#pragma region oldAnimationPlayer
	//class GameObject;

	//struct AnimationClip;
	//struct SkeletalModelData;

	//struct AnimationPlayingData
	//{
	//	AnimationClip* clip = nullptr;

	//	float time = 0.0f;
	//	float speed = 1.0f;
	//	bool loop = true;
	//};

	//class AnimationPlayer
	//{
	//	KE_EDITOR_FRIEND
	//public:
	//	AnimationPlayer() = default;
	//	~AnimationPlayer() = default;

	//	void Init(SkeletalModelData* aModelData, GameObject* aGameObject);
	//	void UpdateWorldSpaceJoints() const;
	//	void Animate();

	//	void PlayAnimation(const std::string& aAnimationName, bool aShouldLoop = true, float aSpeed = 1.0f);
	//	void PlayAnimationBlend(const std::string& aAnimationName, bool aShouldLoop = true, float aSpeed = 1.0f, float aBlendTime = 0.2f);

	//	void PauseAnimation();
	//	void ResumeAnimation();
	//	void SetAnimationShouldLoop(bool aShouldLoop);
	//	void SetAnimationSpeed(float aSpeed);

	//	bool IsAnimationPlaying() const;
	//	float GetCurrentAnimationTime() const;
	//	float GetCurrentAnimationLength() const;


	//private:
	//	GameObject* myGameObject = nullptr;
	//	SkeletalModelData* myModelData = nullptr;

	//	//AnimationClip* myCurrentAnimationClip = nullptr;
	//	//AnimationClip* myBlendAnimationClip = nullptr;
	//	//
	//	//float myAnimationTime = 0.0f;
	//	//float myAnimationSpeed = 1.0f;

	//	AnimationPlayingData myCurrentAnim;
	//	AnimationPlayingData myBlendAnim;

	//	bool isAnimationPlaying = true;
	//	/*bool isAnimationLooping = true;*/

	//	bool isBlending = false;
	//	float myBlendTimer = 0.0f;
	//	float myBlendTime = 0.2f;
	//};
#pragma endregion
}

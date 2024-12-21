#pragma once
#include "Engine/Source/Utility/EventSystem.h"

namespace KE
{
	struct AnimationEvent : ES::Event
	{
		AnimationEvent() = default;
		~AnimationEvent() = default;

		std::string myName = "None";
		bool myIsTriggeredThisFrame = false;
		unsigned int myFrameIndex = 0;
	};

	struct Keyframe
	{
		float time = 0.0f;
		std::vector<DirectX::XMMATRIX> boneTransforms{};
	};

	struct AnimationClip
	{
		std::string name = "None";
		float length = 0.0f;
		float fps = 0.0f;
		float duration = 0.0f;
		std::vector<Keyframe> keyframes{};

		std::vector<AnimationEvent*> myEvents = {};
	};

	struct SkeletonBuffer
	{
		DirectX::XMMATRIX myBones[KE_MAX_BONES];
	};
}

#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"

namespace KE
{
	class AnimationPlayer;
}

namespace P8
{
	class Player;
	enum class PlayerAnimation
	{
		Idle,
		Run,
		Dash,
		Windup,
		Throw,
		Death,
		Taunt,
		Select,
		Dance,
		Podium,
		Count
	};
	class PlayerAnimationController
	{
		friend class Player;
	public:
		void Init(KE::AnimationPlayer& aAnimationPlayer, const int aPlayerIndex);
		void PlayAnimation(PlayerAnimation aAnimation, bool aForcePlay = false);
		bool HasPassedFrame(int aFrame);
		bool AnimationFinished();
	private:
		std::unordered_map<PlayerAnimation, std::string> myAnimationNames;
		std::array<bool, static_cast<int>(PlayerAnimation::Count)> myPlaybackStates;
		KE::AnimationPlayer* myAnimationPlayer = nullptr;
	};
}
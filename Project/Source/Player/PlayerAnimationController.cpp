#include "stdafx.h"
#include "PlayerAnimationController.h"

#include <format>
#include <Engine/Source/Graphics/Animation/AnimationPlayer.h>

namespace P8
{
	void PlayerAnimationController::Init(KE::AnimationPlayer& aAnimationPlayer, const int aPlayerIndex)
	{
		myAnimationPlayer = &aAnimationPlayer;

		const int shiftedIndex = aPlayerIndex; 

		const std::string playerIndexStr = shiftedIndex < 10 ? std::format("0{}", shiftedIndex) : std::to_string(shiftedIndex);


		myAnimationNames[PlayerAnimation::Idle]   = std::format("anim_player{}_idle.fbx", playerIndexStr);
		myAnimationNames[PlayerAnimation::Run]    = std::format("anim_player{}_run.fbx", playerIndexStr);
		myAnimationNames[PlayerAnimation::Dash]   = std::format("anim_player{}_dash.fbx", playerIndexStr);
		myAnimationNames[PlayerAnimation::Throw]  = std::format("anim_player{}_throw.fbx", playerIndexStr);
		myAnimationNames[PlayerAnimation::Death]  = std::format("anim_player{}_death.fbx", playerIndexStr);
		myAnimationNames[PlayerAnimation::Taunt]  = std::format("anim_player{}_taunt.fbx", playerIndexStr);
		myAnimationNames[PlayerAnimation::Select] = std::format("anim_player{}_select.fbx", playerIndexStr);
		myAnimationNames[PlayerAnimation::Dance]  = std::format("anim_player{}_victoryDance.fbx", playerIndexStr);
		myAnimationNames[PlayerAnimation::Podium]  = std::format("anim_player{}_podium.fbx", playerIndexStr);

		//no animation for this yet
		myAnimationNames[PlayerAnimation::Windup] = std::format("anim_player{}_idle.fbx", playerIndexStr);

		myPlaybackStates[(int)PlayerAnimation::Idle] = true;
		myPlaybackStates[(int)PlayerAnimation::Run] = true;
		myPlaybackStates[(int)PlayerAnimation::Dash] = false;
		myPlaybackStates[(int)PlayerAnimation::Throw] = false;
		myPlaybackStates[(int)PlayerAnimation::Death] = false;
		myPlaybackStates[(int)PlayerAnimation::Windup] = false;
		myPlaybackStates[(int)PlayerAnimation::Taunt] = false;
		myPlaybackStates[(int)PlayerAnimation::Select] = false;
		myPlaybackStates[(int)PlayerAnimation::Dance] = true;
		myPlaybackStates[(int)PlayerAnimation::Podium] = true;

		myAnimationPlayer->PlayAnimation(myAnimationNames[PlayerAnimation::Idle]);
	}

	void PlayerAnimationController::PlayAnimation(PlayerAnimation aAnimation, bool aForcePlay)
	{
		if (!myAnimationPlayer) return;

		if (aForcePlay)
		{
			myAnimationPlayer->ForcePlayAnimationBlend(myAnimationNames[aAnimation], myPlaybackStates[(int)aAnimation]);
		}
		else
		{
			myAnimationPlayer->PlayAnimationBlend(myAnimationNames[aAnimation], myPlaybackStates[(int)aAnimation]);
		}
	}

	bool PlayerAnimationController::HasPassedFrame(int aFrame)
	{
		if (!myAnimationPlayer) return false;

		return myAnimationPlayer->GetCurrentAnimation().HasPassedFrame(aFrame);
	}

	bool PlayerAnimationController::AnimationFinished()
	{
		if (!myAnimationPlayer) return false;

		return myAnimationPlayer->GetCurrentAnimation().Finished();
	}

}

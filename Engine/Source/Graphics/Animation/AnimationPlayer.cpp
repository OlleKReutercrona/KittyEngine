#include "stdafx.h"
#include "AnimationPlayer.h"

#include "ComponentSystem/GameObject.h"
#include "ComponentSystem/GameObjectManager.h"
//#include "ComponentSystem/Components/AudioComponent.h"
#include "Engine/Source/Graphics/ModelData.h"
#include "Graphics/ModelLoader.h"
#include "SceneManagement/Scene.h"

namespace KE
{
	void AnimationPlayer::Init(SkeletalModelData* aModelData)
	{
		modelData = aModelData;
		currentAnimation.myCombinedTransforms.resize(modelData->mySkeleton->myBones.size());
		currentAnimation.myFinalTransforms.resize(modelData->mySkeleton->myBones.size());
		blendAnimation.myCombinedTransforms.resize(modelData->mySkeleton->myBones.size());
		blendAnimation.myFinalTransforms.resize(modelData->mySkeleton->myBones.size());

		//if (useBlendTree)
		//{
		//	StartAnimation(modelData->animationNames[0], currentAnimation);
		//	StartAnimation(modelData->animationNames[1], blendAnimation);
		//}
	}

	void AnimationPlayer::Animate(const float aDeltaTime)
	{
		if (modelData == nullptr)
		{
			return;
		}

		if (modelData->mySkeleton == nullptr)
		{
			return;
		}

		const Skeleton* skeleton = modelData->mySkeleton;

		if (UpdateAnimation(aDeltaTime, currentAnimation))
		{
			isAnimationPlaying = true;

			if (useBlendTree)
			{
				if (UpdateAnimation(aDeltaTime, blendAnimation))
				{
					BlendPoses(currentAnimation, blendAnimation, blendFactor);
				}
			}
			else
			{
				modelData->myCombinedTransforms = currentAnimation.myCombinedTransforms;
				modelData->myFinalTransforms = currentAnimation.myFinalTransforms;

				if (isBlending)
				{
					if (UpdateAnimation(aDeltaTime, blendAnimation))
					{
						blendTimer += aDeltaTime;
						blendFactor = (std::min)(1.0f, blendTimer / blendTime);

						BlendPoses(currentAnimation, blendAnimation, blendFactor);

						// If blending is complete, reset variables
						if (blendFactor >= 1.0f)
						{
							isBlending = false;
							blendTimer = 0.0f;
							currentAnimation.myClip = blendAnimation.myClip;
							currentAnimation.myTime = blendAnimation.myTime;
							currentAnimation.mySpeed = blendAnimation.mySpeed;
							currentAnimation.isPlaying = blendAnimation.isPlaying;
							currentAnimation.isLooping = blendAnimation.isLooping;
							blendAnimation.myClip = nullptr;
						}
					}
				}
			}
		}
		else
		{
			// Show bind pose (T-pose)
			isAnimationPlaying = false;
			for (size_t i = 0; i < skeleton->myBones.size(); i++)
			{
				// This works because the vertex is not moved in the vertex shader
				modelData->myCombinedTransforms[i] = DirectX::XMMatrixInverse(nullptr, modelData->mySkeleton->myBones[i].myBindPose);
				modelData->myFinalTransforms[i] = DirectX::XMMatrixIdentity();
			}
		}
	}

	bool AnimationPlayer::UpdateAnimation(const float aDeltaTime, Animation& aOutAnimation) const
	{
		const Skeleton* skeleton = modelData->mySkeleton;

		if (aOutAnimation.myClip != nullptr)
		{
			// Apply animation transformation based on the current time
			if (aOutAnimation.isPlaying)
			{
				aOutAnimation.myTime += aDeltaTime * aOutAnimation.mySpeed;

				if (aOutAnimation.isLooping)
				{
					if (aOutAnimation.myTime >= aOutAnimation.myClip->duration)
					{
						aOutAnimation.ResetEvents();
						aOutAnimation.myTime = 0.0f;
					}
				}
				else
				{
					if (aOutAnimation.myTime >= aOutAnimation.myClip->duration)
					{
						//aOutAnimation.ResetEvents();
						aOutAnimation.myTime = aOutAnimation.myClip->duration;
						return true;
					}
				}

				// Calculate the current frame and delta
				const float frameRate = 1.0f / aOutAnimation.myClip->fps;
				const float result = aOutAnimation.myTime / frameRate;
				const int frame = static_cast<int>(std::floor(result)); // Current frame
				const float delta = result - static_cast<float>(frame); // Progress to the next frame

				aOutAnimation.UpdateEvents(frame);

				// Interpolate between current and next frame
				for (size_t i = 0; i < skeleton->myBones.size(); i++)
				{
					DirectX::XMMATRIX currentFramePose = aOutAnimation.myClip->keyframes[frame].boneTransforms[i];
					DirectX::XMMATRIX nextFramePose = aOutAnimation.myClip->keyframes[(frame + 1) % aOutAnimation.myClip->keyframes.size()].boneTransforms[i];

					// Interpolate between current and next frame using delta
					DirectX::XMMATRIX blendedPose = currentFramePose + delta * (nextFramePose - currentFramePose);

					const int parentIndex = skeleton->myBones[i].myParentIndex;

					if (parentIndex >= 0)
					{
						// Accumulate relative transformation
						aOutAnimation.myCombinedTransforms[i] = blendedPose * aOutAnimation.myCombinedTransforms[parentIndex];
					}
					else
					{
						// Root bone, use absolute transformation
						aOutAnimation.myCombinedTransforms[i] = blendedPose;
					}

					aOutAnimation.myFinalTransforms[i] = skeleton->myBones[i].myBindPose * aOutAnimation.myCombinedTransforms[i];
				}
			}
			else
			{
				// Play paused frame i.e do nothing
			}
			return true;
		}
		aOutAnimation.myTime = 0.0f;

		return false;
	}

	void AnimationPlayer::BlendPoses(Animation& aFromAnimation, Animation& aToAnimation, const float aBlendFactor) const
	{
		const Skeleton* skeleton = modelData->mySkeleton;

		for (size_t i = 0; i < skeleton->myBones.size(); i++)
		{
			{
				const DirectX::XMMATRIX currentFramePose = aFromAnimation.myCombinedTransforms[i];
				const DirectX::XMMATRIX blendFramePose = aToAnimation.myCombinedTransforms[i];

				// Blended pose needs to be multiplication of decomposed matrices
				DirectX::XMVECTOR currentScale, currentRotation, currentTranslation;
				DirectX::XMVECTOR blendScale, blendRotation, blendTranslation;
				DirectX::XMMatrixDecompose(&currentScale, &currentRotation, &currentTranslation, currentFramePose);
				DirectX::XMMatrixDecompose(&blendScale, &blendRotation, &blendTranslation, blendFramePose);

				DirectX::XMVECTOR rotationOrigin = DirectX::XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

				DirectX::XMMATRIX blendedPose = DirectX::XMMatrixAffineTransformation(
					DirectX::XMVectorLerp(currentScale, blendScale, aBlendFactor),
					rotationOrigin,
					DirectX::XMQuaternionSlerp(currentRotation, blendRotation, aBlendFactor),
					DirectX::XMVectorLerp(currentTranslation, blendTranslation, aBlendFactor)
				);

				//DirectX::XMMATRIX blendedPose = currentFramePose * (1.0f - aBlendFactor) + blendFramePose * aBlendFactor;



				modelData->myFinalTransforms[i] = skeleton->myBones[i].myBindPose * blendedPose;
			}

			//{
			//	const DirectX::XMMATRIX currentFramePose = aFromAnimation.combinedTransforms[i];
			//	const DirectX::XMMATRIX blendFramePose = aToAnimation.combinedTransforms[i];
			//
			//	// Blended pose needs to be multiplication of decomposed matrices
			//	DirectX::XMVECTOR currentScale, currentRotation, currentTranslation;
			//	DirectX::XMVECTOR blendScale, blendRotation, blendTranslation;
			//	DirectX::XMMatrixDecompose(&currentScale, &currentRotation, &currentTranslation, currentFramePose);
			//	DirectX::XMMatrixDecompose(&blendScale, &blendRotation, &blendTranslation, blendFramePose);
			//
			//	DirectX::XMMATRIX blendedPose = DirectX::XMMatrixAffineTransformation(
			//		DirectX::XMVectorLerp(currentScale, blendScale, aBlendFactor),
			//		DirectX::XMVectorZero(),
			//		DirectX::XMQuaternionSlerp(currentRotation, blendRotation, aBlendFactor),
			//		DirectX::XMVectorLerp(currentTranslation, blendTranslation, aBlendFactor)
			//	);
			//
			//	modelData->myCombinedTransforms[i] = blendedPose;
			//}
		}
	}

	void AnimationPlayer::PlayAnimation(const std::string& aAnimationName, const bool aShouldLoop, const float aSpeed)
	{
		if (isBlending)
		{
			isBlending = false;
			currentAnimation.myClip = modelData->myAnimationClipMap[aAnimationName];
			currentAnimation.myTime = 0.0f;
			currentAnimation.mySpeed = aSpeed;
			currentAnimation.isPlaying = true;
			currentAnimation.isLooping = aShouldLoop;
			currentAnimation.ResetEvents();
			return;
		}

		if (modelData->myAnimationClipMap.empty())
		{
			return;
		}

		if (currentAnimation.myClip != nullptr && currentAnimation.myClip->name == aAnimationName)
		{
			return;
		}

		if (currentAnimation.myClip != nullptr)
		{
			PlayAnimationBlend(aAnimationName, aShouldLoop, aSpeed, blendTime);
			return;
		}

		currentAnimation.myClip = modelData->myAnimationClipMap[aAnimationName];
		currentAnimation.myTime = 0.0f;
		currentAnimation.mySpeed = aSpeed;
		currentAnimation.isPlaying = true;
		currentAnimation.isLooping = aShouldLoop;
		currentAnimation.ResetEvents();
	}

	bool AnimationPlayer::StartAnimation(const std::string& aAnimationName, Animation& aAnimation, bool aShouldLoop, float aSpeed)
	{
		if (modelData->myAnimationClipMap.empty())
		{
			return false;
		}

		if (aAnimation.myClip != nullptr && aAnimation.myClip->name == aAnimationName)
		{
			return false;
		}

		aAnimation.myClip = modelData->myAnimationClipMap[aAnimationName];
		aAnimation.myTime = 0.0f;
		aAnimation.mySpeed = aSpeed;
		aAnimation.isPlaying = true;
		aAnimation.isLooping = aShouldLoop;

		return true;
	}

	void AnimationPlayer::PlayAnimationBlend(const std::string& aAnimationName, const bool aShouldLoop,
		const float aSpeed, const float aBlendTime)
	{
		if (modelData->myAnimationClipMap.empty())
		{
			return;
		}

		if (currentAnimation.myClip == nullptr)
		{
			PlayAnimation(aAnimationName, aShouldLoop, aSpeed);
			return;
		}

		if (currentAnimation.myClip->name == aAnimationName)
		{
			//PlayAnimation(aAnimationName, aShouldLoop, aSpeed);
			return;
		}

		blendAnimation.myClip = modelData->myAnimationClipMap[aAnimationName];
		isBlending = true;
		blendTime = aBlendTime;
		blendAnimation.myTime = 0.0f;
		blendAnimation.mySpeed = aSpeed;
		blendAnimation.isPlaying = true;
		blendAnimation.isLooping = aShouldLoop;
		blendAnimation.ResetEvents();
	}

	void AnimationPlayer::ForcePlayAnimationBlend(const std::string& aAnimationName, bool aShouldLoop, float aSpeed, float aBlendTime)
	{
		if (modelData->myAnimationClipMap.empty())
		{
			return;
		}

		if (currentAnimation.myClip == nullptr)
		{
			PlayAnimation(aAnimationName, aShouldLoop, aSpeed);
			return;
		}

		blendAnimation.myClip = modelData->myAnimationClipMap[aAnimationName];
		isBlending = true;
		blendTime = aBlendTime;
		blendAnimation.myTime = 0.0f;
		blendAnimation.mySpeed = aSpeed;
		blendAnimation.isPlaying = true;
		blendAnimation.isLooping = aShouldLoop;
		blendAnimation.ResetEvents();
	}

	void AnimationPlayer::PauseAnimation()
	{
		currentAnimation.isPlaying = false;
	}

	void AnimationPlayer::ResumeAnimation()
	{
		currentAnimation.isPlaying = true;
	}

	void AnimationPlayer::StopAnimation()
	{
		currentAnimation.myClip = nullptr;
	}

	void AnimationPlayer::SetAnimationShouldLoop(const bool aShouldLoop)
	{
		currentAnimation.isLooping = aShouldLoop;
	}

	void AnimationPlayer::SetAnimationSpeed(const float aSpeed)
	{
		currentAnimation.mySpeed = aSpeed;
	}

	void AnimationPlayer::SetBlendTime(const float aBlendTime)
	{
		blendTime = aBlendTime;
	}

	void AnimationPlayer::UpdateWorldSpaceJoints()
	{
		for (int i = 0; i < modelData->mySkeleton->myBones.size(); i++)
		{
			modelData->myWorldSpaceJoints[i] = 
				DirectX::XMMatrixInverse(nullptr, modelData->mySkeleton->myBones[i].myBindPose) * 
				modelData->myFinalTransforms[i] * 
				DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f) *
				*modelData->myTransform;
		}
	}

	void AnimationPlayer::AssignAnimations(ModelLoader& aModelLoader, const std::vector<std::string>& aAnimationNames, bool aKeepExisting)
	{
		if (!aKeepExisting)
		{
			modelData->myAnimationClipMap.clear();
			currentAnimation.myClip = nullptr;
		}

		for (const auto& animationPath : aAnimationNames)
		{
			aModelLoader.LoadAnimation(*modelData, animationPath);
			auto* animationClip = aModelLoader.GetAnimationClip(animationPath);

			if (animationClip != nullptr)
			{
				modelData->myAnimationClipMap[animationClip->name] = animationClip;
			}
		}
	}

#pragma region oldAnimplayer
	//void AnimationPlayer::Init(SkeletalModelData* aModelData, GameObject* aGameObject)
	//{
	//	myModelData = aModelData;
	//	myGameObject = aGameObject;
	//}

	//void AnimationPlayer::UpdateWorldSpaceJoints() const
	//{
	//	for (int i = 0; i < myModelData->mySkeleton->myBones.size(); i++)
	//	{
	//		myModelData->myWorldSpaceJoints[i] = DirectX::XMMatrixInverse(
	//			nullptr, myModelData->mySkeleton->myBones[i].myBindPose
	//		) * myModelData->myFinalTransforms[i] * DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f) * *myModelData->myTransform;
	//	}
	//}

	//void AnimationPlayer::Animate()
	//{
	//	const Skeleton* skeleton = myModelData->mySkeleton;



	//	if (isBlending)
	//	{
	//		// Increment the blend timer
	//		myBlendTimer += KE_GLOBAL::deltaTime;
	//		myCurrentAnim.time += KE_GLOBAL::deltaTime * myCurrentAnim.speed;
	//		myBlendAnim.time += KE_GLOBAL::deltaTime * myBlendAnim.speed;

	//		// Calculate blend factor based on the timer and blend time
	//		const float blendFactor = (std::min)(1.0f, myBlendTimer / myBlendTime);

	//		// Calculate the current frame and delta
	//		const float currentFrameRate = 1.0f / myCurrentAnim.clip->myFps;
	//		const float currentResult = myCurrentAnim.time / currentFrameRate;
	//		const size_t currentFrame = (std::size_t)std::floor(currentResult) % myCurrentAnim.clip->myKeyframes.size(); // Current frame

	//		// Calculate the blend frame and delta
	//		const float blendFrameRate = 1.0f / myBlendAnim.clip->myFps;
	//		const float blendResult = myCurrentAnim.time / blendFrameRate;
	//		const size_t blendFrame = (std::size_t)std::floor(blendResult) % myBlendAnim.clip->myKeyframes.size(); // Current frame


	//		// Interpolate between the two animations using blend factor
	//		for (size_t i = 0; i < skeleton->myBones.size(); i++)
	//		{
	//			const DirectX::XMMATRIX currentFramePose = myCurrentAnim.clip->myKeyframes[currentFrame].myBoneTransforms[i];
	//			const DirectX::XMMATRIX nextFramePose = myBlendAnim.clip->myKeyframes[blendFrame].myBoneTransforms[i];

	//			// Blended pose needs to be multiplication of decomposed matrices
	//			DirectX::XMVECTOR currentScale, currentRotation, currentTranslation;
	//			DirectX::XMVECTOR nextScale, nextRotation, nextTranslation;
	//			DirectX::XMMatrixDecompose(&currentScale, &currentRotation, &currentTranslation, currentFramePose);
	//			DirectX::XMMatrixDecompose(&nextScale, &nextRotation, &nextTranslation, nextFramePose);

	//			DirectX::XMMATRIX blendedPose = DirectX::XMMatrixAffineTransformation(
	//				DirectX::XMVectorLerp(currentScale, nextScale, blendFactor),
	//				DirectX::XMVectorZero(),
	//				DirectX::XMQuaternionSlerp(currentRotation, nextRotation, blendFactor),
	//				DirectX::XMVectorLerp(currentTranslation, nextTranslation, blendFactor)
	//			);

	//			const int parentIndex = skeleton->myBones[i].myParentIndex;

	//			if (parentIndex >= 0)
	//			{
	//				// Accumulate relative transformation
	//				myModelData->myCombinedTransforms[i] = blendedPose * myModelData->myCombinedTransforms[parentIndex];
	//			}
	//			else
	//			{
	//				// Root bone, use absolute transformation
	//				myModelData->myCombinedTransforms[i] = blendedPose;
	//			}

	//			myModelData->myFinalTransforms[i] = skeleton->myBones[i].myBindPose * myModelData->myCombinedTransforms[i];
	//		}

	//		// If blending is complete, reset variables
	//		if (blendFactor >= 1.0f)
	//		{
	//			isBlending = false;
	//			myBlendTimer = 0.0f;
	//			myCurrentAnim = myBlendAnim;
	//			myBlendAnim = {};
	//		}
	//	}
	//	else
	//	{
	//		if (myCurrentAnim.clip != nullptr)
	//		{
	//			// Apply animation transformation based on the current time
	//			if (isAnimationPlaying)
	//			{
	//				myCurrentAnim.time += KE_GLOBAL::deltaTime * myCurrentAnim.speed;

	//				if (myCurrentAnim.loop)
	//				{
	//					if (myCurrentAnim.time >= myCurrentAnim.clip->myDuration)
	//					{
	//						myCurrentAnim.time = 0.0f;
	//					}
	//				}
	//				else
	//				{
	//					if (myCurrentAnim.time >= myCurrentAnim.clip->myDuration)
	//					{
	//						myCurrentAnim.time = myCurrentAnim.clip->myDuration;
	//						isAnimationPlaying = false;
	//						myCurrentAnim.clip = nullptr;
	//						return;
	//					}
	//				}

	//				// Calculate the current frame and delta
	//				const float frameRate = 1.0f / myCurrentAnim.clip->myFps;
	//				const float result = myCurrentAnim.time / frameRate;
	//				const size_t frame = (std::size_t)std::floor(result); // Current frame
	//				const float delta = result - static_cast<float>(frame); // Progress to the next frame

	//				// Interpolate between current and next frame
	//				for (size_t i = 0; i < skeleton->myBones.size(); i++)
	//				{
	//					DirectX::XMMATRIX currentFramePose = myCurrentAnim.clip->myKeyframes[frame].myBoneTransforms[i];
	//					DirectX::XMMATRIX nextFramePose = myCurrentAnim.clip->myKeyframes[(frame + 1) % myCurrentAnim.clip->myKeyframes.size()].
	//						myBoneTransforms[i];

	//					// Interpolate between current and next frame using delta
	//					DirectX::XMMATRIX blendedPose = currentFramePose + delta * (nextFramePose - currentFramePose);

	//					const int parentIndex = skeleton->myBones[i].myParentIndex;

	//					if (parentIndex >= 0)
	//					{
	//						// Accumulate relative transformation
	//						myModelData->myCombinedTransforms[i] = blendedPose * myModelData->myCombinedTransforms[parentIndex];
	//					}
	//					else
	//					{
	//						// Root bone, use absolute transformation
	//						myModelData->myCombinedTransforms[i] = blendedPose;
	//					}

	//					myModelData->myFinalTransforms[i] = skeleton->myBones[i].myBindPose * myModelData->myCombinedTransforms[i];
	//				}
	//			}
	//			else
	//			{
	//				// Play paused frame i.e do nothing
	//			}
	//		}
	//		else
	//		{
	//			// Show bind pose (T-pose)
	//			myCurrentAnim.time = 0.0f;

	//			for (size_t i = 0; i < skeleton->myBones.size(); i++)
	//			{
	//				// This works because the vertex is not moved in the vertex shader
	//				myModelData->myFinalTransforms[i] = DirectX::XMMatrixIdentity();
	//			}
	//		}
	//	}

	//	if (!isAnimationPlaying || myCurrentAnim.clip == nullptr)
	//	{
	//		return;
	//	}
	//}

	//
	//void AnimationPlayer::PlayAnimation(const std::string& aAnimationName, const bool aShouldLoop, const float aSpeed)
	//{
	//	if (this == nullptr)
	//	{
	//		KE_ERROR("AnimationPlayer::PlayAnimation() > AnimationPlayer is nullptr!");
	//		return;
	//	}
	//	if (isBlending)
	//	{
	//		isBlending = false;
	//		isAnimationPlaying = true;

	//		myCurrentAnim.clip = myModelData->myAnimationClipMap[aAnimationName];
	//		myCurrentAnim.time = 0.0f;
	//		myCurrentAnim.speed = aSpeed;
	//		myCurrentAnim.loop = aShouldLoop;
	//		return;
	//	}
	//	if (myModelData->myAnimationClipMap.empty())
	//	{
	//		KE_ERROR("AnimationPlayer::PlayAnimation() > No animation clips found!");
	//		return;
	//	}

	//	if (myCurrentAnim.clip != nullptr && myCurrentAnim.clip->myName == aAnimationName)
	//	{
	//		return;
	//	}

	//	if (myCurrentAnim.clip != nullptr)
	//	{
	//		PlayAnimationBlend(aAnimationName, aShouldLoop, aSpeed, 0.1f);
	//		return;
	//	}

	//	isBlending = false;
	//	isAnimationPlaying = true;

	//	myCurrentAnim.clip = myModelData->myAnimationClipMap[aAnimationName];
	//	myCurrentAnim.time = 0.0f;
	//	myCurrentAnim.speed = aSpeed;
	//	myCurrentAnim.loop = aShouldLoop;

	//}



	//void AnimationPlayer::PlayAnimationBlend(const std::string& aAnimationName, const bool aShouldLoop, const float aSpeed, const float aBlendTime)
	//{
	//	if (this == nullptr)
	//	{
	//		KE_ERROR("AnimationPlayer::PlayAnimation() > AnimationPlayer is nullptr!");
	//		return;
	//	}
	//	if (myModelData->myAnimationClipMap.empty())
	//	{
	//		KE_ERROR("AnimationPlayer::PlayAnimationBlend() > No animation clips found!");
	//		return;
	//	}

	//	if (myCurrentAnim.clip == nullptr)
	//	{
	//		PlayAnimation(aAnimationName, aShouldLoop, aSpeed);
	//		return;
	//	}

	//	isBlending = true;
	//	isAnimationPlaying = true;
	//	myBlendTimer = 0.0f;
	//	myBlendTime = aBlendTime;
	//	myBlendAnim.clip = myModelData->myAnimationClipMap[aAnimationName];
	//	myBlendAnim.time = 0.0f;
	//	myBlendAnim.speed = aSpeed;

	//	myBlendAnim.loop = aShouldLoop;
	//}


	//void AnimationPlayer::PauseAnimation()
	//{
	//	isAnimationPlaying = false;
	//}

	//void AnimationPlayer::ResumeAnimation()
	//{
	//	isAnimationPlaying = true;
	//}

	//void AnimationPlayer::SetAnimationShouldLoop(const bool aShouldLoop)
	//{
	//	myCurrentAnim.loop = aShouldLoop;
	//}

	//void AnimationPlayer::SetAnimationSpeed(const float aSpeed)
	//{
	//	myCurrentAnim.speed = aSpeed;
	//}

	//bool AnimationPlayer::IsAnimationPlaying() const
	//{
	//	return isAnimationPlaying;
	//}

	//float AnimationPlayer::GetCurrentAnimationTime() const
	//{
	//	return myCurrentAnim.time;
	//}

	//float AnimationPlayer::GetCurrentAnimationLength() const
	//{
	//	return myCurrentAnim.clip ? myCurrentAnim.clip->myDuration : -1.0f;
	//}
#pragma endregion
}

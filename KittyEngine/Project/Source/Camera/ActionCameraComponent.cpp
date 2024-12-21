#include "stdafx.h"
#include "ActionCameraComponent.h"
#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/ComponentSystem/GameObjectManager.h"
#include "Engine/Source/Graphics/DebugRenderer.h"
#include "Engine/Source/ComponentSystem/Components/Graphics/CameraComponent.h"

#include "Engine/Source/Utility/Logging.h"
#include "Engine/Source/Math/KittyMath.h"
#include "Engine/Source/Utility/Global.h"
#include "Engine/Source/Utility/Randomizer.h"
#include "Project/Source/GameEvents/GameEvents.h"
#include "Project/Source/Managers/GameManager.h"

#include "Project/Source/Player/Player.h"
#include "Project/Source/Camera/CameraSettingsFile.h"
#include "Engine/Source/Collision/Intersection.h"

P8::ActionCameraComponent::ActionCameraComponent(KE::GameObject& aGameObject) : KE::Component(aGameObject)
{
	CameraSettingsData tempData;
	if (CameraSettingsFile::Load(&tempData))
	{
		mySettings = tempData;
	}

	OnInit();
}

P8::ActionCameraComponent::~ActionCameraComponent()
{
	myTargets.clear();
}

void P8::ActionCameraComponent::Awake()
{
	//auto targets = myGameObject.GetManager().GetGameObjectsWithComponent<P8::Player>();

	//for (unsigned int i = 0; i < targets.size(); i++)
	//{
	//	AddTarget(*targets[i]);
	//}

	myCameraComponent = &myGameObject.GetComponent<KE::CameraComponent>();
	ApplyLoadedSettings();

	myDBGRenderer = KE_GLOBAL::blackboard.Get<KE::DebugRenderer>("debugRenderer");
}

void P8::ActionCameraComponent::LateUpdate()
{
	if (isDebugCamera)
	{
		Vector3f targetPosition;
		if (useTargetPosition)
		{
			targetPosition = debugTargetPos;
		}
		else
		{
			targetPosition = CalculateTargetPosition();
			targetPosition.y += 0.60f;
		}


		Vector3f calculatedPosition = CalculateMovement(targetPosition);

		myGameObject.myTransform.SetPosition(calculatedPosition);
		myGameObject.myTransform.LookAt(targetPosition);

		return;
	}
	if (P8::GameManager::IsPaused()) return;
	if (!isActive) { return; }

#ifdef _DEBUG
	DrawScreenSpaceLines();
#endif
	if (isCameraShaking)
	{
		if (cachedPosition.x != FLT_MAX)
		{
			myGameObject.myTransform.SetPosition(cachedPosition);
		}
	}


	const Vector3f targetPosition = CalculateTargetPosition();
	Vector3f calculatedPosition = CalculateMovement(targetPosition);

	float distanceFromGround = Vector3f::Length(calculatedPosition - targetPosition);
	Vector3f calcDimensions = CalculateHeight(distanceFromGround);

	//std::cout << calcDimensions.Length() << "\n";

	calculatedPosition += calcDimensions;

	if (isCameraShaking)
	{
		myTime += KE_GLOBAL::deltaTime;

		// Shake the camera
		Vector3f shakeVector = ShakeCamera();

		cachedPosition = calculatedPosition;
		calculatedPosition += shakeVector;

		if (myTime >= myCameraShakeTime)
		{
			cachedPosition.x = FLT_MAX;

			isCameraShaking = false;
			myTime = 0.0f;
		}
	}

	myGameObject.myTransform.SetPosition(calculatedPosition);

	myDBGRenderer->RenderSphere(targetPosition, 0.5f, { 1.0f, 0.0f, 0.0f, 1.0f });
}

void P8::ActionCameraComponent::SetData(void* someData)
{
}

void P8::ActionCameraComponent::AddTarget(const KE::GameObject& aTarget)
{
	for (unsigned int i = 0; i < myTargets.size(); i++)
	{
		if (myTargets[i] == &aTarget)
		{
			KE_LOG_CHANNEL("ActionCamera", "Tried to add target that already existed in ActionCameraComponent");
			return;
		}
	}

	myTargets.push_back(&aTarget);

	CalculateTargetPosition();
}

void P8::ActionCameraComponent::RemoveTarget(const KE::GameObject& aTarget)
{
	for (unsigned int i = 0; i < myTargets.size(); i++)
	{
		if (myTargets[i] == &aTarget)
		{
			myTargets.erase(myTargets.begin() + i);
			CalculateTargetPosition();
			return;
		}
	}
	KE_LOG_CHANNEL("ActionCamera", "Tried to remove target that didn't exist in ActionCameraComponent");
}

Vector3f P8::ActionCameraComponent::CalculateTargetPosition()
{
	if (myTargets.empty()) { return Vector3f(); }

	const Vector3f firstPos = myTargets[0]->myTransform.GetPosition();

	Vector2f firstSP;
	myCameraComponent->GetCamera()->WorldToScreenPoint(firstPos, firstSP, *KE_GLOBAL::resolution);


	Vector2f min(firstSP.x, firstSP.y);
	Vector2f max(firstSP.x, firstSP.y);

	if (!isDebugCamera)
	{
		for (unsigned int i = 1; i < myTargets.size(); i++)
		{
			const Vector3f pos = myTargets[i]->myTransform.GetPosition();

			Vector2f screenPoint;
			myCameraComponent->GetCamera()->WorldToScreenPoint(pos, screenPoint, *KE_GLOBAL::resolution);

			if (screenPoint.x < min.x) { min.x = screenPoint.x; };
			if (screenPoint.y < min.y) { min.y = screenPoint.y; };
			if (screenPoint.x > max.x) { max.x = screenPoint.x; };
			if (screenPoint.y > max.y) { max.y = screenPoint.y; };
		}
	}

	Vector2f averageSP = {
		min.x + (max.x - min.x) / 2.0f,
		min.y + (max.y - min.y) / 2.0f
	};

	Rayf moveToRay = myCameraComponent->GetCamera()->GetRay(averageSP);
	Vector3f planeHit = moveToRay.GetOrigin() + moveToRay.GetDirection() * (-moveToRay.GetOrigin().y / moveToRay.GetDirection().y);

	Vector3f returnPosition = planeHit;

	returnPosition.y = 0.0f;

	return returnPosition;

}

Vector3f P8::ActionCameraComponent::CalculateMovement(const Vector3f& aTargetPosition)
{
	const Vector3f currentPosition = myGameObject.myTransform.GetPosition();

	Vector3f forward = myCameraComponent->GetCamera()->transform.GetForward();

	float distanceFromGround = Vector3f::Length(currentPosition - aTargetPosition);

	if (distanceFromGround < mySettings.minDistanceFromGround) { distanceFromGround = mySettings.minDistanceFromGround; }

	if (distanceFromGround > mySettings.maxDistanceFromGround) { distanceFromGround = mySettings.maxDistanceFromGround; }



	Vector3f direction = (forward * -1.0f) * distanceFromGround;

	Vector3f potentialTarget = aTargetPosition + direction;

	Vector3f potentialMovement = (potentialTarget - currentPosition) * mySettings.horizontalMovementSpeed * KE_GLOBAL::deltaTime;

	if (potentialMovement.Length() >= 15.0f)
	{
		potentialMovement = Vector3f();
	}

	return currentPosition + potentialMovement;
}

Vector3f P8::ActionCameraComponent::CalculateAndApplyDimensions(const Vector3f& aPossiblePosition)
{
	//	 			  b
	//				  |\
	//				  | \
	//				  |  \
	//	b			  |	  \
	//  |\			  |	   \
	//  | \		->	  |	    \
	//  |  \		  |	     \
	//  |	\		  |		  \
	//  |____\		  |________\		
	//	a	  c		  a			c

	if (myTargets.size() <= 0) return Vector3f();


	const float validOutsideBorderPercentage = 1.0f - mySettings.outerBorderDistancePercentage / 100.0f;
	const float validInnerBorderPercentage = 1.0f - (mySettings.outerBorderDistancePercentage + mySettings.innerBorderDistancePercentage) / 100.0f;

	Vector2f resF((float)KE_GLOBAL::resolution->x, (float)KE_GLOBAL::resolution->y);

	bool isTargetOutside = false;
	float rangefromValidBorder = 0.0f;
	float closestFromInnerBorder = FLT_MAX;
	float movementForward = 0.0f;

	std::vector<Vector3f> targetPositions(myTargets.size());
	for (unsigned int i = 0; i < myTargets.size(); i++)
	{
		targetPositions.push_back(myTargets[i]->myTransform.GetPosition());
	}
	std::vector<bool> isExtreme(myTargets.size());

	for (unsigned int i = 0; i < myTargets.size(); i++)
	{
		auto& thisPos = targetPositions[i];
		bool extremeOnAxis[4] = { 1 };
		//+x, -x, +y, -y
		for (unsigned int j = 0; j < myTargets.size(); j++)
		{
			if (i == j) { continue; }
			auto& otherPos = targetPositions[j];

			if (thisPos.x < otherPos.x) { extremeOnAxis[0] = false; }
			if (thisPos.x > otherPos.x) { extremeOnAxis[1] = false; }
			if (thisPos.y < otherPos.y) { extremeOnAxis[2] = false; }
			if (thisPos.y > otherPos.y) { extremeOnAxis[3] = false; }
		}
		if (*(int*)extremeOnAxis) { isExtreme[i] = true; }
	}

	for (unsigned int i = 0; i < myTargets.size(); i++)
	{
		if (!isExtreme[i]) { continue; }

		auto position = myTargets[i]->myTransform.GetPosition();

		Vector2f point;

		myCameraComponent->GetCamera()->WorldToScreenPoint(position, point);

		Vector2f uv = point / resF;
		Vector2f ndc = (uv * 2.0f - 1.0f) * Vector2f(1.0f, -1.0f);
		Vector2f absNDC = { abs(ndc.x), abs(ndc.y) };

		if (absNDC.x <= validOutsideBorderPercentage && absNDC.x >= validInnerBorderPercentage
			&& absNDC.y <= validOutsideBorderPercentage && absNDC.y >= validInnerBorderPercentage)
		{
			// target is inside valid bounds
			continue;
		}

		if (IntersectBoxPoint({ validOutsideBorderPercentage, validOutsideBorderPercentage }, absNDC) == false)
		{
			// Target is outside valid bounds

			isTargetOutside = true;

			if (absNDC.x > rangefromValidBorder) { rangefromValidBorder = absNDC.x * -1.0f; }
			if (absNDC.y > rangefromValidBorder) { rangefromValidBorder = absNDC.y * -1.0f; }
		}

		if (isTargetOutside == false && IntersectBoxPoint({ validInnerBorderPercentage, validInnerBorderPercentage }, absNDC))
		{
			float xdist = (validInnerBorderPercentage - absNDC.x);
			float ydist = (validInnerBorderPercentage - absNDC.y);

			if (xdist < closestFromInnerBorder) { closestFromInnerBorder = xdist; }
			if (ydist < closestFromInnerBorder) { closestFromInnerBorder = ydist; }
		}
	}
	if (closestFromInnerBorder != FLT_MAX && rangefromValidBorder == 0.0f)
	{
		rangefromValidBorder = closestFromInnerBorder;
	}

	std::cout << rangefromValidBorder << "\n";

	movementForward = KE_GLOBAL::deltaTime * mySettings.verticalMovementSpeed * rangefromValidBorder;

	if (abs(movementForward) <= 0.002f)
	{
		movementForward = 0.0f;
	}


	Vector4f cameraSpaceForward = { 0.0f, 0.0f, movementForward, 0.0f };
	cameraSpaceForward = myGameObject.myTransform.GetCUMatrix() * cameraSpaceForward;

	Vector3f finalMovement = { cameraSpaceForward.x,cameraSpaceForward.y, cameraSpaceForward.z };

	return finalMovement;
}

Vector3f P8::ActionCameraComponent::CalculateHeight(const float aDistanceFromGround)
{
	Vector3f height;

	const float validOutsideBorderPercentage = 1.0f - mySettings.outerBorderDistancePercentage / 100.0f;
	const float validInnerBorderPercentage = 1.0f - (mySettings.outerBorderDistancePercentage + mySettings.innerBorderDistancePercentage) / 100.0f;

	Vector2f resF((float)KE_GLOBAL::resolution->x, (float)KE_GLOBAL::resolution->y);


	Vector2f min{ FLT_MAX, FLT_MAX };
	Vector2f max{ -FLT_MAX, -FLT_MAX };
	for (int i = 0; i < myTargets.size(); i++)
	{
		auto position = myTargets[i]->myTransform.GetPosition();

		Vector2f point;

		myCameraComponent->GetCamera()->WorldToScreenPoint(position, point);

		Vector2f uv = point / resF;
		Vector2f ndc = (uv * 2.0f - 1.0f) * Vector2f(1.0f, -1.0f);

		if (ndc.x > max.x) { max.x = ndc.x; }
		if (ndc.y > max.y) { max.y = ndc.y; }
		if (ndc.x < min.x) { min.x = ndc.x; }
		if (ndc.y < min.y) { min.y = ndc.y; }
	}

	bool isOutside = false;
	if (IsInValidCameraZone(
		{ validOutsideBorderPercentage ,validOutsideBorderPercentage },
		{ validInnerBorderPercentage , validInnerBorderPercentage },
		min,
		max,
		isOutside))
	{
		return height;
	}

	bool isTargetOutside = false;
	float rangefromValidBorder = 0.0f;
	float closestFromInnerBorder = FLT_MAX;

	for (unsigned int i = 0; i < myTargets.size(); i++)
	{
		auto position = myTargets[i]->myTransform.GetPosition();

		Vector2f point;

		myCameraComponent->GetCamera()->WorldToScreenPoint(position, point);

		Vector2f uv = point / resF;
		Vector2f ndc = (uv * 2.0f - 1.0f) * Vector2f(1.0f, -1.0f);
		Vector2f absNDC = { abs(ndc.x), abs(ndc.y) };

		if (absNDC.x <= validOutsideBorderPercentage && absNDC.x >= validInnerBorderPercentage
			&& absNDC.y <= validOutsideBorderPercentage && absNDC.y >= validInnerBorderPercentage)
		{
			// target is inside valid bounds
			continue;
		}

		if (IntersectBoxPoint({ validOutsideBorderPercentage, validOutsideBorderPercentage }, absNDC) == false)
		{
			// Target is outside valid bounds

			isTargetOutside = true;

			if (absNDC.x * -1.0f < rangefromValidBorder) { rangefromValidBorder = absNDC.x * -1.0f; }
			if (absNDC.y * -1.0f < rangefromValidBorder) { rangefromValidBorder = absNDC.y * -1.0f; }
		}

		if (isTargetOutside == false && IntersectBoxPoint({ validInnerBorderPercentage, validInnerBorderPercentage }, absNDC))
		{
			float xdist = (validInnerBorderPercentage - absNDC.x);
			float ydist = (validInnerBorderPercentage - absNDC.y);

			if (xdist < closestFromInnerBorder) { closestFromInnerBorder = xdist; }
			if (ydist < closestFromInnerBorder) { closestFromInnerBorder = ydist; }
		}
	}

	float zoomSpeed = mySettings.verticalMovementSpeed;

	if (closestFromInnerBorder != FLT_MAX && rangefromValidBorder == 0.0f)
	{
		// This is a bit jank with hardcoded numbers, I know I know. But it feels good and at this point we just need to finish this -- Olle
		rangefromValidBorder = closestFromInnerBorder * 1.75f;
		zoomSpeed = 35.0f;
	}


	float movementForward = KE_GLOBAL::deltaTime * zoomSpeed * rangefromValidBorder;

	if (abs(movementForward) <= 0.002f)
	{
		movementForward = 0.0f;
	}

	const float padding = 0.01f;

	if (movementForward > 0.0f)
	{
		int hej = 0;
		hej++;
	}


	if (movementForward < 0.0f)
	{
		// if zooming out
		if (aDistanceFromGround + movementForward > mySettings.maxDistanceFromGround)
		{
			movementForward = std::clamp(aDistanceFromGround - mySettings.maxDistanceFromGround, 0.0f, mySettings.maxDistanceFromGround);

			//return Vector3f();
		}
	}
	else if (movementForward > 0.0f)
	{
		// if zooming in
		if (aDistanceFromGround - movementForward < mySettings.minDistanceFromGround)
		{
			movementForward = std::clamp(aDistanceFromGround - mySettings.minDistanceFromGround, 0.0f, mySettings.maxDistanceFromGround);

			//return Vector3f();
		}
	}

	Vector4f cameraSpaceForward = { 0.0f, 0.0f, movementForward, 0.0f };
	cameraSpaceForward = myGameObject.myTransform.GetCUMatrix() * cameraSpaceForward;

	height = { cameraSpaceForward.x,cameraSpaceForward.y, cameraSpaceForward.z };

	return height;
}



bool P8::ActionCameraComponent::IsInValidCameraZone(const Vector2f& aOuterBox, const Vector2f& aInnerBox, const Vector2f& aMin, const Vector2f& aMax, bool& isOutside)
{
	Vector2f BR = { aMax.x, aMin.y };
	Vector2f UL = { aMin.x, aMax.y };

	// Is inside the outer box?
	bool isBL = IntersectBoxPoint(aOuterBox, aMin);
	bool isBR = IntersectBoxPoint(aOuterBox, BR);
	bool isUL = IntersectBoxPoint(aOuterBox, UL);
	bool isUR = IntersectBoxPoint(aOuterBox, aMax);


	bool isInside = isBL && isBR && isUL && isUR;
	if (isInside == false)
	{
		isOutside = true;

		return false;
	}

	// If all is inside outer box, is it outside the inner box?
	isBL = !IntersectBoxPoint(aInnerBox, aMin);
	isBR = !IntersectBoxPoint(aInnerBox, BR);
	isUL = !IntersectBoxPoint(aInnerBox, UL);
	isUR = !IntersectBoxPoint(aInnerBox, aMax);


	isOutside = false;
	return isBL && isBR && isUL && isUR;
}


Vector3f P8::ActionCameraComponent::CalculateStartPosition()
{
	const Vector3f targetPosition = CalculateTargetPosition();

	float height = mySettings.minDistanceFromGround;

	if (myTargets.size() > 1)
	{
		Vector3f min(FLT_MAX, 0, FLT_MAX);
		Vector3f max(-FLT_MAX, 0, -FLT_MAX);

		GetWPTargetbounds(min, max);

		float dist = Vector3f::Length(min - max);
		dist = dist * 2.0f/* + 5.0f*/;

		if (dist > mySettings.minDistanceFromGround)
		{
			height = dist;
		}
	}


	const Vector3f currentPosition = myGameObject.myTransform.GetPosition();

	Vector3f forward = myCameraComponent->GetCamera()->transform.GetForward();

	Vector3f direction = (forward * -1.0f) * height;

	Vector3f potentialTarget = targetPosition + direction;

	return potentialTarget;
}

void P8::ActionCameraComponent::ApplyLoadedSettings()
{
	if (!isActive) { return; }
	myCameraComponent->GetCamera()->SetFOV(KE::DegToRadImmediate * mySettings.cameraFOV);

	myGameObject.myTransform.SetRotation(KE::DegToRadImmediate * mySettings.cameraDirection);
}


void P8::ActionCameraComponent::OnReceiveEvent(ES::Event& aEvent)
{
	if (isDebugCamera) { return; }

	if (P8::ActionCameraEvent* aceMsg = dynamic_cast<P8::ActionCameraEvent*>(&aEvent))
	{
		aceMsg->addTarget ? AddTarget(aceMsg->gameObject) : RemoveTarget(aceMsg->gameObject);

		if (GameManager::GetCurrentGameState() == eGameStates::ePlayTutorial || GameManager::GetCurrentGameState() == eGameStates::ePlayCountdown)
		{
			//Vector3f startPosition = CalculateStartPosition();

			//myGameObject.myTransform.SetPosition(startPosition);
		}
		return;
	}

	else if (P8::GameStateHasChanged* gshcMsg = dynamic_cast<P8::GameStateHasChanged*>(&aEvent))
	{
		//isPaused = P8::IsGamePaused(gshcMsg->newGameState);
		if (gshcMsg->newGameState == eGameStates::ePlayCountdown || gshcMsg->newGameState == eGameStates::ePlayTutorial)
		{
			//Vector3f startPosition = CalculateStartPosition();
			//
			//myGameObject.myTransform.SetPosition(startPosition);
		}
	}

	else if (P8::CameraShakeEvent* csMsg = dynamic_cast<P8::CameraShakeEvent*>(&aEvent))
	{
		myShakeFactor = csMsg->data.shakefactor;
		myCameraShakeTime = csMsg->data.shakeTime;

		myTime = 0.0f;
		isCameraShaking = true;
		cachedPosition = myGameObject.myTransform.GetPosition();
	}
}

void P8::ActionCameraComponent::OnInit()
{
	ES::EventSystem::GetInstance().Attach<P8::ActionCameraEvent>(this);
	ES::EventSystem::GetInstance().Attach<P8::GameStateHasChanged>(this);
	ES::EventSystem::GetInstance().Attach<P8::CameraShakeEvent>(this);
}

void P8::ActionCameraComponent::OnDestroy()
{
	ES::EventSystem::GetInstance().Detach<P8::ActionCameraEvent>(this);
	ES::EventSystem::GetInstance().Detach<P8::GameStateHasChanged>(this);
	ES::EventSystem::GetInstance().Detach<P8::CameraShakeEvent>(this);
}

void P8::ActionCameraComponent::DrawScreenSpaceLines()
{
	Vector2i res = *KE_GLOBAL::resolution;

	const float outerPerc = ((mySettings.outerBorderDistancePercentage * 0.5f) / 100.0f);

	Vector2f topLeft = { res.x * outerPerc, res.y * (1.0f - outerPerc) };
	Vector2f topRight = { res.x * (1.0f - outerPerc), res.y * (1.0f - outerPerc) };

	Vector2f bottomLeft = { res.x * outerPerc, res.y * outerPerc };
	Vector2f bottomRight = { res.x * (1.0f - outerPerc), res.y * outerPerc };

	static Vector4f outercolour = { 1.0, 0.0f, 0.0f, 1.0f };

	myDBGRenderer->RenderScreenSpaceLine(topLeft, topRight, outercolour);
	myDBGRenderer->RenderScreenSpaceLine(topRight, bottomRight, outercolour);
	myDBGRenderer->RenderScreenSpaceLine(bottomRight, bottomLeft, outercolour);
	myDBGRenderer->RenderScreenSpaceLine(bottomLeft, topLeft, outercolour);


	const float innerPerc = (((mySettings.outerBorderDistancePercentage + mySettings.innerBorderDistancePercentage) * 0.5f) / 100.0f);

	Vector2f topInnerLeft = { res.x * innerPerc, res.y * (1.0f - innerPerc) };
	Vector2f topInnerRight = { res.x * (1.0f - innerPerc), res.y * (1.0f - innerPerc) };

	Vector2f bottomInnerLeft = { res.x * innerPerc, res.y * innerPerc };
	Vector2f bottomInnerRight = { res.x * (1.0f - innerPerc), res.y * innerPerc };

	static Vector4f innercolour = { 0.0, 1.0f, 0.0f, 1.0f };

	myDBGRenderer->RenderScreenSpaceLine(topInnerLeft, topInnerRight, innercolour);
	myDBGRenderer->RenderScreenSpaceLine(topInnerRight, bottomInnerRight, innercolour);
	myDBGRenderer->RenderScreenSpaceLine(bottomInnerRight, bottomInnerLeft, innercolour);
	myDBGRenderer->RenderScreenSpaceLine(bottomInnerLeft, topInnerLeft, innercolour);
}

void P8::ActionCameraComponent::GetWPTargetbounds(OUT Vector3f& aMin, OUT Vector3f& aMax)
{
	if (myTargets.size() < 1) { return; }

	aMin = { FLT_MAX, 0, FLT_MAX };
	aMax = { -FLT_MAX, 0, -FLT_MAX };

	for (int i = 0; i < myTargets.size(); i++)
	{
		Vector3f pos = myTargets[i]->myTransform.GetPosition();

		if (pos.x > aMax.x) { aMax.x = pos.x; }
		if (pos.z > aMax.z) { aMax.z = pos.z; }
		if (pos.x < aMin.x) { aMin.x = pos.x; }
		if (pos.z < aMin.z) { aMin.z = pos.z; }
	}
}

const Vector3f P8::ActionCameraComponent::ShakeCamera()
{
	Vector4f shakeVector;

	float calcShakeFact = myShakeFactor * (1.0f - myTime * myCameraShakeTime);

	if (calcShakeFact < 0.0001f)
	{
		calcShakeFact = 0.0f;
	}

	shakeVector.x = KE::GetRandomFloat(-calcShakeFact, calcShakeFact, 100000.0f);
	shakeVector.y = KE::GetRandomFloat(-calcShakeFact, calcShakeFact, 100000.0f);

	shakeVector = myGameObject.myTransform.GetCUMatrix() * shakeVector;

	return { shakeVector.x, shakeVector.y, shakeVector.z };
}

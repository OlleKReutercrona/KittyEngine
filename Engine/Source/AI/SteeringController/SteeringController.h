#pragma once
#include <Engine/Source/Utility/Global.h>
#include <Engine/Source/AI/Pathfinding/Pathfinder.h>
#include <Engine/Source/AI/SteeringController/SteeringOutput.h>
#include "Kinematic.h"
#include "Face.h"
#include "Separation.h"
#include "Arrive.h"

namespace KE_EDITOR
{
	class AIMovement;
}

class SteeringController
{
	friend class KE_EDITOR::AIMovement;

public:
	
	SteeringController(Transform& aGoTransform) : 
		myKinematic(aGoTransform),
		myFace(this->myKinematic), 
		myArrive(this->myKinematic),
		mySeparation(this->myKinematic)
	{
	
	}

	inline void SetPathfinder(KE::Pathfinder* aPathfinder) { myPathfinder = aPathfinder; }
	inline void ApplySteering(float aTimeDelta);
	inline void UpdateSteering();
	inline void MoveTowards(const Vector3f& aTarget);
	inline void FaceTowards(const Vector3f& aTarget);
	inline void ClearSteering() { mySteeringOutput.linear = {}; myKinematic.velocity = {}; }
	inline const Vector3f& GetVelocity() { return myKinematic.velocity; }

private:
	KE::Pathfinder* myPathfinder = nullptr;
	std::vector<Vector2f> myPath;
	SteeringOutput mySteeringOutput;
	Kinematic myKinematic;
	Separation mySeparation;
	Arrive myArrive;
	Face myFace;
};

inline void SteeringController::UpdateSteering()
{
	SteeringOutput result;
	result += myArrive.GetSteering();
	result += mySeparation.GetSteering();
	result += myFace.GetSteering(KE_GLOBAL::deltaTime);

	mySteeringOutput = result;
}

inline void SteeringController::ApplySteering(float aTimeDelta)
{
	myKinematic.Update(mySteeringOutput, aTimeDelta);

	Vector3f position = myKinematic.transform.GetPosition();

	position.y = myPathfinder->GetHeightByPos(position);
	myKinematic.transform.SetPosition(position);
}

inline void SteeringController::MoveTowards(const Vector3f& aTarget)
{
	Vector3f position = myKinematic.transform.GetPosition();
	myPath = myPathfinder->FindPath_2D(position, aTarget);

	if (myPath.size() > 0) {

		Vector2f targetPos = myPath.back();

		myFace.faceTarget = { targetPos.x, 0.0f, targetPos.y };
		myArrive.target = { targetPos.x, 0.0f, targetPos.y };
		myArrive.SetFinalTarget(myPath.front());
	}
}

inline void SteeringController::FaceTowards(const Vector3f& aTarget)
{
	myFace.faceTarget = aTarget;
}
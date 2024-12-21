#include "stdafx.h"
#include "Portal.h"
#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Project/Source/Player/Player.h"
#include "Engine/Source/ComponentSystem/Components/Collider/BoxColliderComponent.h"
#include "Engine/Source/Collision/Collider.h"
#include "Project/Source/Boomerang/BoomerangComponent.h"
#include "Engine/Source/ComponentSystem/GameObjectManager.h"
#include "Project/Source/Managers/BallManager.h"
#include "Engine/Source/ComponentSystem/Components/Graphics/VFXComponent.h"
#include "Engine/Source/Audio/GlobalAudio.h"

namespace P8
{
	P8::Portal::Portal(KE::GameObject& aGameObject) : KE::Component(aGameObject)
	{

	}

	P8::Portal::~Portal()
	{
		OnDestroy();
	}

	void P8::Portal::Awake()
	{
		auto& manager = myGameObject.GetManager();

		if (auto* gO = manager.GetGameObject(myConnectedPortalID))
		{
			myLinkedPortal = &gO->GetComponent<Portal>();
		}
		if(auto* gamesystems = manager.GetGameObject(KE::eGameSystemManager))
		{
			myBallManager = &gamesystems->GetComponent<BallManager>();
		}
		if (auto* vfx = &myGameObject.GetComponent<KE::VFXComponent>())
		{
			vfx->TriggerAllVFX(true, false);
		}

		OnInit();
	}

	void P8::Portal::SetData(void* aData)
	{
		PortalData* data = static_cast<PortalData*>(aData);

		myConnectedPortalID = data->linkedPortalID;
	}

	void P8::Portal::Update()
	{
		CollectPortableObjects();

		auto players = myBallManager->GetPlayers();
		auto balls = myBallManager->GetBalls();

		const Vector3f forward = myGameObject.myTransform.GetForward();
		const Vector3f position = myGameObject.myTransform.GetPosition();

		for (auto& key : myPortedObjects)
		{
			PortedObject& portedObject = key.second;
			portedObject.time += KE_GLOBAL::deltaTime;

			if (portedObject.time < 0.1f) { continue; }

			Vector3f objVelocity;
			if (portedObject.type == PortedObject::eType::Player)
			{
				objVelocity = portedObject.object->GetComponent<Player>().GetVelocityRef();
			}
			else if (portedObject.type == PortedObject::eType::Ball)
			{
				objVelocity = portedObject.object->GetComponent<BoomerangComponent>().GetVelocity();
			}

			if (IsInPortal(position, forward, *portedObject.object, objVelocity))
			{
				portedObject.time = 0.0f;
				Teleport(*portedObject.object);
			}
		}
	}

	void Portal::Teleport(KE::GameObject& aGo)
	{
		if (!myLinkedPortal) { return; }

		KE::GlobalAudio::PlaySFX(sound::SFX::PortalTeleport);

		Vector3f linkedPortalPosition = myLinkedPortal->myGameObject.myTransform.GetPosition();
		Vector3f linkedPortalFoward = myLinkedPortal->myGameObject.myTransform.GetForward();
		Vector3f offset = linkedPortalFoward * 0.0f;

		Vector3f* velocityPtr = nullptr;

		Vector3f teleportedPosition = GetTransformedPosition(this, myLinkedPortal, aGo.myWorldSpaceTransform.GetPosition());

		if (auto* player = &aGo.GetComponent<Player>())
		{
			player->SetPosition(teleportedPosition);
			velocityPtr = &player->GetVelocityRef();
		}
		else if (auto* boomerang = &aGo.GetComponent<BoomerangComponent>())
		{
			boomerang->SetPosition(teleportedPosition);
			velocityPtr = &boomerang->GetVelocity();
		}
		if (!velocityPtr) { return; }

		const float speed = velocityPtr->Length();
		*velocityPtr = GetTransformedDirection(this, myLinkedPortal, velocityPtr->GetNormalized()) * speed;
	}

	bool Portal::IsInPortal(const Vector3f& aPortalPos, const Vector3f& aPortalDir, KE::GameObject& aGo, const Vector3f& aVelocity)
	{
		const Vector3f otherPosition = aGo.myTransform.GetPosition();

		const Vector3f delta = otherPosition - aPortalPos;
		const Vector3f direction = delta.GetNormalized();
		const float distance = delta.Length();

		const bool isInFront = aPortalDir.Dot(delta) > 0.0f;

		//project otherPosition onto forward
		const Vector3f alongForward = delta.Project(aPortalDir);
		const Vector3f portalRight  = aPortalDir.Cross({ 0.0f, 1.0f, 0.0f });
		const Vector3f alongRight   = delta.Project(portalRight);

		constexpr float maxForwardLength = 0.4f;
		constexpr float maxRightLength = 1.0f;

		const float alongForwardLengthSqr = alongForward.LengthSqr();
		const float alongRightLengthSqr = alongRight.LengthSqr();

		const bool withinForwardLength = alongForwardLengthSqr <= maxForwardLength * maxForwardLength;
		const bool withinRightLength = alongRightLengthSqr <= maxRightLength * maxRightLength;

		const float velocityDot = aVelocity.Dot(aPortalDir);
		const bool isMovingTowards = velocityDot < 0.0f;

		return (isInFront && isMovingTowards && withinForwardLength && withinRightLength);
	}
	void Portal::CollectPortableObjects()
	{
		auto players = myBallManager->GetPlayers();
		auto balls = myBallManager->GetBalls();

		for (auto* player : players)
		{
			if(!player) continue;
			int id = player->GetGameObject().myID;
			if (myPortedObjects.find(id) == myPortedObjects.end())
			{
				PortedObject portedObject;
				portedObject.object = &player->GetGameObject();
				portedObject.type = PortedObject::Player;
				portedObject.time = 0.0f;
				myPortedObjects[id] = portedObject;
			}
		}
		for (auto* ball : balls)
		{
			if (!ball) continue;
			int id = ball->GetGameObject().myID;
			if (myPortedObjects.find(id) == myPortedObjects.end())
			{
				PortedObject portedObject;
				portedObject.object = &ball->GetGameObject();
				portedObject.type = PortedObject::Ball;
				portedObject.time = 0.0f;
				myPortedObjects[id] = portedObject;
			}
		}
	}

	Vector3f Portal::GetTransformedDirection(const Portal* aFromPortal, const Portal* aToPortal, const Vector3f& aDirection)
	{
		const Vector3f toPortalForward = aToPortal->myGameObject.myTransform.GetForward();

		const float directionAngle = aFromPortal->myGameObject.myTransform.GetForward().Dot(aDirection);
		const float directionSide = aFromPortal->myGameObject.myTransform.GetRight().Dot(aDirection) <= 0.0f ? 1.0f : -1.0f;
		const float angleRadians = acos(directionAngle < -1.0f ? -1.0f : directionAngle > 1.0f ? 1.0f : directionAngle);

		const Matrix3x3f rotation = Matrix3x3f::CreateRotationAroundY(angleRadians * directionSide);
		const Vector3f transformedDirection = (rotation * toPortalForward).GetNormalized();
		const Vector3f reflectedDirection = transformedDirection - 2.0f * transformedDirection.Dot(toPortalForward) * toPortalForward;

		return reflectedDirection;
	}

	Vector3f Portal::GetTransformedPosition(const Portal* aFromPortal, const Portal* aToPortal, const Vector3f& aPosition)
	{
		const auto& fromTransform = aFromPortal->GetGameObject().myWorldSpaceTransform;
		const Vector3f fromForward = fromTransform.GetForward();
		const Vector3f fromRight = fromTransform.GetRight();
		const Vector3f fromDelta = aPosition - fromTransform.GetPosition();
		const float rightSide = fromRight.Dot(fromDelta) <= 0.0f ? 1.0f : -1.0f;
		const float forwardSide = fromForward.Dot(fromDelta) <= 0.0f ? 1.0f : -1.0f;

		const auto& toTransform = aToPortal->GetGameObject().myWorldSpaceTransform;
		const Vector3f toForward = toTransform.GetForward();
		const Vector3f toRight = toTransform.GetRight();

		const Vector3f alongForward = fromDelta.Project(fromForward);
		const Vector3f alongRight = fromDelta.Project(fromRight);

		Vector3f newPosition = toTransform.GetPosition();
		newPosition += toForward * alongForward.Length() * -forwardSide;
		newPosition += toRight * alongRight.Length() * rightSide;
		return newPosition;
	}

	void Portal::DrawDebug(KE::DebugRenderer& aDbg)
	{
		const auto& players = myBallManager->GetPlayers();
		if (players.empty()) { return; }

		const Vector3f portalPos = myGameObject.myWorldSpaceTransform.GetPosition();
		const Vector3f linkedPortalPos = myLinkedPortal->GetGameObject().myWorldSpaceTransform.GetPosition();

		const Vector3f playerPos = players[0]->GetGameObject().myTransform.GetPosition();

		const bool isClosest = (portalPos - playerPos).LengthSqr() < (linkedPortalPos - playerPos).LengthSqr();
		const Vector3f playerToLinked = myLinkedPortal->GetGameObject().myWorldSpaceTransform.GetPosition() - playerPos;

		if (isClosest)
		{
			aDbg.RenderLine(playerPos, portalPos, { 0.0f, 1.0f, 0.0f, 1.0f });
		}
		else
		{
			const Vector3f teleportedDirection = GetTransformedDirection(myLinkedPortal, this, playerToLinked.GetNormalized());

			aDbg.RenderLine(portalPos, portalPos + teleportedDirection * playerToLinked.Length(), {0.0f, 1.0f, 0.0f, 1.0f});

		}
	}

	void Portal::OnReceiveEvent(ES::Event& aEvent)
	{
		if (auto* gameStateEvent = dynamic_cast<GameStateHasChanged*>(&aEvent))
		{
			if (gameStateEvent->newGameState == eGameStates::eMenuMain)
			{
				myPortedObjects.clear();
			}
		}
	}

	void Portal::OnInit()
	{
		ES::EventSystem::GetInstance().Attach<GameStateHasChanged>(this);
	}

	void Portal::OnDestroy()
	{
		ES::EventSystem::GetInstance().Detach<GameStateHasChanged>(this);
	}
}

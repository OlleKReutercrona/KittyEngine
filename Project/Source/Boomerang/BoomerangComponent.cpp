#include "stdafx.h"
#include "Boomerang/BoomerangComponent.h"

#include <format>

#include "Project/Source/Boomerang/BoomerangPhysxController.h"
#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/ComponentSystem/Components/Graphics/ModelComponent.h"
#include "Engine/Source/ComponentSystem/Components/Graphics/SkeletalModelComponent.h"
#include "Engine/Source/ComponentSystem/Components/Graphics/VFXComponent.h"
#include "Engine/Source/Graphics/DebugRenderer.h"
#include "Managers/BallManager.h"
#include "Player/Player.h"

#include "Project/Source/GameEvents/GameEvents.h"
#include "Project/Source/Managers/GameManager.h"
#include "Engine/Source/Audio/GlobalAudio.h"
#include "Engine/Source/ComponentSystem/Components/LightComponent.h"

namespace P8
{
	BoomerangComponent::BoomerangComponent(KE::GameObject& aGameObject) : KE::Component(aGameObject) 
	{
		BoomerangComponent::OnInit();
	}
	BoomerangComponent::~BoomerangComponent()
	{
		delete myPhysxController; 
	}

	void BoomerangComponent::Awake()
	{
		// CharacterController's are not colliders, but they need a component defined for the collision system to trigger onContacts.
		if (myPhysxController) {
			KE::PhysxShapeUserData userData;
			userData.myID = myGameObject.myID;
			userData.gameObject = &myGameObject;
			myPhysxController->GetPhysicsObject().myKECollider.SetPhysxUserData(&userData);
			myPhysxController->GetPhysicsObject().myKECollider.myComponent = this;
		}
	}

	void BoomerangComponent::SetData(void* aDataObject)
	{
		if (!aDataObject) { return; }
		auto* data = static_cast<BoomerangComponentData*>(aDataObject);

		myPhysxController = data->controller;
		myPhysxController->SetBoomerangComponent(this);
	}

	void BoomerangComponent::Update()
	{
		if (P8::GameManager::IsPaused()) { return; }
		if (myThrowParameters.team >= 999) { return; }

		auto oldState = myThrowParameters.state;
		myPhysxController->Update(KE_GLOBAL::deltaTime);

		if (myThrowParameters.state == BoomerangState::Recalled)
		{
			myPhysxController->Recall();
		}

		//if (myThrowParameters.state == BoomerangState::Die)
		//{
		//	if (KE::VFXComponent* vfxComponent; myGameObject.TryGetComponent<KE::VFXComponent>(vfxComponent))
		//	{
		//		vfxComponent->StopVFX(myThrowParameters.throwerPlayer->GetCharacterIndex() - 1);
		//	}
		//}

		if (oldState != BoomerangState::PickedUp && myThrowParameters.state == BoomerangState::PickedUp)
		{
			myThrowParameters.throwerPlayer->GetBoomerangs()->Pickup(this);
		}

		if (myThrowParameters.state == BoomerangState::Flying || 
			myThrowParameters.state == BoomerangState::Recalled || 
			myThrowParameters.state == BoomerangState::Telekinesis
			)
		{
			for (auto& player : myBallManager->GetPlayers())
			{
				auto& playerObj = player->GetGameObject();

				Vector3f playerPos = playerObj.myWorldSpaceTransform.GetPosition();
				playerPos.y = 0.0f;

				Vector3f boomerangPos = myGameObject.myWorldSpaceTransform.GetPosition();
				boomerangPos.y = 0.0f;

				const float distanceSqr = (playerPos - boomerangPos).LengthSqr();

				if (myThrowParameters.team == player->GetTeam()) { continue; }

				if (distanceSqr < myThrowParameters.damageRadius * myThrowParameters.sizeMult)
				{
					player->TakeDamage(myThrowParameters.throwerPlayer->GetIndex());
				}
			}
		}
	}

	void BoomerangComponent::Throw(const Vector3f& aFromPos, const Vector3f& aVelocity, BallThrowParams& throwParams)
	{
		if (!IsPickedUp()) { return; }

		GetGameObject().SetActive(true);
		OnThrow();
		myThrowParameters = throwParams;

		myPhysxController->GetUserData()->team = myThrowParameters.team;
		myPhysxController->Throw(aVelocity, aFromPos, myThrowParameters);
		SetSize(myThrowParameters.size);
	}

	void BoomerangComponent::SetPosition(const Vector3f& aPosition)
	{
		myPhysxController->SetPosition(aPosition);
		myGameObject.myTransform.SetPosition(aPosition);
	}

	void BoomerangComponent::OnReceiveEvent(ES::Event& aEvent)
	{
		if (P8::GameStateHasChanged* gshcMsg = dynamic_cast<P8::GameStateHasChanged*>(&aEvent))
		{
			isPaused = P8::IsGamePaused(gshcMsg->newGameState);
		}
	}

	void BoomerangComponent::OnInit()
	{
		ES::EventSystem::GetInstance().Attach<P8::GameStateHasChanged>(this);
	}

	void BoomerangComponent::OnDestroy()
	{
		ES::EventSystem::GetInstance().Detach<P8::GameStateHasChanged>(this);
	}

	void BoomerangComponent::BeginRecall()
	{
		myPhysxController->Recall();
		myThrowParameters.oldState = myThrowParameters.state;
		myThrowParameters.state = BoomerangState::Recalled;

		if (KE::VFXComponent* vfxComponent; myGameObject.TryGetComponent<KE::VFXComponent>(vfxComponent))
		{
			vfxComponent->TriggerVFX(5, true);
		}
	}

	void BoomerangComponent::EndRecall()
	{
		if (KE::VFXComponent* vfxComponent; myGameObject.TryGetComponent<KE::VFXComponent>(vfxComponent))
		{
			vfxComponent->StopVFX(5);
		}

		if (myThrowParameters.state == BoomerangState::PickedUp) { return; }
		myThrowParameters.state = myThrowParameters.oldState;
	}

	void BoomerangComponent::OnThrow()
	{
		if (KE::ModelComponent* modelComponent; myGameObject.TryGetComponent<KE::ModelComponent>(modelComponent))
		{
			modelComponent->SetActive(true);
		}
		if (KE::VFXComponent* vfxComponent; myGameObject.TryGetComponent<KE::VFXComponent>(vfxComponent))
		{
			vfxComponent->TriggerVFX(myThrowParameters.throwerPlayer->GetCharacterIndex() -1, true);
		}

		P8::ActionCameraEvent msg(myGameObject, true);
		ES::EventSystem::GetInstance().SendEvent<P8::ActionCameraEvent>(msg);
	}

	void BoomerangComponent::OnPickup()
	{
		if (KE::ModelComponent* modelComponent; myGameObject.TryGetComponent<KE::ModelComponent>(modelComponent))
		{
			modelComponent->SetActive(false);
		}
		if (KE::VFXComponent* vfxComponent; myGameObject.TryGetComponent<KE::VFXComponent>(vfxComponent))
		{
			vfxComponent->StopVFX(myThrowParameters.throwerPlayer->GetCharacterIndex() - 1);
		}

		P8::ActionCameraEvent msg(myGameObject, false);
		ES::EventSystem::GetInstance().SendEvent<P8::ActionCameraEvent>(msg);

		myPhysxController->SetPosition({ -9999.0f, -999.0f, -99999.0f });

		myThrowParameters.state = BoomerangState::PickedUp;
	}

	void BoomerangComponent::DrawDebug(KE::DebugRenderer& aDbg)
	{
		myPhysxController->DebugRender();
	}

	bool BoomerangComponent::IsPickedUp() const
	{
		return myThrowParameters.state == BoomerangState::PickedUp;
	}

	void BoomerangComponent::Kill()
	{
		//myPhysxController->SetVelocity({0.0f, 0.0f, 0.0f});
		myThrowParameters.state = BoomerangState::Die;
	}

	Vector3f& BoomerangComponent::GetVelocity()
	{
		return myPhysxController->GetVelocity();
	}

	const Vector3f& BoomerangComponent::GetVelocity() const
	{
		return myPhysxController->GetVelocity();
	}

	void BoomerangComponent::Deflect(const Vector3f& aDeflectionDirection)
	{
		if (myThrowParameters.state == BoomerangState::PickedUp)   { return; }
		if (myThrowParameters.state == BoomerangState::PickupAnim) { return; }
		if (myThrowParameters.state == BoomerangState::Die)		   { return; }

		myPhysxController->Deflect(aDeflectionDirection.GetNormalized());

		KE::GlobalAudio::PlaySFX(sound::SFX::BallParry);

		P8::SlowMotionEventData smed;
		smed.timeModifier = 0.35f;
		P8::SlowMotionEvent sme(smed);
		ES::EventSystem::GetInstance().SendEvent<P8::SlowMotionEvent>(sme);
	}

	void BoomerangComponent::SyncCharacterIndex(Player* aPlayer)
	{
		myThrowParameters.throwerPlayer = aPlayer;
		if (KE::ModelComponent* modelComponent; myGameObject.TryGetComponent<KE::ModelComponent>(modelComponent))
		{
			modelComponent->SetActive(false);

			if (KE::SkeletalModelComponent* playerModelComponent; aPlayer->GetGameObject().TryGetComponent<KE::SkeletalModelComponent>(playerModelComponent))
			{
				modelComponent->GetModelData()->myRenderResources[0].myMaterial = playerModelComponent->GetModelData()->myRenderResources[0].myMaterial;
			}
		}

		const Vector3f myCharacterColours[4] = {
			{0.24f, 0.78f, 1.0f}, // Yellow e9c936
			{0.0f , 1.0f , 0.15f}, // Green 4b8852
			{0.94f, 0.36f, 0.58f}, // Pink f05b94
			{0.42f, 0.27f, 0.67f}  // Purple 6c46ab
		}; 

		int lightIndex = aPlayer->GetCharacterIndex() - 1;
		if (std::size(myCharacterColours) > lightIndex)
		{
			SetColour(myCharacterColours[lightIndex]);
		}

	}

	void BoomerangComponent::SetSize(float aSize)
	{
		myThrowParameters.size = aSize;

		float scaledSize = aSize * myThrowParameters.sizeMult;
		myGameObject.myTransform.SetScale({scaledSize,scaledSize,scaledSize});
		//myPhysxController->SetRadius(aSize);

		if (KE::LightComponent* lightComponent; myGameObject.TryGetComponent<KE::LightComponent>(lightComponent))
		{
			KE::PointLightData* light = static_cast<KE::PointLightData*>(lightComponent->GetLightData()->myLightData);
			light->myRange = 2.5f * scaledSize;
		}
	}

	void BoomerangComponent::SetColour(const Vector3f& aColour)
	{
		if (KE::LightComponent* lightComponent; myGameObject.TryGetComponent<KE::LightComponent>(lightComponent))
		{
			KE::PointLightData* light = static_cast<KE::PointLightData*>(lightComponent->GetLightData()->myLightData);
			light->myColour = { aColour.x, aColour.y, aColour.z};

		}
	}

	void BoomerangComponent::OnPhysXCollision(const KE::PhysXCollisionData& aPhysXCollisionData)
	{
		P8::Player* playerComponent = nullptr;
		BoomerangComponent* boomerangComponent = nullptr;

		int myIndex = myGameObject.myID - (-9999);
		if (aPhysXCollisionData.objHit->TryGetComponent<Player>(playerComponent))
		{
			//KE_LOG("Boomerang %i collided with Player %i", myIndex, static_cast<int>(playerComponent->GetIndex()));
		}
		else if (aPhysXCollisionData.objHit->TryGetComponent<BoomerangComponent>(boomerangComponent))
		{
			//KE_LOG("Boomerang %i collided with Boomerang %i", myIndex, aPhysXCollisionData.objHit->myID - (-9999));
		}
	}

	void BoomerangComponent::OnDisable()
	{
		EndRecall();
	}
}

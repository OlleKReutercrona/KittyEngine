#include "stdafx.h"
#include "BattleArea.h"
#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/ComponentSystem/GameObjectManager.h"
#include "Project/Source/Managers/BallManager.h"
#include "Engine/Source/Audio/GlobalAudio.h"

namespace P8
{
	BattleArea::BattleArea(KE::GameObject& aGo) : KE::Component(aGo) {}
	BattleArea::~BattleArea() {
	};

	void BattleArea::Awake()
	{
		myEffectInterface.manager = &KE_GLOBAL::blackboard.Get<KE::Graphics>()->GetVFXManager();
		KE_GLOBAL::blackboard.Get<KE::Graphics>()->GetShaderLoader().GetPixelShader(SHADER_LOAD_PATH "BattleArea_PS.cso");
		myEffectInterface.AddVFX("BattleAreaBounds");
		KE::VFXRenderInput in(myGameObject.myWorldSpaceTransform, true);
		myEffectInterface.TriggerVFXSequence(0, in);

		OnInit();
	}

	void BattleArea::SetData(void* aDataObject)
	{
		BattleAreaData* data = static_cast<BattleAreaData*>(aDataObject);

		myShrinkDuration = data->shrinkDuration;
		myStartShrinkTime = data->shrinkDelay;
		myShouldShrink = data->shouldShrink;

		myWidth = myGameObject.myTransform.GetScale().x;
		myDepth = myGameObject.myTransform.GetScale().z;
		myCurrentWidth = myGameObject.myTransform.GetScale().x;
		myCurrentDepth = myGameObject.myTransform.GetScale().z;
	}

	void BattleArea::Update()
	{
		if (!myPlayersCollected) {
			CollectPlayers();
		}

		if (myShouldShrink) {
			ShrinkArea();
		}

		// Check if players are outside of the battle area //
		bool unitOutside = false;
		for (auto& unit : myBattleUnits)
		{
			if (!unit.player || unit.player->IsCurrentState<PlayerDeathState>()) { continue; }

			if (!IsOutsideArea(unit.player->GetGameObject().myTransform.GetPosition())) 
			{ 
				unit.player->SetDeathzoneTime(0.0f);
				unit.gamepadRumbleTimer = 0.5f;
				continue; 
			}
			unitOutside = true;

			float timer = unit.player->GetDeathzoneTime();
			timer += KE_GLOBAL::deltaTime;
			unit.player->SetDeathzoneTime(timer);

			unit.gamepadRumbleTimer += KE_GLOBAL::deltaTime;

			if (unit.gamepadRumbleTimer >= 0.5f)
			{
				unit.gamepadRumbleTimer = 0.0f;
				float rumbleIntensity = std::clamp(timer / myKillThreshold, 0.1f, 0.5f);
				GamepadRumbleEvent rumbleEvent(unit.player->GetControllerID(), rumbleIntensity, rumbleIntensity, 0.5f, KE::RumbleType::Timed);
				ES::EventSystem::GetInstance().SendEvent<GamepadRumbleEvent>(rumbleEvent);
			}

			if (timer > myKillThreshold)
			{
				int playerIndex = unit.player->GetIndex();
				unit.player->TakeDamage(playerIndex, true);
			}
		}

		if (unitOutside)
		{
			if (!KE::GlobalAudio::IsSFXPlaying(sound::SFX::DeathzoneTick))
			{
				KE::GlobalAudio::PlaySFX(sound::SFX::DeathzoneTick);
			}
		}
		else
		{
			KE::GlobalAudio::StopSFX(sound::SFX::DeathzoneTick);
		}
	}


	void BattleArea::ShrinkArea()
	{
		myShrinkTimer += KE_GLOBAL::deltaTime;

		// Update size of battle area //
		if (myShrinkTimer > myStartShrinkTime)
		{
			float size = 1.0f - (myShrinkTimer - myStartShrinkTime) / myShrinkDuration;
			size = std::clamp(size, 0.0f, 1.0f);

			myCurrentWidth = myWidth * size;
			myCurrentDepth = myDepth * size;

			myGameObject.myTransform.SetScale({ myCurrentWidth, 1.0f, myCurrentDepth });
		}
	}

	void BattleArea::DrawDebug(KE::DebugRenderer& aDbg)
	{
		Vector4f color = { 1.0f, 1.0f, 1.0f, 1.0f };
		Vector3f scale = {1.0f, 0.3f, 1.0f};

		aDbg.RenderCube(myGameObject.myTransform, scale, color);
	}

	bool BattleArea::IsOutsideArea(const Vector3f& aPosition) const
	{
		Matrix4x4f inverse = DirectX::XMMatrixInverse(nullptr, myGameObject.myWorldSpaceTransform.GetMatrix());

		Vector4f pos4 = Vector4f(aPosition, 1.0f);
		Vector3f transformedPos = (pos4 * inverse).xyz();

		if (transformedPos.x > 0.5f || transformedPos.x < -0.5f || transformedPos.z > 0.5f || transformedPos.z < -0.5f)
		{
			return true;
		}

		return false;
	}
	void BattleArea::CollectPlayers()
	{
		if (auto* ballManagerGo = myGameObject.GetManager().GetGameObjectWithComponent<BallManager>())
		{
			auto* ballManager = &ballManagerGo->GetComponent<BallManager>();

			for (auto& player : ballManager->GetPlayers())
			{
				myBattleUnits.push_back({ player });
				myPlayersCollected = true;
			}
		}
	}

	void BattleArea::OnEnable()
	{
		KE::VFXRenderInput in(myGameObject.myWorldSpaceTransform, true);
		myEffectInterface.TriggerVFXSequence(0, in);
	}

	void BattleArea::OnDisable()
	{
		KE::VFXRenderInput in(myGameObject.myWorldSpaceTransform, true);
		myEffectInterface.StopVFXSequence(0, in);
	}

	void BattleArea::OnDestroy()
	{
		KE::VFXRenderInput in(myGameObject.myWorldSpaceTransform, true);
		myEffectInterface.StopVFXSequence(0, in);

		ES::EventSystem::GetInstance().Detach<GameStateHasChanged>(this);
	}

	void BattleArea::OnReceiveEvent(ES::Event& aEvent)
	{
		if (GameStateHasChanged* gameStateEvent = dynamic_cast<GameStateHasChanged*>(&aEvent))
		{
			if (gameStateEvent->newGameState == eGameStates::eMenuMain)
			{
				myBattleUnits.clear();
				myPlayersCollected = false;
			}
		}
	}

	void BattleArea::OnInit()
	{
		ES::EventSystem::GetInstance().Attach<GameStateHasChanged>(this);
	}
}

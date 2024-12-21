#pragma once
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include "Project/Source/Boomerang/BallThrowParams.h"
#include "Engine/Source/Utility/EventSystem.h"

namespace P8
{
	class BallManager;
	class Player;
	class BoomerangPhysicsController;

	struct BoomerangComponentData
	{
		BoomerangPhysicsController* controller = nullptr;
		P8::Player* player = nullptr;
	};

	class BoomerangComponent : public KE::Component, public ES::IObserver
	{
		KE_EDITOR_FRIEND

	public:
		BoomerangComponent(KE::GameObject& aGameObject);
		~BoomerangComponent() override;

		void Awake() override;
		void SetData(void* aDataObject = nullptr) override;
		void Update() override;
		void DrawDebug(KE::DebugRenderer& aDbg) override;

		void Throw(const Vector3f& aFromPos, const Vector3f& aDirection, BallThrowParams& throwParams);
		void Deflect(const Vector3f& aDeflectionDirection);
		void BeginRecall();
		void EndRecall();
		void OnThrow();
		void OnPickup();
		bool IsPickedUp() const;

		void Kill();

		void SyncCharacterIndex(Player* aPlayer);

		void SetBallManager(BallManager* aBallManager) { myBallManager = aBallManager; }
		void SetPosition(const Vector3f& aPosition);

		void OnReceiveEvent(ES::Event& aEvent) override;
		void OnInit() override;
		void OnDestroy() override;

		Vector3f& GetVelocity();
		const Vector3f& GetVelocity() const;
		BoomerangPhysicsController* GetController() const { return myPhysxController; }
		BallThrowParams& GetThrowParameters() { return myThrowParameters; }

		bool IsOriginal() const { return isOriginal; }
		void SetOriginal(bool aIsImportant) { isOriginal = aIsImportant; }
		bool IsFromPowerup() const { return isFromPowerup; }
		void SetFromPowerup(bool aIsFromPowerup) { isFromPowerup = aIsFromPowerup; }

		void SetThrowCooldown(float aCooldown) { throwCooldown = aCooldown; }
		float GetThrowCooldown() const { return throwCooldown; }
		void UpdateThrowCooldown(float aDeltaTime) { throwCooldown -= aDeltaTime; }

		template<class SerializationIO> 
		void Serialize(SerializationIO& aSerializer)
		{
			//SET_REFLECTION(BoomerangComponent);
			//aSerializer & PARAM( myTestBool    )
			//			  & PARAM( myTestInt	 )
			//			  & PARAM( myTestFloat   )
			//			  & PARAM( myTestVector3 )
			//			  & PARAM( myTestVector2 )
			//;
		}

		void SetSize(float aSize);

		void SetColour(const Vector3f& aColour);

	protected:
		void OnPhysXCollision(const KE::PhysXCollisionData& aPhysXCollisionData) override;
		void OnDisable() override;

	private:
		BallManager* myBallManager = nullptr;
		BoomerangPhysicsController* myPhysxController = nullptr;
		BallThrowParams myThrowParameters;

		bool isPaused = false;
		bool isOriginal = false; //important boomerangs are those "owned" by a player, ie the one they start with etc.
		bool isFromPowerup = false;
		float throwCooldown = 0.0f;
	};
}

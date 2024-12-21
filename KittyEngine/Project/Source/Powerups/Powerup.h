#pragma once
#include <format>

#include "Engine/Source/ComponentSystem/GameObject.h"
#include "Engine/Source/ComponentSystem/Components/Component.h"
#include "Engine/Source/Graphics/Graphics.h"
#include "Engine/Source/Graphics/Sprite/Sprite.h"
#include "Engine/Source/Graphics/Text/Text.h"
#include "Engine/Source/Utility/Randomizer.h"

namespace KE
{
	struct VFXPlayerInterface;
}

namespace P8
{
	class Player;
	class BoomerangComponent;

	struct PowerupInputData
	{
		Player* player = nullptr;
		BoomerangComponent* boomerang = nullptr;
	};

	enum class PlayerActionType
	{
		Throw,
		Recall,
		Attack,
		Dash,
		Windup,

		TakeDamage,
		Kill,

		Count
	};

	enum class BoomerangAction
	{
		Throw,
		Recall,
		Fly,
		Bounce,

		Kill,

		Count
	};

	enum class ActionState
	{
		Begin,
		End,
		Ongoing,

		Count, Pressed
	};

	class Powerup;
	struct PowerupIdentityBase
	{
		std::string name;
		std::string icon;
		std::string description;

		std::vector<std::string> vfxNames;

		virtual ~PowerupIdentityBase() = default;
		virtual Powerup* CreateInstance() = 0;
		virtual bool IsType(Powerup* aPowerup) = 0;
	};



	class Powerup
	{
	protected:
		bool isActive = true;
		Player* ownerPlayer = nullptr;
		KE::VFXPlayerInterface* vfxInterface = nullptr;
		std::string name;
		PowerupIdentityBase* identity = nullptr;

	public:
		Powerup();
		virtual ~Powerup();
		void SetPlayer(Player* aPlayer) { ownerPlayer = aPlayer; }
		void SetVFX(std::vector<std::string> aVFXNames);
		void SetName(const std::string& aName) { name = aName; }
		const std::string& GetName() { return name; }
		void SetIdentity(PowerupIdentityBase* anIdentity);
		PowerupIdentityBase* GetIdentity() { return identity; }

		virtual void Update(const PowerupInputData& aInputData) = 0;
		virtual void SetActive(bool aIsActive) { if (isActive == aIsActive) { return; } isActive = aIsActive; isActive ? OnEnable() : OnDisable();};

		virtual void OnPickup()  { __noop; }
		virtual void OnDrop()	 { __noop; }

		virtual void OnEnable()  { __noop; }
		virtual void OnDisable() { __noop; }

		virtual bool OnPlayerAction(PlayerActionType aAction, ActionState aState, PowerupInputData& input)    { return false; };
		virtual bool OnBoomerangAction(BoomerangAction aAction, ActionState aState, PowerupInputData& input) { return false; };

	};

	template <typename T>
	struct PowerupIdentity : public PowerupIdentityBase
	{
		PowerupIdentity(const std::string& aName, const std::string& anIcon, const std::string& aDescription, const std::vector<std::string>& aVFXList)
		{
			name = aName;
			icon = anIcon;
			description = aDescription;
			vfxNames = aVFXList;
		}

		Powerup* CreateInstance() override
		{
			Powerup* out = new T();
			out->SetName(name);
			out->SetVFX(vfxNames);
			out->SetIdentity(this);
			return out;
		}
		bool IsType(Powerup* aPowerup) override { return dynamic_cast<T*>(aPowerup) != nullptr; }
	};

	struct PowerupPickupDisplay
	{
		KE::SpriteBatch powerupTextBatch;
		std::string text;
		std::vector<KE::Sprite*> textSpriteVec;

		KE::SpriteBatch powerupIconBatch;

		float displayTimer;

		Transform* transform = nullptr;

		int ownedPowerupDisplayIndex = 0;
		int ownedPowerupDisplayCount = 0;

		enum class Type
		{
			Pickup,
			Owned
		};
		Type myType = Type::Pickup;
	};


	struct PowerupList;
	class PowerupManager : public KE::Component
	{
	private:
		std::vector<PowerupIdentityBase*> myPowerupIdentities;

		struct PowerupSpawnBounds
		{
			Vector3f min;
			Vector3f max;
		};

		std::vector<PowerupSpawnBounds> myPowerupSpawnBounds;

		float powerupSpawnTimer = 10.0f;
		float powerupSpawnInterval = 15.0f;

		std::vector<PowerupPickupDisplay> myPowerupPickupDisplays;

		KE::SpriteFont powerupDisplayFont;

	public:
		PowerupManager(KE::GameObject& aGameObject);
		~PowerupManager();

		template<typename T>
		void RegisterPowerup(const PowerupIdentity<T>& anIdentity)
		{
			myPowerupIdentities.push_back(new PowerupIdentity<T>(anIdentity));
		}

		inline const std::vector<PowerupIdentityBase*>& GetPowerupIdentities() { return myPowerupIdentities; }

		Vector3f GetPowerupSpawnLocation();
		void SpawnPowerup(const Vector3f& aSpawnPosition = {-999.0f, -999.0f, -999.0f});
		Powerup* GetRandomPowerup(int aPowerupMask);

		void SetupDisplay(PowerupPickupDisplay& aDisplay);
		PowerupPickupDisplay* GetFreeDisplay();
		void DisplayPickup(Powerup* aPowerup, Transform* aTransform);
		void DisplayDrop(Powerup* aPowerup, Transform* aTransform);
		void DisplayOwnedPowerups(PowerupList* aPowerupList);
		void UpdateDisplays();

	protected:
		void Awake() override;
		void Update() override;
		void LateUpdate() override;
	};

	//

	struct PowerupList
	{
		PowerupManager* manager = nullptr;
		std::vector<Powerup*> powerups;
		Player* ownerPlayer = nullptr;

		int maxPowerups = 3;
		

		//void UpdateBatches();
		//void InitBatches();
		//void PrintPowerupText(const std::string& aText);

		void AddPowerup(Powerup* aPowerup, bool aIgnorePowerupLimit = false, bool aDisplayPopup = true);
		void RemovePowerup(Powerup* aPowerup);
		void ClearPowerups();

		bool OnPlayerAction(const PlayerActionType& anAction, const ActionState& aState, PowerupInputData& anInput);
		bool OnBoomerangAction(const BoomerangAction& anAction, const ActionState& aState, PowerupInputData& anInput);
		void Update(PowerupInputData aInput);

		int GetPowerupMask();
		void SetPowerupsFromMask(int mask, bool aDisplayOwned = true);
	};
}


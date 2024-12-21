#pragma once
#include "Boomerang/BallThrowParams.h"
#include "Project/Source/Powerups/Powerup.h"
#include "Engine/Source/Graphics/CBuffer.h"

namespace physx
{
	class PxScene;
}

namespace P8
{
	constexpr unsigned int explosionRangeBlockCount = 16;
	struct ExplosionVFXBufferData
	{
		struct ExplosionVFXBlock
		{
			float angleMin = -1.0f;
			float angleMax = -1.0f;
			float rangeMin = -1.0f;
			float rangeMax = -1.0f;
		} blocks[explosionRangeBlockCount];
	};

	struct HitRange
	{
		float angle = 0.0f;
		float range = 0.0f;
		Vector3f normal = { 0.0f, 0.0f, 0.0f };
	};

	struct Explosion
	{
		constexpr static int slices = 16;
		std::array<Transform*, slices> transforms;
		std::array<float, slices> ranges;
		Vector3f position;

		float explosionTimer = 0.0f;
		float explosionDuration = 0.5f;
		float explosionRange = 3.0f;
		float scaleFactor = 1.0f;

		ExplosionVFXBufferData vfxBufferData;
		Transform vfxTransform;

		//std::vector<ExplosionVFXBufferData::ExplosionVFXBlock> vfxBlocksTemp;
		//std::vector<HitRange> hitRangesTemp;

		Explosion()
		{
			for (int i = 0; i < slices; i++)
			{
				transforms[i] = new Transform();
			}
		}
		Explosion(const Explosion& aExplosion)
		{
			for (int i = 0; i < slices; i++)
			{
				transforms[i] = new Transform(*aExplosion.transforms[i]);
			}
		}
		Explosion(Explosion&& aExplosion) noexcept
		{
			for (int i = 0; i < slices; i++)
			{
				transforms[i] = aExplosion.transforms[i];
				aExplosion.transforms[i] = nullptr;
			}
		}
		Explosion& operator=(const Explosion& aExplosion)
		{
			if (this == &aExplosion) return *this;

			for (int i = 0; i < slices; i++)
			{
								delete transforms[i];
				transforms[i] = new Transform(*aExplosion.transforms[i]);
			}

			return *this;
		}
		~Explosion()
		{
			for (int i = 0; i < slices; i++)
			{
				delete transforms[i];
			}
		}


	};

	class ExplosionBoomerangPowerup : public Powerup
	{
	public:
		ExplosionBoomerangPowerup();
		void Update(const PowerupInputData& aInputData) override;
		void ExplodeBall(PowerupInputData& input, BallThrowParams& throwParams);

		void OnPickup() override { __noop; }
		void OnDrop() override { __noop; }

		void OnEnable() override { __noop; }
		void OnDisable() override { __noop; }

		bool ValidateConditions(BoomerangAction aAction, ActionState aState, PowerupInputData& input);

		bool OnPlayerAction(PlayerActionType aAction, ActionState aState, PowerupInputData& input) override { __noop; return false; }
		bool OnBoomerangAction(BoomerangAction aAction, ActionState aState, PowerupInputData& input) override;

		void InitExplosion(Explosion& anExplosion);
		void UpdateExplosion(Explosion& anExplosion);
		Explosion& GetFreeExplosion();

		void CalculateRanges(Explosion& anExplosion, physx::PxScene* aScene);
	private:
		std::vector<Explosion> explosions;

	public:
		inline static KE::CBuffer ourExplosionVFXBuffer;
		inline static bool ourExplosionVFXBufferInitialised = false;
	};


}

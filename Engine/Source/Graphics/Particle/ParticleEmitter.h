#pragma once

#include <stack>

#include "Engine/Source/Graphics/Sprite/SpriteManager.h"
#include "Engine/Source/Math/Transform.h"

#include <Editor/Source/EditorInterface.h>

namespace KE
{
	class SpriteFont;
	class Graphics;

	struct PerParticleAttributes
	{
		Vector3f velocity = Vector3f(0.0f, 0.0f, 0.0f);
		Vector3f acceleration = Vector3f(0.0f, 0.0f, 0.0f); 
		float lifeTimeRemaining = 0.0f;
		float lifeTimeTotal = 0.0f;
		

	};

	struct SharedParticleAttributes
	{
		float burstTimeMin = 0.01f;
		float burstTimeMax = 0.01f;
		int burstCountMin = 1;
		int burstCountMax = 1;

		float velocityMin = 0;
		float velocityMax = 0;

		float accelerationMin = 0;
		float accelerationMax = 0;
		
		float velocityDegradation = 0;
		float accelerationDegradation = 0;
		
		float lifeTimeMin = 0.1f;
		float lifeTimeMax = 0.1f;
		float lifeTimeMidPoint = 0.5f;

		float angleMin = 0;
		float angleMax = 0;
		
		float horizontalVelocityFactor = 0;
		float verticalVelocityFactor = 0;

		Vector4f startColor = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
		Vector4f midColor = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
		Vector4f endColor = Vector4f(1.0f, 1.0f, 1.0f, 1.0f);

		float startSize = 1.0f;
		float midSize = 1.0f;
		float endSize = 1.0f;
	};
	
	class ParticleEmitter
	{
		KE_EDITOR_FRIEND;
	private:
		unsigned int myParticleCapacity;

		float myTimer;
		float myBurstTimer;

		SpriteBatch mySpriteBatch;		
		std::stack<unsigned int> myFreeParticleIndices;
		std::vector<PerParticleAttributes> myPerParticleAttributes;
		SharedParticleAttributes mySharedParticleAttributes;
		
	public:
		ParticleEmitter();
		~ParticleEmitter();

		void Init(KE::Graphics* aGraphics, unsigned int aParticleCapacity, const std::string& aTexturePath);
		void Update(const Transform& aTransform, bool aAllowBurst = true);

		void Burst(const Transform& aTransform);
		void BurstText(const Transform& aTransform, KE::SpriteFont* aFont, const std::string& aText);
		
		SharedParticleAttributes& GetSharedAttributes() { return mySharedParticleAttributes; }

		inline SpriteBatch* GetSpriteBatch() { return &mySpriteBatch; }
	};
}	
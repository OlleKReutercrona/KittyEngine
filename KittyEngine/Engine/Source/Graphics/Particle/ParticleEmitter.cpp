#include "stdafx.h"
#include "ParticleEmitter.h"

#include "Engine/Source/Graphics/Graphics.h"
#include "Engine/Source/Graphics/Texture/TextureLoader.h"

#include "Engine/Source/Utility/Global.h"
#include "Graphics/Text/Text.h"

#ifndef  KITTYENGINE_NO_EDITOR
#include <Editor/Source/ImGui/ImGuiHandler.h>
#endif // ! KITTYENGINE_NO_EDITOR


KE::ParticleEmitter::ParticleEmitter() : mySpriteBatch(SpriteData{})
{}

KE::ParticleEmitter::~ParticleEmitter()
{
}

void KE::ParticleEmitter::Init(Graphics* aGraphics, unsigned int aParticleCapacity, const std::string& aTexturePath)
{
	mySpriteBatch.myData.myTexture = aGraphics->GetTextureLoader().GetTextureFromPath(aTexturePath);
	myParticleCapacity = aParticleCapacity;

	mySpriteBatch.myInstances.resize(myParticleCapacity);
	myPerParticleAttributes.resize(myParticleCapacity);

	mySpriteBatch.myCustomPS = aGraphics->GetShaderLoader().GetPixelShader(SHADER_LOAD_PATH "Particle_PS.cso");

	//set alpha of all particles to 0
	for (unsigned int i = 0; i < mySpriteBatch.myInstances.size(); ++i)
	{
		mySpriteBatch.myInstances[i].myAttributes.myColor.a = 0.0f;
	}
}

void KE::ParticleEmitter::Update(const Transform& aTransform, bool aAllowBurst)
{
	if (aAllowBurst)
	{
		myTimer += KE_GLOBAL::deltaTime;
		while (myTimer >= myBurstTimer)
		{
			myTimer -= myBurstTimer;
		
			myBurstTimer = GetRandomFloat(
				mySharedParticleAttributes.burstTimeMin, 
				mySharedParticleAttributes.burstTimeMax
			);
		
			Burst(aTransform);
			if (myFreeParticleIndices.empty()) { break; }
		}
	}

	for (unsigned int i = 0; i < mySpriteBatch.myInstances.size(); ++i)
	{
		if (myPerParticleAttributes[i].lifeTimeRemaining > 0.0f)
		{
			float lifeTimeRatio = myPerParticleAttributes[i].lifeTimeRemaining / myPerParticleAttributes[i].lifeTimeTotal;

			myPerParticleAttributes[i].lifeTimeRemaining -= KE_GLOBAL::deltaTime;
			myPerParticleAttributes[i].velocity += myPerParticleAttributes[i].acceleration * KE_GLOBAL::deltaTime;
			mySpriteBatch.myInstances[i].myAttributes.myTransform.TranslateWorld(myPerParticleAttributes[i].velocity * KE_GLOBAL::deltaTime);

			//lerp size between start, mid and end

			float size = std::lerp(
				mySharedParticleAttributes.startSize,
				mySharedParticleAttributes.endSize,
				1.0f - lifeTimeRatio
			);

			auto scl = aTransform.GetScale();

			mySpriteBatch.myInstances[i].myAttributes.myTransform.SetScale(scl * size);
			
		}
		else if (myPerParticleAttributes[i].lifeTimeRemaining > -100.0f)
		{
			myPerParticleAttributes[i].lifeTimeRemaining = -100.0f;
			mySpriteBatch.myInstances[i].myAttributes.myTransform.SetPosition({-10000.0f, -10000.0f, -10000.0f});
			myFreeParticleIndices.push(i);
		}
	}
	

	
}

void KE::ParticleEmitter::Burst(const Transform& aTransform)
{
	for (int i = 0; i < mySharedParticleAttributes.burstCountMin; ++i)
	{
		if (myFreeParticleIndices.empty()) { break; }

		unsigned int freeParticleIndex = myFreeParticleIndices.top();
		myFreeParticleIndices.pop();

		myPerParticleAttributes[freeParticleIndex].lifeTimeTotal = GetRandomFloat(
			mySharedParticleAttributes.lifeTimeMin,
			mySharedParticleAttributes.lifeTimeMax
		);

		myPerParticleAttributes[freeParticleIndex].lifeTimeRemaining = myPerParticleAttributes[freeParticleIndex].lifeTimeTotal;

		myPerParticleAttributes[freeParticleIndex].velocity = {
			GetRandomFloat(mySharedParticleAttributes.velocityMin, mySharedParticleAttributes.velocityMax),
			GetRandomFloat(mySharedParticleAttributes.velocityMin, mySharedParticleAttributes.velocityMax),
			GetRandomFloat(mySharedParticleAttributes.velocityMin, mySharedParticleAttributes.velocityMax)
		};

		myPerParticleAttributes[freeParticleIndex].acceleration = { 0.0f,0.0f,0.0f };

		mySpriteBatch.myInstances[freeParticleIndex].myAttributes.myTransform = aTransform;
		mySpriteBatch.myInstances[freeParticleIndex].myAttributes.myTransform.RotateLocal({
			0.0f,
			GetRandomFloat(mySharedParticleAttributes.angleMin, mySharedParticleAttributes.angleMax) * KE::DegToRadImmediate,
			0.0f
		});

		mySpriteBatch.myInstances[freeParticleIndex].myAttributes.myColor = {
			mySharedParticleAttributes.startColor.x,
			mySharedParticleAttributes.startColor.y,
			mySharedParticleAttributes.startColor.z,
			mySharedParticleAttributes.startColor.w
		};
		//
	}
}

void KE::ParticleEmitter::BurstText(const Transform& aTransform, KE::SpriteFont* aFont, const std::string& aText)
{
	const size_t textLength = aText.length();
	if (myFreeParticleIndices.size() < textLength) { return; }

	KE::Sprite* sprites[16] = { 0 };
	for (int i = 0; i < textLength; ++i)
	{
		sprites[i] = &mySpriteBatch.myInstances[myFreeParticleIndices.top()];
		myPerParticleAttributes[myFreeParticleIndices.top()].lifeTimeRemaining = 2.0f;
		myPerParticleAttributes[myFreeParticleIndices.top()].velocity = { 0.0f, 1.0f, 0.0f };

		myFreeParticleIndices.pop();
	}

	aFont->PrepareSprites(sprites, aText, {}, aTransform);
}

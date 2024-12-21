#pragma once
#include "Engine\Source\Math\Vector.h"
#include "Engine\Source\Math\Transform.h"

namespace KE
{
	class Graphics;

	class DebugRenderer
	{
	public:
		DebugRenderer() = default;
		~DebugRenderer() {};

		void Init(Graphics& aGraphics);

		void RenderLine(const Vector3f& aFrom, const Vector3f& aTo, const Vector4f* aColour = nullptr);
		void RenderLine(const Vector4f& aFrom, const Vector4f& aTo, const Vector4f* aColour = nullptr);
		void RenderLine(const Vector4f& aFrom, const Vector4f& aTo, const Vector4f& aColour);
		void RenderCube(const Vector3f& aPosition, const Vector3f& aSize, const Vector4f& aColour = myDefaultColour);
		void RenderCube(const Vector3f& aPosition, const Vector3i& aSize, const Vector4f& aColour = myDefaultColour);
		void RenderCube(const Transform& aTransform, const Vector3f& aSize, const Vector4f& aColour = myDefaultColour);
		void RenderSphere(const Vector3f& aPosition, const float aRadius, const Vector4f& aColour = myDefaultColour);
		void RenderSphere(const Transform& aTransform, float aRadius, const Vector4f& aColour = myDefaultColour);
		void RenderCone(const Vector3f& aPosition, const Vector3f& aDirection, const float aLength, const float anOuterRadius, const float anInnerRadius, const Vector4f& aColour = myDefaultColour);
		void RenderScreenSpaceLine(const Vector2f& aFrom, const Vector2f& aTo, const Vector4f& aColour = myDefaultColour);
		void RenderCapsule(const Vector3f& aPosition, const float aRadius, const float aLength, const Vector4f& aColour = myDefaultColour);
		void RenderCapsule(const Transform& aTransform, const float aRadius, const float aLength, const Vector4f& aColour = myDefaultColour);
	private:
		Graphics* myGraphics;

		const inline static Vector4f myDefaultColour = { 1.0f, 1.0f, 1.0f, 1.0f };
	};

}
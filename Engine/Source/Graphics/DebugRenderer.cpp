#include "stdafx.h"
#include "DebugRenderer.h"
#include "Engine\Source\Graphics\Graphics.h"



void KE::DebugRenderer::Init(Graphics& aGraphics)
{
	myGraphics = &aGraphics;
	KE_GLOBAL::blackboard.Register<DebugRenderer>("debugRenderer", this);
}

void KE::DebugRenderer::RenderLine(const Vector3f& aFrom, const Vector3f& aTo, const Vector4f* aColour)
{
	assert(myGraphics && "Graphics in DBGRenderer is nullptr");

	LineVertex v1(aFrom);
	LineVertex v2(aTo);
	if (aColour)
	{
		v1.colour = *aColour;
		v2.colour = *aColour;
	}

	myGraphics->myDebugLines.AddLine(v1, v2);
}

void KE::DebugRenderer::RenderLine(const Vector4f& aFrom, const Vector4f& aTo, const Vector4f* aColour)
{
	assert(myGraphics && "Graphics in DBGRenderer is nullptr");

	LineVertex v1(aFrom);
	LineVertex v2(aTo);
	if (aColour)
	{
		v1.colour = *aColour;
		v2.colour = *aColour;
	}

	myGraphics->myDebugLines.AddLine(v1, v2);
}

void KE::DebugRenderer::RenderLine(const Vector4f& aFrom, const Vector4f& aTo, const Vector4f& aColour)
{
	assert(myGraphics && "Graphics in DBGRenderer is nullptr");

	LineVertex v1(aFrom);
	LineVertex v2(aTo);
	v1.colour = aColour;
	v2.colour = aColour;

	myGraphics->myDebugLines.AddLine(v1, v2);
}

void KE::DebugRenderer::RenderCube(const Vector3f& aPosition, const Vector3f& aSize, const Vector4f& aColour)
{
	assert(myGraphics && "Graphics in DBGRenderer is nullptr");

	myGraphics->myDebugLines.DrawCube(aPosition, aSize, aColour);
}

void KE::DebugRenderer::RenderCube(const Vector3f& aPosition, const Vector3i& aSize, const Vector4f& aColour)
{
	assert(myGraphics && "Graphics in DBGRenderer is nullptr");

	myGraphics->myDebugLines.DrawCube(aPosition, { (float)aSize.x, (float)aSize.y, (float)aSize.z }, aColour);
}

void KE::DebugRenderer::RenderCube(const Transform& aTransform, const Vector3f& aSize, const Vector4f& aColour)
{
	assert(myGraphics && "Graphics in DBGRenderer is nullptr");

	myGraphics->myDebugLines.DrawCube(aTransform, aSize, aColour);
}

void KE::DebugRenderer::RenderSphere(const Vector3f& aPosition, const float aRadius, const Vector4f& aColour)
{
	assert(myGraphics && "Graphics in DBGRenderer is nullptr");

	myGraphics->myDebugLines.DrawSphere(aPosition, aRadius, aColour);
}

void KE::DebugRenderer::RenderSphere(const Transform& aTransform, const float aRadius, const Vector4f& aColour)
{
	assert(myGraphics && "Graphics in DBGRenderer is nullptr");

	myGraphics->myDebugLines.DrawSphere(aTransform, aRadius, aColour);
}

void KE::DebugRenderer::RenderCone(const Vector3f& aPosition, const Vector3f& aDirection, const float aLength, const float anOuterRadius, const float anInnerRadius, const Vector4f& aColour)
{
	myGraphics->myDebugLines.DrawCone(aPosition, aDirection, aLength, anOuterRadius, anInnerRadius, aColour);
}

void KE::DebugRenderer::RenderScreenSpaceLine(const Vector2f& aFrom, const Vector2f& aTo, const Vector4f& aColour)
{
	assert(myGraphics && "Graphics in DBGRenderer is nullptr");

	myGraphics->AddScreenSpaceLine(aFrom, aTo, aColour);
}

void KE::DebugRenderer::RenderCapsule(const Vector3f& aPosition, const float aRadius, const float aLength, const Vector4f& aColour)
{
	myGraphics->myDebugLines.DrawCapsule(aPosition, aRadius, aLength, aColour);
}

void KE::DebugRenderer::RenderCapsule(const Transform& aTransform, const float aRadius, const float aLength, const Vector4f& aColour)
{
	myGraphics->myDebugLines.DrawCapsule(aTransform, aRadius, aLength, aColour);
}

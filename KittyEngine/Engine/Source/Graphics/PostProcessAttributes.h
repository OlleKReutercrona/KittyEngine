#pragma once
namespace KE
{
	struct PostProcessAttributes
	{
		// Chromatic Abberation
		Vector2f CARedOffset;
		Vector2f CAGreenOffset;
		Vector2f CABlueOffset;
		float CAMultiplier;

		// Colour Correction
		float saturation;
		Vector3f colourCorrecting;
		float contrast;
		Vector4f tint;
		Vector4f blackPoint;
		float exposure;

		// Bloom
		Vector3f bloomSampleTreshold;
		float bloomSampleStage;
		float bloomTreshold;
		float bloomBlending;

		// Vignette
		float vignetteSize;
		float vignetteFeatherThickness;
		float vignetteIntensity;
		int vignetteShowMask;

		// Guassian Blur
		float gaussianDirection;
		float gaussianQuality;
		float gaussianSize;
		float gaussianTreshold;

		// Tonemap
		float toneMapIntensity;
	};
}

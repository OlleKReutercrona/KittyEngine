#pragma once
#include <cmath>
#include <DirectXMath.h>
#include "Vector.h"

namespace KE
{
	constexpr float PI = 3.14159265f;
	constexpr double PI_D = 3.1415926535897932;

	constexpr float DegToRadImmediate = PI / 180.0f;
	constexpr float RadToDegImmediate = 180.0f / PI;

	template <typename T>
	inline T Clamp(const T& aValue, const T& aMin, const T& aMax)
	{
		if (aValue < aMin)
		{
			return aMin;
		}
		else if (aValue > aMax)
		{
			return aMax;
		}
		return aValue;
	}

	template <typename T>
	constexpr auto Square(const T& aValue) noexcept
	{
		return aValue * aValue;
	}

	template <typename T>
	T WrapAngle(T aTheta) noexcept
	{
		constexpr T twoPi = static_cast<T>(2) * static_cast<T>(PI_D);
		const T mod = static_cast<T>(fmod(aTheta, twoPi));
		if (mod > static_cast<T>(PI_D))
		{
			return mod - twoPi;
		}
		else if (mod < -static_cast<T>(PI_D))
		{
			return mod + twoPi;
		}
		return mod;
	}

	template <typename T>
	constexpr T Interp(const T& aCurrent, const T& aTarget, float aAlpha) noexcept
	{
		return aCurrent + (aTarget - aCurrent) * aAlpha;
	}

	template <typename T>
	constexpr T DegToRad(T aDegrees) noexcept
	{
		return aDegrees * PI / static_cast<T>(180.0);
	}

	template <typename T>
	constexpr T RadToDeg(T aRad) noexcept
	{
		return aRad / PI * static_cast<T>(180.0);
	}

	template <typename T>
	constexpr T Gauss(T aX, T aSigma) noexcept
	{
		const auto ss = Square(aSigma);
		return (static_cast<T>(1.0) / sqrt(static_cast<T>(2.0) * static_cast<T>(PI_D) * ss)) * exp(
			-Square(aX) / (static_cast<T>(2.0) * ss));
	}

	template <typename T>
	constexpr T Wrap(T aValue, T aMin, T aMax) noexcept
	{
		return aValue > aMax ? aValue - aMax : aValue < aMin ? aValue + aMax : aValue;
	}

	constexpr float lerp(float start, float end, float percentage) noexcept
	{
		// This is a clamp
		// it should probably use a function called clamp
		// but kitty clamp is not working and making me angry
		// So here it is again...
		if (percentage < 0.0f)
		{
			percentage = 0.0f;
		}
		else if (percentage > 1.0f)
		{
			percentage = 1.0f;
		}

		return (start * (1.0f - percentage)) + (end * percentage);
	}

	constexpr float Smoothstep(float t) noexcept
	{
		float v1 = t * t;
		float v2 = 1.f - (1.f - t) * (1.f - t);

		return lerp(v1, v2, t);
	}

	constexpr float CubicHermiteStep(float t) noexcept
	{
		return t * t * (3.0f - 2.0f * t);
	}

	constexpr float CubicBezierStep(float t) noexcept
	{
		return 3.0f * t * t * (1.0f - t);
	}

	// Uses SmoothStep() to get an smooth curve in start and in the end.
	inline Vector3f SmoothLerp(Vector3f aStart, Vector3f aEnd, float aPercentage)
	{
		float smoothstep = Smoothstep(aPercentage);

		return Vector3f(
			aStart.x + (aEnd.x - aStart.x) * smoothstep,
			aStart.y + (aEnd.y - aStart.y) * smoothstep,
			aStart.z + (aEnd.z - aStart.z) * smoothstep
		);
	}

	// Returns the optimal rotation from aStart to aTarget.
	inline Vector3f GetNearestRotation(Vector3f aStart, Vector3f aTarget)
	{
		if ((aTarget.x - aStart.x) > PI)
		{
			aTarget.x = aStart.x + (aTarget.x - aStart.x) - (PI * 2.0f);
		}
		else if ((aTarget.x - aStart.x) < -PI)
		{
			aTarget.x = aStart.x + (aTarget.x - aStart.x) + (PI * 2.0f);
		}

		if ((aTarget.y - aStart.y) > PI)
		{
			aTarget.y = aStart.y + (aTarget.y - aStart.y) - (PI * 2.0f);
		}
		else if ((aTarget.y - aStart.y) < -PI) 
		{
			aTarget.y = aStart.y + (aTarget.y - aStart.y) + (PI * 2.0f);
		}

		if ((aTarget.z - aStart.z) > PI)
		{
			aTarget.z = aStart.z + (aTarget.z - aStart.z) - (PI * 2.0f);
		}
		else if ((aTarget.z - aStart.z) < -PI) 
		{
			aTarget.z = aStart.z + (aTarget.z - aStart.z) + (PI * 2.0f);
		}

		return aTarget;
	}

	inline float GetRandomFloat(float aMin, float aMax, float aPrecision = 1000.0f)
	{
		if (aMin - aMax == 0.0f)
		{
			return aMin;
		}

		float random = static_cast<float>(rand() % static_cast<int>((aMax - aMin) * aPrecision));
		return aMin + random / aPrecision;
	}

	inline int GetRandomInt(int aMin, int aMax)
	{
		return rand() % (aMax - aMin) + aMin;
	}

	inline float MapToRange(float aRotation)
	{
		while (aRotation <= PI) {
			aRotation += 2 * PI;
		}

		while (aRotation > PI) {
			aRotation -= 2 * PI;
		}

		return aRotation;
	}

}

#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <string>

#include "Engine/Source/Collision/Shape.h"
#include "imgui/imgui.h"
#include "Engine/Source/Math/Ray.h"
#include "imgui/imgui_internal.h"

namespace KE_EDITOR
{
	inline static const char* FormatString(const char* fmt, ...)
	{
		static char buffer[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf_s(buffer, sizeof(buffer), fmt, args);
		va_end(args);
		return buffer;
	}

	inline static const char* GetFileNameFromPath(const std::string& aPath)
	{
		size_t lastSlash = aPath.find_last_of("/\\");
		if (lastSlash == std::string::npos)
		{
			return aPath.c_str();
		}
		else
		{
			return aPath.c_str() + lastSlash + 1;
		}
	}

	typedef unsigned int KittyFlags;

	enum KittyFlagTypes : KittyFlags
	{
		eNone = 0,
		eBeginWindow = 1 << 0,
		eSeparatorAbove = 1 << 1,
		eSeparatorBelow = 1 << 2,
		eSeparatorButton = 1 << 3,
	};

	inline ImVec2 GetMousePosInWindow(ImVec2 upperLeft)
	{
		POINT mousePos;
		GetCursorPos(&mousePos);

		return ImVec2(mousePos.x - upperLeft.x, mousePos.y - upperLeft.y);
	}
	
	inline bool EditorMouseInside(ImVec2 pos1, ImVec2 pos2, ImVec2 mousePos)
	{
		return mousePos.x > pos1.x && mousePos.x < pos2.x && mousePos.y > pos1.y && mousePos.y < pos2.y;
	}

	inline bool EditorIntersectBox(const KE::Box& aBox, const Rayf& aRay, Vector3f& aOutHitPoint)
	{
		// r.dir is unit direction vector of ray
		Vector3f dirfrac;
		dirfrac.x = 1.0f / aRay.GetDirection().x;
		dirfrac.y = 1.0f / aRay.GetDirection().y;
		dirfrac.z = 1.0f / aRay.GetDirection().z;
		// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
		// r.org is origin of ray
		float t1 = (aBox.myMin.x + aBox.myPosition.x + aBox.myOffset.x - aRay.GetOrigin().x) * dirfrac.x;
		float t2 = (aBox.myMax.x + aBox.myPosition.x + aBox.myOffset.x - aRay.GetOrigin().x) * dirfrac.x;

		float t3 = (aBox.myMin.y + aBox.myPosition.y + aBox.myOffset.y - aRay.GetOrigin().y) * dirfrac.y;
		float t4 = (aBox.myMax.y + aBox.myPosition.y + aBox.myOffset.y - aRay.GetOrigin().y) * dirfrac.y;

		float t5 = (aBox.myMin.z + aBox.myPosition.z + aBox.myOffset.z - aRay.GetOrigin().z) * dirfrac.z;
		float t6 = (aBox.myMax.z + aBox.myPosition.z + aBox.myOffset.z - aRay.GetOrigin().z) * dirfrac.z;

		float tmin = fmax(fmax(fmin(t1, t2), fmin(t3, t4)), fmin(t5, t6));
		float tmax = fmin(fmin(fmax(t1, t2), fmax(t3, t4)), fmax(t5, t6));

		float t;
		if (tmax < 0)
		{
			t = tmax;
			return false;
		}
		if (tmin > tmax)
		{
			t = tmax;
			return false;
		}

		t = tmin;

		aOutHitPoint = aRay.GetOrigin() + aRay.GetDirection() * t;
		return true;
	}

	inline bool MatchesFilter(const char* filter, const char* name)
	{
		ImGuiTextFilter textFilter(filter);
		return textFilter.PassFilter(name);
	}

	inline void ClearString(const char* string, int size)
	{
		memset((void*)string, 0, size);
	}


	inline ImVec2 GetAspectRegion(ImVec2 aRegion, float aRatio)
	{
		return aRegion.x / aRegion.y > aRatio		   ? 
			   ImVec2( aRegion.y * aRatio, aRegion.y ) :
			   ImVec2( aRegion.x, aRegion.x / aRatio ) ;
	}
}


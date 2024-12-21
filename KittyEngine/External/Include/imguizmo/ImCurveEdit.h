// https://github.com/CedricGuillemet/ImGuizmo
// v 1.83
//
// The MIT License(MIT)
//
// Copyright(c) 2021 Cedric Guillemet
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#pragma once
#include <stdint.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include <set>
#include <vector>

namespace ImCurveEdit
{
   enum CurveType
   {
      CurveNone,
      CurveDiscrete,
      CurveLinear,
      CurveSmooth,
      CurveBezier,
   };

   struct EditPoint
   {
      int curveIndex;
      int pointIndex;
      bool operator <(const EditPoint& other) const
      {
         if (curveIndex < other.curveIndex)
            return true;
         if (curveIndex > other.curveIndex)
            return false;

         if (pointIndex < other.pointIndex)
            return true;
         return false;
      }
   };

   struct EditData
   {
	   bool selectingQuad = false;
	   ImVec2 quadSelection;
	   int overCurve = -1;
	   float overCurveT = 0.f;

	   int movingCurve = -1;
	   bool scrollingV = false;
	   std::set<EditPoint> selection;
	   bool overSelectedPoint = false;

       int selectedPointCurve = -1;
	   int selectedPoint = -1;

	   bool pointsMoved = false;
	   ImVec2 mousePosOrigin;
	   std::vector<ImVec2> originalPoints;

	   ImRect sequenceRect;
   };

   struct Delegate
   {
	   virtual ~Delegate() = default;
	   bool focused = false;
      virtual size_t GetCurveCount() = 0;
      virtual bool IsVisible(size_t /*curveIndex*/) { return true; }
      virtual CurveType GetCurveType(size_t /*curveIndex*/) const { return CurveSmooth; }
      virtual ImVec2& GetMin() = 0;
      virtual ImVec2& GetMax() = 0;
      virtual size_t GetPointCount(size_t curveIndex) = 0;
      virtual uint32_t GetCurveColor(size_t curveIndex) = 0;
      virtual ImVec2* GetPoints(size_t curveIndex) = 0;
      virtual int EditPoint(size_t curveIndex, int pointIndex, ImVec2 value) = 0;
      virtual void AddPoint(size_t curveIndex, ImVec2 value) = 0;
      virtual unsigned int GetBackgroundColor() { return 0xFF202020; }
      virtual bool IsPointVisible(size_t aCurveIndex, size_t aPointIndex) { return true; }
      // handle undo/redo thru this functions
      virtual void BeginEdit(int /*index*/) {}
      virtual void EndEdit() {}
      virtual const char* GetCurveName(size_t curveIndex) { return "unnamed curve"; }
   };

   int Edit(Delegate& delegate, const ImVec2& size, unsigned int id, EditData* editData, const ImRect* clippingRect = NULL, ImVector<EditPoint>* selectedPoints = NULL);
}

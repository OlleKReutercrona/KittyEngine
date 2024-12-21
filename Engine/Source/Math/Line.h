#pragma once
#include "Vector2.h"
#include "Vector3.h"

namespace CommonUtilities
{
	
	template <class T>
	class Line
	{
	public:

		// Default constructor: there is no line, the normal is the zero vector.
		Line<T>();
		// Copy constructor.
		Line<T>(const Line <T>& aLine);
		// Constructor that takes two points that define the line, the direction is aPoint1 - aPoint0.
		Line<T>(const Vector2<T>& aPoint0, const Vector2<T>& aPoint1);
		// Init the line with two points, the same as the constructor above.
		void InitWith2Points(const Vector2<T>& aPoint0, const Vector2<T>& aPoint1);
		// Init the line with a point and a direction.
		void InitWithPointAndDirection(const Vector2<T>& aPoint, const Vector2<T>& ´aDirection);
		// Returns whether a point is inside the line: it is inside when the point is on the line or on the side the normal is pointing away from.
		bool IsInside(const Vector2<T>& aPosition) const;
		bool IsInside(const Vector3<T>& aPosition) const;
		// Flips the normal
		void FlipNormal();
		// Returns the direction of the line.
		const Vector2<T>& GetDirection() const;
		// Returns the normal of the line, which is (-direction.y, direction.x).
		const Vector2<T> GetNormal() const;

		const Vector2<T> GetOrigin() const;

		Vector2<T> myDirection;
		Vector2<T> myPoint;
		Vector2<T> myNormal;
		Vector2<T> myDebugPoint;
		float debugLength = 0;
	private:

	};

	// Default constructor: there is no line, the normal is the zero vector.
	template <class T> inline Line<T>::Line()
	{
		myDirection = {};
		myPoint = {};
	}
	// Copy constructor.
	template <class T> inline Line<T>::Line(const Line <T>& aLine)
	{
		myPoint = aLine.myPoint;
		myDirection = aLine.myDirection;
		myNormal = aLine.myNormal;
		myDebugPoint = aLine.myDebugPoint;
		debugLength = aLine.debugLength;

	}
	// Constructor that takes two points that define the line, the direction is aPoint1 - aPoint0.
	template <class T> inline Line<T>::Line(const Vector2<T>& aPoint0, const Vector2<T>& aPoint1)
	{
		myPoint = aPoint0;
		myDebugPoint = aPoint1;
		myDirection = aPoint1 - aPoint0;
		myDirection.Normalize();
		myNormal = { -myDirection.y, myDirection.x };
		debugLength = (aPoint0 - aPoint1).Length();
	}

	// Init the line with two points, the same as the constructor above.
	template <class T> inline void Line<T>::InitWith2Points(const Vector2<T>& aPoint0, const Vector2<T>& aPoint1)
	{
		myPoint = aPoint0;
		myDirection = aPoint1 - aPoint0;
		myDirection.Normalize();
		myNormal = { -myDirection.y, myDirection.x };
	}

	// Init the line with a point and a direction.
	template <class T> inline void Line<T>::InitWithPointAndDirection(const Vector2<T>& aPoint, const Vector2<T>& aDirection)
	{
		myPoint = aPoint;
		myDirection = aDirection;
		myDirection.Normalize();
		myNormal = { -myDirection.y, myDirection.x };
	}

	// Returns whether a point is inside the line: it is inside when the point is on the line or on the side the normal is pointing away from.
	template <class T> inline bool Line<T>::IsInside(const Vector2<T>& aPosition) const
	{
		if (myNormal.Dot(aPosition - myPoint) < 0.000001)
		{
			return true;
		}
		return false;
	}
	template <class T> inline bool Line<T>::IsInside(const Vector3<T>& aPosition) const
	{
		Vector2f pos_2D = { aPosition.x, aPosition.z };

		if (myNormal.Dot(pos_2D - myPoint) < 0.000001)
		{
			return true;
		}
		return false;
	}

	template <class T> inline void Line<T>::FlipNormal()
	{
		myNormal *= -1.0f;
	}

	// Returns the direction of the line.
	template <class T> inline const Vector2<T>& Line<T>::GetDirection() const
	{
		return myDirection;
	}

	// Returns the normal of the line, which is (-direction.y, direction.x).
	template <class T> inline const Vector2<T> Line<T>::GetNormal() const
	{
		return myNormal;
	}

	template <class T> inline const Vector2<T> Line<T>::GetOrigin() const
	{
		return myPoint;
	}

}

typedef CommonUtilities::Line<float> Linef;

typedef CommonUtilities::Line<int> Linei;
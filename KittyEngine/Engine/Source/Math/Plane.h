#pragma once
#include "Vector3.h"


namespace CommonUtilities
{
	template <class T>
	class Plane
	{
	public:
		// Default constructor.
		Plane<T>();
		// Constructor taking three points where the normal is (aPoint1 - aPoint0) x (aPoint2 -aPoint0).
		Plane<T>(const Vector3<T>& aPoint0, const Vector3<T>& aPoint1, const Vector3<T>& aPoint2);
		// Constructor taking a point and a normal.
		Plane<T>(const Vector3<T>& aPoint0, const Vector3<T>& aNormal);
		// Init the plane with three points, the same as the constructor above.
		void InitWith3Points(const Vector3<T>& aPoint0, const Vector3<T>& aPoint1, const Vector3<T>& aPoint2);
		// Init the plane with a point and a normal, the same as the constructor above.
		void InitWithPointAndNormal(const Vector3<T>& aPoint, const Vector3<T>& aNormal);
		// Returns whether a point is inside the plane: it is inside when the point is on the plane or on the side the normal is pointing away from.
		bool IsInside(const Vector3<T>&aPosition) const;

		bool IsOnPlane(const Vector3<T>& aPosition) const;

		// Returns the normal of the plane.
		const Vector3<T>& GetNormal() const;
		const Vector3<T>& GetPoint() const;

	private:
		Vector3<T> myPoint;
		Vector3<T> myNormal;

	};

	// Default constructor.
	template <class T> inline Plane<T>::Plane()
	{
		myPoint = {};
		myNormal = {};
	}

	// Constructor taking three points where the normal is (aPoint1 - aPoint0) cross-product (aPoint2 -aPoint0).
	template <class T> inline Plane<T>::Plane(const Vector3<T>& aPoint0, const Vector3<T>& aPoint1, const Vector3<T>& aPoint2)
	{
		myNormal = Vector3<T>(aPoint1 - aPoint0).Cross(Vector3<T>(aPoint2 - aPoint0));
		myNormal.Normalize();
		myPoint = aPoint0;
	}


	// Constructor taking a point and a normal.
	template <class T> inline Plane<T>::Plane(const Vector3<T>& aPoint0, const Vector3<T>& aNormal)
	{
		myNormal = aNormal;
		myPoint = aPoint0;
	}

	// Init the plane with three points, the same as the constructor above.
	template <class T> inline void Plane<T>::InitWith3Points(const Vector3<T>& aPoint0, const Vector3<T>& aPoint1, const Vector3<T>& aPoint2)
	{
		myNormal = Vector3<T>(aPoint1 - aPoint0).Cross(Vector3<T>(aPoint2 - aPoint0));
		myNormal.Normalize();
		myPoint = aPoint0;
	}

	// Init the plane with a point and a normal, the same as the constructor above.
	template <class T> inline void Plane<T>::InitWithPointAndNormal(const Vector3<T>& aPoint, const Vector3<T>& aNormal)
	{
		myNormal = aNormal;
		myPoint = aPoint;
	}


	// Returns whether a point is inside the plane: it is inside when the point is on the plane or on the side the normal is pointing away from.
	template <class T> inline bool Plane<T>::IsInside(const Vector3<T>& aPosition) const
	{
		if (GetNormal().Dot(aPosition - myPoint) < 0.000001)
		{
			return true;
		}

		return false;
	}

	// Returns whether a point is inside the plane: it is inside when the point is on the plane or on the side the normal is pointing away from.
	template <class T> inline bool Plane<T>::IsOnPlane(const Vector3<T>& aPosition) const
	{
		if (GetNormal().Dot(aPosition - myPoint) < 0.000001 && GetNormal().Dot(aPosition - myPoint) > -0.000001)
		{
			return true;
		}

		return false;
	}

	// Returns the normal of the plane.
	template <class T> inline const Vector3<T>& Plane<T>::GetNormal() const
	{
		return myNormal;
	}

	template <class T> inline const Vector3<T>& Plane<T>::GetPoint() const
	{
		return myPoint;
	}
}

typedef CommonUtilities::Plane<float> Plane;
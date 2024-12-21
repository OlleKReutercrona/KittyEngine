#pragma once
#include "Vector3.h"

namespace CommonUtilities
{
	template <class T>
	class Sphere
	{
	public:

		// Default constructor: there is no sphere, the radius is zero and the position is the zero vector.
		Sphere();

		// Copy constructor.
		Sphere(const Sphere<T>& aSphere);

		// Constructor that takes the center position and radius of the sphere.
		Sphere(const Vector3<T>& aCenter, T aRadius);

		// Init the sphere with a center and a radius, the same as the constructor above.
		void InitWithCenterAndRadius(const Vector3<T>& aCenter, T aRadius);

		// Returns whether a point is inside the sphere: it is inside when the point is on the sphere surface or inside of the sphere.
		bool IsInside(const Vector3<T>& aPosition) const;

		Vector3<T> GetPosition( ) const;
		T GetRadius() const;
		Vector3<T> GetNormal(Vector3<T> aPosition) const;

	private:
		Vector3<T> myPosition;
		T myRadius;
	};



	template <class T> inline Sphere<T>::Sphere()
	{
		// Default constructor: there is no sphere, the radius is zero and the position is the zero vector.

		myPosition = {};
		myRadius = 0;
	}

	template <class T> inline Sphere<T>::Sphere(const Sphere<T>& aSphere)
	{
		// Copy constructor.

		myPosition = aSphere.myPosition;
		myRadius = aSphere.myRadius;

	}

	template <class T> inline Sphere<T>::Sphere(const Vector3<T>& aCenter, T aRadius)
	{
		// Constructor that takes the center position and radius of the sphere.

		myPosition = aCenter;
		myRadius = aRadius;

	}

	template <class T> inline void Sphere<T>::InitWithCenterAndRadius(const Vector3<T>& aCenter, T aRadius)
	{
		// Init the sphere with a center and a radius, the same as the constructor above.

		myPosition = aCenter;
		myRadius = aRadius;

	}

	template <class T> inline bool Sphere<T>::IsInside(const Vector3<T>& aPosition) const
	{
		// Returns whether a point is inside the sphere:
		// - it is inside when the point is on the sphere surface or inside of the sphere.

		Vector3<T> aVector =  aPosition - myPosition;

		return aVector.LengthSqr() <= myRadius * myRadius;
	
	}

	template <class T> inline Vector3<T> Sphere<T>::GetPosition() const
	{
		return myPosition;
	}

	template <class T> inline T Sphere<T>::GetRadius() const
	{
		return myRadius;
	}

	template <class T> inline Vector3<T> Sphere<T>::GetNormal(Vector3<T> aPosition) const
	{
		Vector3<T> aVector = aPosition - myPosition;
		aVector.Normalize();
		return aVector;
	}

}
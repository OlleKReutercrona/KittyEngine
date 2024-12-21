#pragma once
#include "Vector2.h"

namespace CommonUtilities
{
	template <class T>
	class Circle
	{
	public:

		// Default constructor: there is no Circle, the radius is zero and the position is the zero vector.
		Circle();

		// Copy constructor.
		Circle(const Circle<T>& aCircle);

		// Constructor that takes the center position and radius of the Circle.
		Circle(const Vector2<T>& aCenter, T aRadius);

		// Init the Circle with a center and a radius, the same as the constructor above.
		void InitWithCenterAndRadius(const Vector2<T>& aCenter, T aRadius);

		// Returns whether a point is inside the Circle: it is inside when the point is on the Circle surface or inside of the Circle.
		bool IsInside(const Vector2<T>& aPosition) const;

	private:
		Vector<T> myPosition;
		T myRadius;
	};



	template <class T> inline Circle<T>::Circle()
	{
		// Default constructor: there is no Circle, the radius is zero and the position is the zero vector.

		myPosition = {};
		myRadius = 0;
	}

	template <class T> inline Circle<T>::Circle(const Circle<T>& aCircle)
	{
		// Copy constructor.

		myPosition = aCircle.myPosition;
		myRadius = aCircle.myRadius;

	}

	template <class T> inline Circle<T>::Circle(const Vector2<T>& aCenter, T aRadius)
	{
		// Constructor that takes the center position and radius of the Circle.

		myPosition = aCenter;
		myRadius = aRadius;

	}

	template <class T> inline void Circle<T>::InitWithCenterAndRadius(const Vector2<T>& aCenter, T aRadius)
	{
		// Init the Circle with a center and a radius, the same as the constructor above.

		myPosition = aCenter;
		myRadius = aRadius;

	}

	template <class T> inline bool Circle<T>::IsInside(const Vector2<T>& aPosition) const
	{
		// Returns whether a point is inside the Circle:
		// - it is inside when the point is on the Circle surface or inside of the Circle.

		Vector2<T> aVector = aPosition - myPosition;

		return aVector.LengthSqr() <= myRadius * myRadius;

	}

}
#pragma once
#include "Vector2.h"

namespace CommonUtilities
{
	template <class T>
	class AABB2D
	{
	public:
		// Default constructor: there is no AABB, both min and max points are the zero vector.
		AABB2D();

		// Copy constructor.
		AABB2D(const AABB2D<T>& aAABB2D);

		// Constructor taking the positions of the minimum and maximum corners.
		AABB2D(const Vector2<T>& aMin, const Vector2<T>& aMax);

		// Init the AABB with the positions of the minimum and maximum corners, same as the constructor above.
		void InitWithMinAndMax(const Vector2<T>& aMin, const Vector2<T>& aMax);

		// Returns whether a point is inside the AABB: it is inside when the point is on any of the AABB's sides or inside of the AABB.
		bool IsInside(const Vector2<T>& aPosition) const;


	private:
		Vector2<T> myMin;
		Vector2<T> myMax;
	};


	template<class T> inline AABB2D<T>::AABB2D()
	{
		// Default constructor: there is no AABB, both min and max points are the zero vector.

		myMin = {};
		myMax = {};

	}

	template<class T> inline AABB2D<T>::AABB2D(const AABB2D<T>& aAABB2D)
	{
		// Copy constructor.
		myMin = aAABB2D.myMin;
		myMax = aAABB2D.myMax;
	}

	template<class T> inline AABB2D<T>::AABB2D(const Vector2<T>& aMin, const Vector2<T>& aMax)
	{
		// Constructor taking the positions of the minimum and maximum corners.
		myMin = aMin;
		myMax = aMax;

	}

	template<class T> inline void AABB2D<T>::InitWithMinAndMax(const Vector2<T>& aMin, const Vector2<T>& aMax)
	{
		// Init the AABB with the positions of the minimum and maximum corners, same as the constructor above.
		myMin = aMin;
		myMax = aMax;
	}

	template<class T> inline bool AABB2D<T>::IsInside(const Vector2<T>& aPosition) const
	{
		// Returns whether a point is inside the AABB: it is inside when the point is on any of the AABB's sides or inside of the AABB.

		if (!(aPosition.x < myMin.x) != !(aPosition.x > myMax.x)) return false;

		if (!(aPosition.y < myMin.y) != !(aPosition.y > myMax.y)) return false;

		return true;

	}

}
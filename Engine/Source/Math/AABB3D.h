#pragma once
#include "Vector3.h"

namespace CommonUtilities
{
	template <class T>
	class AABB3D
	{
	public:
		// Default constructor: there is no AABB, both min and max points are the zero vector.
		AABB3D();

		// Copy constructor.
		AABB3D(const AABB3D<T>& aAABB3D);

		// Constructor taking the positions of the minimum and maximum corners.
		AABB3D(const Vector3<T>& aMin, const Vector3<T>& aMax);

		// Init the AABB with the positions of the minimum and maximum corners, same as the constructor above.
		void InitWithMinAndMax(const Vector3<T>& aMin, const Vector3<T>& aMax);

		// Returns whether a point is inside the AABB: it is inside when the point is on any of the AABB's sides or inside of the AABB.
		bool IsInside(const Vector3<T>& aPosition) const;
		bool IsInside(const Vector3<T>& aPosition, const Vector3<T>& aNormal) const;

		const Vector3<T> GetMin() const;
		const Vector3<T> GetMax() const;

	private:
		Vector3<T> myMin;
		Vector3<T> myMax;
	};

	
	template<class T> inline AABB3D<T>::AABB3D()
	{
		// Default constructor: there is no AABB, both min and max points are the zero vector.

		myMin = {};
		myMax = {};

	}

	template<class T> inline AABB3D<T>::AABB3D(const AABB3D<T>& aAABB3D)
	{
		// Copy constructor.
		myMin = aAABB3D.myMin;
		myMax = aAABB3D.myMax;
	}

	template<class T> inline AABB3D<T>::AABB3D(const Vector3<T>& aMin, const Vector3<T>& aMax)
	{
		// Constructor taking the positions of the minimum and maximum corners.
		myMin = aMin;
		myMax = aMax;

	}

	template<class T> inline void AABB3D<T>::InitWithMinAndMax(const Vector3<T>& aMin, const Vector3<T>& aMax)
	{
		// Init the AABB with the positions of the minimum and maximum corners, same as the constructor above.
		myMin = aMin;
		myMax = aMax;
	}

	template<class T>  inline const Vector3<T> AABB3D<T>::GetMin() const
	{
		return myMin;
	}
	template<class T>  inline const Vector3<T> AABB3D<T>::GetMax() const
	{
		return myMax;
	}
	template<class T> inline bool AABB3D<T>::IsInside(const Vector3<T>& aPosition) const
	{
		// Returns whether a point is inside the AABB: it is inside when the point is on any of the AABB's sides or inside of the AABB.

#pragma region old_solution
		//if (!(aPosition.x < myMin.x) != !(aPosition.x > myMax.x)) return false;
		//	
		//if (!(aPosition.y < myMin.y) != !(aPosition.y > myMax.y)) return false;
		//
		//if (!(aPosition.z < myMin.z) != !(aPosition.z > myMax.z)) return false;
		//
		//return true;
#pragma endregion

		return (
			((aPosition.x >= myMin.x) && (aPosition.x <= myMax.x)) &&
			((aPosition.y >= myMin.y) && (aPosition.y <= myMax.y)) &&
			((aPosition.z >= myMin.z) && (aPosition.z <= myMax.z))
			);

	}

	template<class T> bool AABB3D<T>::IsInside(const Vector3<T>& aPosition, const Vector3<T>& aNormal) const
	{
		return (
			((aPosition.x >= myMin.x) && (aPosition.x <= myMax.x) || aNormal.x != 0) &&
			((aPosition.y >= myMin.y) && (aPosition.y <= myMax.y) || aNormal.y != 0) &&
			((aPosition.z >= myMin.z) && (aPosition.z <= myMax.z) || aNormal.z != 0)
			);
	}

}

typedef CommonUtilities::AABB3D<float> AABB3Df;
typedef CommonUtilities::AABB3D<int> AABB3Di;
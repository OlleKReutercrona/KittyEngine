#pragma once
#include <cmath>
#include <iostream>

namespace CommonUtilities
{

	template <class T>
	class Vector2
	{
	public:
		T x;
		T y;

		//Creates a null-vector
		Vector2<T>();

		//Creates a vector (X, Y)
		Vector2<T>(const T& aX, const T& aY);

		template <class Y>
		Vector2<T>(const Vector2<Y>& aVector);

		//Copy constructor (compiler generated)
		Vector2<T>(const Vector2<T>& aVector) = default;

		//Assignment operator (compiler generated)
		Vector2<T>& operator=(const Vector2<T>& aVector2) = default;

		//Destructor (compiler generated)
		~Vector2<T>() = default;

		//Returns the squared length of the vector
		T LengthSqr() const;

		//Returns the length of the vector
		T Length() const;

		//Returns a normalized copy of this
		Vector2<T> GetNormalized() const;

		//Normalizes the vector
		void Normalize();

		//Returns the dot product of this and aVector
		T Dot(const Vector2<T>& aVector) const;

		//Prints out the X and Y value of an vector.
		template <class U> friend std::ostream& operator<<(std::ostream& os, const Vector2<U>& aVector);
	};

	template<class T> inline Vector2<T>::Vector2()
	{
		//Creates a null-vector
		x = 0;
		y = 0;
	}
	template<class T> inline Vector2<T>::Vector2(const T& aX, const T& aY)
	{
		//Creates a vector (X, Y)
		x = aX;
		y = aY;
	}

	template<class T> template<class Y> inline Vector2<T>::Vector2(const Vector2<Y>& aVector)
	{
		x = static_cast<T>(aVector.x);
		y = static_cast<T>(aVector.y);
	}

	template <class T> Vector2<T> operator+(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		//Returns the vector sum of aVector0 and aVector1

		Vector2<T> Temp;
		Temp.x = aVector0.x + aVector1.x;
		Temp.y = aVector0.y + aVector1.y;

		return Temp;
	}
	template <class T> Vector2<T> operator-(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		//Returns the vector difference of aVector0 and aVector1

		Vector2<T> Temp;

		Temp.x = aVector0.x - aVector1.x;
		Temp.y = aVector0.y - aVector1.y;

		return Temp;
	}

	template <class T> Vector2<T> operator-(const Vector2<T>& aVector, const float aScalar)
	{
		//Returns the vector difference of aVector0 and aVector1

		Vector2<T> Temp;

		Temp.x = aVector.x -aScalar;
		Temp.y = aVector.y -aScalar;

		return Temp;
	}

	template <class T> Vector2<T> operator*(const Vector2<T>& aVector, const T& aScalar)
	{
		//Returns the vector aVector multiplied by the scalar aScalar

		Vector2<T> Temp;
		Temp.x = aVector.x * aScalar;
		Temp.y = aVector.y * aScalar;

		return Temp;
	}
	template <class T> Vector2<T> operator*(const Vector2<T>& aVector, const Vector2<T>& aVector2)
	{
		//Returns the component-wise sum of aVector and aVector2

		Vector2<T> Temp;
		Temp.x = aVector.x * aVector2.x;
		Temp.y = aVector.y * aVector2.y;

		return Temp;
	}
	template <class T> Vector2<T> operator*(const T& aScalar, const Vector2<T>& aVector)
	{
		//Returns the vector aVector multiplied by the scalar aScalar

		Vector2<T> Temp;
		Temp.x = aScalar * aVector.x;
		Temp.y = aScalar * aVector.y;

		return Temp;
	}
	template <class T> Vector2<T> operator/(const Vector2<T>& aVector, const T& aScalar)
	{
		//Returns the vector aVector divided by the scalar aScalar (equivalent to aVector multiplied by 1 / aScalar)

		//Vector2<T> Temp;
		//Temp.x = aVector.x / aScalar;
		//Temp.y = aVector.y / aScalar;
		//
		//return Temp;
		return { aVector.x / aScalar, aVector.y / aScalar };
	}
	template <class T> Vector2<T> operator/(const Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		return Vector2<T>{ aVector0.x / aVector1.x, aVector0.y / aVector1.y };
	}
	template <class T> std::ostream& operator<<(std::ostream& os, const Vector2<T>& aVector)
	{
		return os << "{ X: " << aVector.x << " Y: " << aVector.y << " }";
	}
	template <class T> void operator+=(Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		//Equivalent to setting aVector0 to (aVector0 + aVector1)

		aVector0.x += aVector1.x;
		aVector0.y += aVector1.y;
	}
	template <class T> void operator-=(Vector2<T>& aVector0, const Vector2<T>& aVector1)
	{
		//Equivalent to setting aVector0 to (aVector0 - aVector1)

		aVector0.x -= aVector1.x;
		aVector0.y -= aVector1.y;
	}
	template <class T> void operator*=(Vector2<T>& aVector, const T& aScalar)
	{
		//Equivalent to setting aVector to (aVector * aScalar)

		aVector.x *= aScalar;
		aVector.y *= aScalar;
	}
	template <class T> void operator/=(Vector2<T>& aVector, const T& aScalar)
	{
		//Equivalent to setting aVector to (aVector / aScalar)

		aVector.x /= aScalar;
		aVector.y /= aScalar;
	}

	template<class T> inline T Vector2<T>::LengthSqr() const
	{
		//Returns the squared length of the vector

		return (x * x) + (y * y);
	}
	template<class T> inline T Vector2<T>::Length() const
	{
		//Returns the length of the vector

		return sqrt(LengthSqr());
	}
	template<class T> inline Vector2<T> Vector2<T>::GetNormalized() const
	{
		//Returns a normalized copy of this
		T length = Length();
		Vector2<T> Temp;

		if (x == 0 && y == 0)
		{
			Temp.x = 0;
			Temp.y = 0;
			return Temp;
		}
		else
		{
			Temp.x = x;
			Temp.y = y;
			Temp.x *= (1 / length);
			Temp.y *= (1 / length);

			return Temp;
		}
	}
	template<class T> inline void Vector2<T>::Normalize()
	{

		if (x == 0 && y == 0)
			return;

		T length = Length();

		x *= (1 / length);
		y *= (1 / length);
	}
	template<class T> inline T Vector2<T>::Dot(const Vector2<T>& aVector) const
	{
		//Returns the dot product of this and aVector

		T result = (x * aVector.x) + (y * aVector.y);

		return result;
	}

}

typedef CommonUtilities::Vector2<float> Vector2f;
typedef CommonUtilities::Vector2<int> Vector2i;

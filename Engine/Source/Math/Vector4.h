#pragma once
#include <DirectXMath.h>

namespace CommonUtilities
{
	template <typename T>
	class Vector3;

	template <class T>
	class Vector4
	{
	public:
		T x;
		T y;
		T z;
		T w;
		//Creates a null-vector
		Vector4<T>();

		Vector4<T>(Vector3<T>, T aW = 1);

		//Creates a vector (aX, aY, aZ)
		Vector4<T>(const T& aX, const T& aY, const T& aZ, const T& aW);

		//Copy constructor (compiler generated)
		Vector4<T>(const Vector4<T>& aVector) = default;

		//Assignment operator (compiler generated)
		Vector4<T>& operator=(const Vector4<T>& aVector4) = default;

		const T& operator[](const unsigned int aIndex) const
		{
			assert(aIndex < 4 && "Trying to access class member out of bounds in Vector3 ");

			return (&x)[aIndex];
		}

		T& operator[](const unsigned int aIndex)
		{
			assert(aIndex < 4 && "Trying to access class member out of bounds in Vector3 ");

			return (&x)[aIndex];
		}

		operator DirectX::XMVECTOR() const
		{
			return DirectX::XMVectorSet(x, y, z, w);
		}

		Vector4<T>(const DirectX::XMVECTOR& aVector)
		{
			x = aVector.m128_f32[0];
			y = aVector.m128_f32[1];
			z = aVector.m128_f32[2];
			w = aVector.m128_f32[3];
		}

		operator DirectX::XMFLOAT4() const
		{
			return DirectX::XMFLOAT4(x, y, z, w);
		}

		Vector4<T>(const DirectX::XMFLOAT4& aVector)
		{
			x = aVector[0];
			y = aVector[1];
			z = aVector[2];
			w = aVector[3];
		}

		Vector3<T> xyz();

		//Destructor (compiler generated)
		~Vector4<T>() = default;

		//Returns the squared length of the vector
		T LengthSqr() const;

		//Returns the length of the vector
		T Length() const;

		//Returns a normalized copy of this
		Vector4<T> GetNormalized() const;

		//Normalizes the vector
		void Normalize();

		//Returns the dot product of this and aVector
		T Dot(const Vector4<T>& aVector) const;
	};

	template<class T> inline Vector4<T>::Vector4()
	{
		x = 0;
		y = 0;
		z = 0;
		w = 0;
	}

	template<class T> inline Vector4<T>::Vector4(const T& aX, const T& aY, const T& aZ, const T& aW)
	{
		x = aX;
		y = aY;
		z = aZ;
		w = aW;
	}

	template <class T> Vector4<T> operator+(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		//Returns the vector sum of aVector0 and aVector1
		Vector4<T> Temp;
		Temp.x = aVector0.x + aVector1.x;
		Temp.y = aVector0.y + aVector1.y;
		Temp.z = aVector0.z + aVector1.z;
		Temp.w = aVector0.w + aVector1.w;

		return Temp;
	}

	template <class T> Vector4<T> operator-(const Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		//Returns the vector difference of aVector0 and aVector1

		Vector4<T> Temp;

		Temp.x = aVector0.x - aVector1.x;
		Temp.y = aVector0.y - aVector1.y;
		Temp.z = aVector0.z - aVector1.z;
		Temp.w = aVector0.w - aVector1.w;

		return Temp;
	}

	template <class T> Vector4<T> operator*(const Vector4<T>& aVector, const T& aScalar)
	{
		//Returns the vector aVector multiplied by the scalar aScalar

		Vector4<T> Temp;
		Temp.x = aVector.x * aScalar;
		Temp.y = aVector.y * aScalar;
		Temp.z = aVector.z * aScalar;
		Temp.w = aVector.w * aScalar;

		return Temp;
	}

	template <class T> Vector4<T> operator*(const T& aScalar, const Vector4<T>& aVector)
	{
		//Returns the vector aVector multiplied by the scalar aScalar

		Vector4<T> Temp;
		Temp.x = aScalar * aVector.x;
		Temp.y = aScalar * aVector.y;
		Temp.z = aScalar * aVector.z;
		Temp.w = aScalar * aVector.w;

		return Temp;
	}

	template <class T> Vector4<T> operator/(const Vector4<T>& aVector, const T& aScalar)
	{
		//Returns the vector aVector divided by the scalar aScalar (equivalent to aVector multiplied by 1 / aScalar)

		Vector4<T> Temp;
		Temp.x = aVector.x / aScalar;
		Temp.y = aVector.y / aScalar;
		Temp.z = aVector.z / aScalar;
		Temp.w = aVector.w / aScalar;

		return Temp;
	}

	template <class T> void operator+=(Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		//Equivalent to setting aVector0 to (aVector0 + aVector1)

		aVector0.x += aVector1.x;
		aVector0.y += aVector1.y;
		aVector0.z += aVector1.z;
		aVector0.w += aVector1.w;
	}

	template <class T> void operator-=(Vector4<T>& aVector0, const Vector4<T>& aVector1)
	{
		//Equivalent to setting aVector0 to (aVector0 - aVector1)
		aVector0.x -= aVector1.x;
		aVector0.y -= aVector1.y;
		aVector0.z -= aVector1.z;
		aVector0.w -= aVector1.w;
	}

	template <class T> void operator*=(Vector4<T>& aVector, const T& aScalar)
	{
		//Equivalent to setting aVector to (aVector * aScalar)
		aVector.x *= aScalar;
		aVector.y *= aScalar;
		aVector.z *= aScalar;
		aVector.w *= aScalar;
	}

	template <class T> void operator/=(Vector4<T>& aVector, const T& aScalar)
	{
		//Equivalent to setting aVector to (aVector / aScalar)
		aVector.x /= aScalar;
		aVector.y /= aScalar;
		aVector.z /= aScalar;
		aVector.w /= aScalar;
	}

	template <class T> void operator+=(Vector4<T>& aVector0, const Vector3<T>& aVector1)
	{
		aVector0.x += aVector1.x;
		aVector0.y += aVector1.y;
		aVector0.z += aVector1.z;
	}

	template<class T> inline T Vector4<T>::LengthSqr() const
	{
		//Returns the squared length of the vector

		return (x * x) + (y * y) + (z * z) + (w * w);
	}

	template<class T> inline T Vector4<T>::Length() const
	{
		//Returns the length of the vector

		return static_cast<T>(sqrt(LengthSqr()));
	}

	template<class T> inline Vector4<T> Vector4<T>::GetNormalized() const
	{
		//Returns a normalized copy of this
		T length = Length();
		Vector4<T> Temp;

		if (x == 0 && y == 0 && z == 0 && w == 0)
		{
			Temp.x = 0;
			Temp.y = 0;
			Temp.z = 0;
			Temp.w = 0;
			return Temp;
		}

		Temp.x = x;
		Temp.y = y;
		Temp.z = z;
		Temp.w = w;
		Temp.x *= (1 / length);
		Temp.y *= (1 / length);
		Temp.z *= (1 / length);
		Temp.w *= (1 / length);

		return Temp;
	}

	template<class T> inline void Vector4<T>::Normalize()
	{
		if (x == 0 && y == 0 && z == 0 && w == 0)
			return;

		T length = Length();

		x *= (1 / length);
		y *= (1 / length);
		z *= (1 / length);
		w *= (1 / length);
	}

	template<class T> inline T Vector4<T>::Dot(const Vector4<T>& aVector) const
	{
		//Returns the dot product of this and aVector

		T result = (x * aVector.x) + (y * aVector.y) + (z * aVector.z) + (w * aVector.w);

		return result;
	}

}

typedef CommonUtilities::Vector4<float> Vector4f;
typedef CommonUtilities::Vector4<int> Vector4i;
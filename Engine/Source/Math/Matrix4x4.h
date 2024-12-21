#pragma once
#include <cmath>

#include "Vector4.h"
#include "Vector3.h"

namespace CommonUtilities
{
	template <class T>
	class Matrix4x4
	{
	public:
		// Creates the identity matrix.
		Matrix4x4<T>();
		// Copy Constructor.
		Matrix4x4<T>(const Matrix4x4<T>& aMatrix);

		// Static functions for creating rotation matrices.
		static Matrix4x4<T> CreateRotationAroundX(T aAngleInRadians);
		static Matrix4x4<T> CreateRotationAroundY(T aAngleInRadians);
		static Matrix4x4<T> CreateRotationAroundZ(T aAngleInRadians);

		// Static function for creating a transpose of a matrix.
		static Matrix4x4<T> Transpose(const Matrix4x4<T>& aMatrixToTranspose);

		operator DirectX::XMMATRIX() const
		{
			return *(DirectX::XMMATRIX*)this;
		}

		Matrix4x4<T>(const DirectX::XMMATRIX& aVector)
		{
			objects[0][0] = aVector.r[0].m128_f32[0];
			objects[0][1] = aVector.r[0].m128_f32[1];
			objects[0][2] = aVector.r[0].m128_f32[2];
			objects[0][3] = aVector.r[0].m128_f32[3];

			objects[1][0] = aVector.r[1].m128_f32[0];
			objects[1][1] = aVector.r[1].m128_f32[1];
			objects[1][2] = aVector.r[1].m128_f32[2];
			objects[1][3] = aVector.r[1].m128_f32[3];

			objects[2][0] = aVector.r[2].m128_f32[0];
			objects[2][1] = aVector.r[2].m128_f32[1];
			objects[2][2] = aVector.r[2].m128_f32[2];
			objects[2][3] = aVector.r[2].m128_f32[3];

			objects[3][0] = aVector.r[3].m128_f32[0];
			objects[3][1] = aVector.r[3].m128_f32[1];
			objects[3][2] = aVector.r[3].m128_f32[2];
			objects[3][3] = aVector.r[3].m128_f32[3];
		}


		// () operator for accessing element (row, column) for read/write or read, respectively.
		const T& operator()(const int aRow, const int aColumn) const;
		T& operator()(const int aRow, const int aColumn);
		void operator=(const Matrix4x4<T>& aMatrix);

		//scalar multiplication
		Matrix4x4<T> operator*(const T& aScalar) const
		{
			Matrix4x4<T> tempMatrix;

			for (int row = 1; row <= 4; row++)
			{
				for (int column = 1; column <= 4; column++)
				{
					tempMatrix(row, column) = objects[row - 1][column - 1] * aScalar;
				}
			}
			return tempMatrix;
		}


		// Assumes aTransform is made up of nothing but rotations and translations.
		static Matrix4x4<T> GetFastInverse(const Matrix4x4<T>& aTransform);

		// New additions needed for Transform class 2023-08-31
		static Matrix4x4<T> CreateScaleMatrix(Vector3f aScaleVector);
		static Matrix4x4<T> CreateTranslationMatrix(Vector3<T> aTranslationVector);
		static Matrix4x4<T> CreateRotationMatrix(Vector3<T> aRollPitchYaw);

	private:
		T objects[4][4] = {};
	};

	template <class T>
	T& Matrix4x4<T>::operator()(const int aRow, const int aColumn)
	{
		return objects[aRow - 1][aColumn - 1];
	}

	template <class T>
	const T& Matrix4x4<T>::operator()(const int aRow, const int aColumn) const
	{
		return objects[aRow - 1][aColumn - 1];
	}

	template <class T>
	void Matrix4x4<T>::operator=(const Matrix4x4<T>& aMatrix)
	{
		for (int row = 1; row <= 4; row++)
		{
			for (int column = 1; column <= 4; column++)
			{
				objects[row - 1][column - 1] = aMatrix(row, column);
			}
		}
	}

	template <class T>
	Matrix4x4<T> operator+(const Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1)
	{
		Matrix4x4<T> tempMatrix;

		for (int row = 1; row <= 4; row++)
		{
			for (int column = 1; column <= 4; column++)
			{
				tempMatrix(row, column) = aMatrix0(row, column) + aMatrix1(row, column);
			}
		}
		return tempMatrix;
	}

	template <class T>
	Matrix4x4<T> operator-(const Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1)
	{
		Matrix4x4<T> tempMatrix;
		for (int row = 1; row <= 4; row++)
		{
			for (int column = 1; column <= 4; column++)
			{
				tempMatrix(row, column) = aMatrix0(row, column) - aMatrix1(row, column);
			}
		}
		return tempMatrix;
	}

	template <class T>
	Matrix4x4<T> operator*(const Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1)
	{
		Matrix4x4<T> tempMatrix;

		for (int i = 1; i <= 4; i++)
		{
			for (int j = 1; j <= 4; j++)
			{
				tempMatrix(i, j) = aMatrix0(i, 1) * aMatrix1(1, j) + aMatrix0(i, 2) * aMatrix1(2, j) + aMatrix0(i, 3) *
					aMatrix1(3, j) + aMatrix0(i, 4) * aMatrix1(4, j);
			}
		}
		return tempMatrix;
	}

	template <class T>
	Vector4<T> operator*(const Vector4<T>& aVector, const Matrix4x4<T>& aMatrix)
	{
		Vector4<T> tempVector;
		tempVector.x = aVector.x * aMatrix(1, 1) + aVector.y * aMatrix(2, 1) + aVector.z * aMatrix(3, 1) + aVector.w *
			aMatrix(4, 1);
		tempVector.y = aVector.x * aMatrix(1, 2) + aVector.y * aMatrix(2, 2) + aVector.z * aMatrix(3, 2) + aVector.w *
			aMatrix(4, 2);
		tempVector.z = aVector.x * aMatrix(1, 3) + aVector.y * aMatrix(2, 3) + aVector.z * aMatrix(3, 3) + aVector.w *
			aMatrix(4, 3);
		tempVector.w = aVector.x * aMatrix(1, 4) + aVector.y * aMatrix(2, 4) + aVector.z * aMatrix(3, 4) + aVector.w *
			aMatrix(4, 4);

		return tempVector;
	}

	template <class T>
	Vector4<T> operator*(const Matrix4x4<T>& aMatrix, const Vector4<T>& aVector)
	{
		Vector4<T> tempVector;
		tempVector.x = aVector.x * aMatrix(1, 1) + aVector.y * aMatrix(2, 1) + aVector.z * aMatrix(3, 1) + aVector.w *
			aMatrix(4, 1);
		tempVector.y = aVector.x * aMatrix(1, 2) + aVector.y * aMatrix(2, 2) + aVector.z * aMatrix(3, 2) + aVector.w *
			aMatrix(4, 2);
		tempVector.z = aVector.x * aMatrix(1, 3) + aVector.y * aMatrix(2, 3) + aVector.z * aMatrix(3, 3) + aVector.w *
			aMatrix(4, 3);
		tempVector.w = aVector.x * aMatrix(1, 4) + aVector.y * aMatrix(2, 4) + aVector.z * aMatrix(3, 4) + aVector.w *
			aMatrix(4, 4);

		return tempVector;
	}

	template <class T>
	void operator+=(Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1)
	{
		for (int row = 1; row <= 4; row++)
		{
			for (int column = 1; column <= 4; column++)
			{
				aMatrix0(row, column) += aMatrix1(row, column);
			}
		}
	}

	template <class T>
	void operator-=(Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1)
	{
		for (int row = 1; row <= 4; row++)
		{
			for (int column = 1; column <= 4; column++)
			{
				aMatrix0(row, column) -= aMatrix1(row, column);
			}
		}
	}

	template <class T>
	void operator*=(Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1)
	{
		Matrix4x4<T> tempMatrix;

		for (int i = 1; i <= 4; i++)
		{
			for (int j = 1; j <= 4; j++)
			{
				tempMatrix(i, j) = aMatrix0(i, 1) * aMatrix1(1, j) + aMatrix0(i, 2) * aMatrix1(2, j) + aMatrix0(i, 3) *
					aMatrix1(3, j) + aMatrix0(i, 4) * aMatrix1(4, j);
			}
		}
		aMatrix0 = tempMatrix;
	}

	template <class T>
	bool operator==(const Matrix4x4<T>& aMatrix0, const Matrix4x4<T>& aMatrix1)
	{
		for (int row = 1; row <= 4; row++)
		{
			for (int column = 1; column <= 4; column++)
			{
				if (aMatrix0(row, column) != aMatrix1(row, column))
				{
					return false;
				}
			}
		}
		return true;
	}

	template <class T>
	inline Matrix4x4<T>::Matrix4x4(const Matrix4x4<T>& aMatrix)
	{
		for (int row = 1; row <= 4; row++)
		{
			for (int column = 1; column <= 4; column++)
			{
				objects[row - 1][column - 1] = aMatrix(row, column);
			}
		}
	}

	template <class T>
	inline Matrix4x4<T>::Matrix4x4()
	{
		for (int row = 1; row <= 4; row++)
		{
			for (int column = 1; column <= 4; column++)
			{
				if (row == column)
				{
					objects[row - 1][column - 1] = 1;
				}
				else
				{
					objects[row - 1][column - 1] = 0;
				}
			}
		}
	}

	template <class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateRotationAroundX(T aAngleInRadians)
	{
		Matrix4x4<T> temp;
		temp(2, 2) = std::cos(aAngleInRadians);
		temp(2, 3) = std::sin(aAngleInRadians);
		temp(3, 2) = -std::sin(aAngleInRadians);
		temp(3, 3) = std::cos(aAngleInRadians);

		return temp;
	}

	template <class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateRotationAroundY(T aAngleInRadians)
	{
		Matrix4x4<T> temp;
		temp(1, 1) = std::cos(aAngleInRadians);
		temp(1, 3) = -std::sin(aAngleInRadians);
		temp(3, 1) = std::sin(aAngleInRadians);
		temp(3, 3) = std::cos(aAngleInRadians);

		return temp;
	}

	template <class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateRotationAroundZ(T aAngleInRadians)
	{
		Matrix4x4<T> temp;
		temp(1, 1) = std::cos(aAngleInRadians);
		temp(1, 2) = std::sin(aAngleInRadians);
		temp(2, 1) = -std::sin(aAngleInRadians);
		temp(2, 2) = std::cos(aAngleInRadians);

		return temp;
	}

	template <class T>
	inline Matrix4x4<T> Matrix4x4<T>::Transpose(const Matrix4x4<T>& aMatrixToTranspose)
	{
		Matrix4x4<T> temp = Matrix4x4<T>(aMatrixToTranspose);

		temp(2, 1) = aMatrixToTranspose(1, 2);
		temp(1, 2) = aMatrixToTranspose(2, 1);

		temp(3, 1) = aMatrixToTranspose(1, 3);
		temp(1, 3) = aMatrixToTranspose(3, 1);

		temp(3, 2) = aMatrixToTranspose(2, 3);
		temp(2, 3) = aMatrixToTranspose(3, 2);

		temp(4, 1) = aMatrixToTranspose(1, 4);
		temp(1, 4) = aMatrixToTranspose(4, 1);

		temp(4, 2) = aMatrixToTranspose(2, 4);
		temp(2, 4) = aMatrixToTranspose(4, 2);

		temp(4, 3) = aMatrixToTranspose(3, 4);
		temp(3, 4) = aMatrixToTranspose(4, 3);

		return temp;
	}

	template <class T>
	inline Matrix4x4<T> Matrix4x4<T>::GetFastInverse(const Matrix4x4<T>& aTransform)
	{
		Matrix4x4<T> newMatrix = aTransform;

		newMatrix(2, 1) = aTransform(1, 2);
		newMatrix(1, 2) = aTransform(2, 1);

		newMatrix(3, 1) = aTransform(1, 3);
		newMatrix(1, 3) = aTransform(3, 1);

		newMatrix(3, 2) = aTransform(2, 3);
		newMatrix(2, 3) = aTransform(3, 2);

		newMatrix(4, 1) = -aTransform(4, 1) * newMatrix(1, 1) - aTransform(4, 2) * newMatrix(2, 1) - aTransform(4, 3) *
			newMatrix(3, 1);
		newMatrix(4, 2) = -aTransform(4, 1) * newMatrix(1, 2) - aTransform(4, 2) * newMatrix(2, 2) - aTransform(4, 3) *
			newMatrix(3, 2);
		newMatrix(4, 3) = -aTransform(4, 1) * newMatrix(1, 3) - aTransform(4, 2) * newMatrix(2, 3) - aTransform(4, 3) *
			newMatrix(3, 3);

		return newMatrix;
	}

	template <class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateScaleMatrix(Vector3f aScaleVector)
	{
		Matrix4x4<T> result;
		result(1, 1) = aScaleVector.x;
		result(2, 2) = aScaleVector.y;
		result(3, 3) = aScaleVector.z;

		//result.myData[0] = aScaleVector.X;
		//result.myData[5] = aScaleVector.Y;
		//result.myData[10] = aScaleVector.Z;
		return result;
	}

	template <class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateTranslationMatrix(Vector3<T> aTranslationVector)
	{
		Matrix4x4<T> result;

		result(4, 1) = aTranslationVector.x;
		result(4, 2) = aTranslationVector.y;
		result(4, 3) = aTranslationVector.z;

		return result;
	}

	template <class T>
	inline Matrix4x4<T> Matrix4x4<T>::CreateRotationMatrix(Vector3<T> aRollPitchYaw)
	{
		Matrix4x4<T> result;

		float cp = cosf(aRollPitchYaw.x);
		float sp = sinf(aRollPitchYaw.x);

		float cy = cosf(aRollPitchYaw.y);
		float sy = sinf(aRollPitchYaw.y);

		float cr = cosf(aRollPitchYaw.z);
		float sr = sinf(aRollPitchYaw.z);

		result(1, 1) = cr * cy + sr * sp * sy;
		result(1, 2) = sr * cp;
		result(1, 3) = sr * sp * cy - cr * sy;
		result(1, 4) = 0.0f;

		result(2, 1) = cr * sp * sy - sr * cy;
		result(2, 2) = cr * cp;
		result(2, 3) = sr * sy + cr * sp * cy;
		result(2, 4) = 0.0f;

		result(3, 1) = cp * sy;
		result(3, 2) = -sp;
		result(3, 3) = cp * cy;
		result(3, 4) = 0.0f;

		result(4, 1) = 0.0f;
		result(4, 2) = 0.0f;
		result(4, 3) = 0.0f;
		result(4, 4) = 1.0f;

		return result;
	}
}

typedef CommonUtilities::Matrix4x4<float> Matrix4x4f;
typedef CommonUtilities::Matrix4x4<int> Matrix4x4i;

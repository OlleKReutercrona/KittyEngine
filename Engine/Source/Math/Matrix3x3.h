	#pragma once
#include <cmath>
#include "Vector3.h"
#include "Matrix4x4.h"

namespace CommonUtilities
{
	template <class T>
	class Matrix3x3
	{
	public:

		// Creates the identity matrix.
		Matrix3x3<T>();
		// Copy Constructor.
		Matrix3x3<T>(const Matrix3x3<T>& aMatrix);
		// Copies the top left 3x3 part of the Matrix4x4.
		Matrix3x3<T>(const Matrix4x4<T>& aMatrix);
		

		// Static functions for creating rotation matrices.
		static Matrix3x3<T> CreateRotationAroundX(T aAngleInRadians);
		static Matrix3x3<T> CreateRotationAroundY(T aAngleInRadians);
		static Matrix3x3<T> CreateRotationAroundZ(T aAngleInRadians);

		// Static function for creating a transpose of a matrix.
		static Matrix3x3<T> Transpose(const Matrix3x3<T>& aMatrixToTranspose);

		// () operator for accessing element (row, column) for read/write or read, respectively.
		const T& operator()(const int aRow, const int aColumn) const;
		T& operator()(const int aRow, const int aColumn);
		void operator=(const Matrix3x3<T>& aMatrix);

	private:
		T objects[3][3] = {};
	};
	
	template <class T> T& Matrix3x3<T>::operator()(const int aRow, const int aColumn)
	{
		return objects[aRow - 1][ aColumn - 1];
	}
	template <class T> const T& Matrix3x3<T>::operator()(const int aRow, const int aColumn) const
	{
		return objects[aRow - 1][ aColumn - 1];
	}
	template <class T> void Matrix3x3<T>::operator=(const Matrix3x3<T>& aMatrix)
	{
		for (int row = 1; row <= 3; row++)
		{
			for (int column = 1; column <= 3; column++)
			{
				objects[row - 1][column - 1] = aMatrix(row, column);
			}
		}
	}
	template <class T> Matrix3x3<T> operator+(const Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		Matrix3x3<T> tempMatrix;
		for (int row = 1; row <= 3; row++)
		{
			for (int column = 1; column <= 3; column++)
			{
				tempMatrix(row, column) = aMatrix0(row, column) + aMatrix1(row, column);
			}
		}
		return tempMatrix;
	}
	template <class T> Matrix3x3<T> operator-(const Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		Matrix3x3<T> tempMatrix;
		for (int row = 1; row <= 3; row++)
		{
			for (int column = 1; column <= 3; column++)
			{
				tempMatrix(row, column) = aMatrix0(row, column) - aMatrix1(row,column);
			}
		}
		return tempMatrix;
	}
	template <class T> Matrix3x3<T> operator*(const Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		Matrix3x3<T> tempMatrix;

		for (T i = 1; i <= 3; i++)
		{
			for (T j = 1; j <= 3; j++) 
			{
				tempMatrix(i, j) = aMatrix0(i, 1) * aMatrix1(1, j) + aMatrix0(i, 2) * aMatrix1(2, j) + aMatrix0(i, 3) * aMatrix1(3, j);
			}
		}

		return tempMatrix;
	}
	template <class T> Vector3<T> operator*(const Vector3<T>& aVector, const Matrix3x3<T>& aMatrix)
	{
		Vector3<T> tempVector;
		tempVector.x = aVector.x * aMatrix(1,1) + aVector.y * aMatrix(2,1) + aVector.z * aMatrix(3,1);
		tempVector.y = aVector.x * aMatrix(1,2) + aVector.y * aMatrix(2,2) + aVector.z * aMatrix(3,2);
		tempVector.z = aVector.x * aMatrix(1,3) + aVector.y * aMatrix(2,3) + aVector.z * aMatrix(3,3);
		return tempVector;
	}
	template <class T> Vector3<T> operator*(const Matrix3x3<T>& aMatrix, const Vector3<T>& aVector)
	{
		Vector3<T> tempVector;
		tempVector.x = aVector.x * aMatrix(1,1) + aVector.y * aMatrix(2,1) + aVector.z * aMatrix(3,1);
		tempVector.y = aVector.x * aMatrix(1,2) + aVector.y * aMatrix(2,2) + aVector.z * aMatrix(3,2);
		tempVector.z = aVector.x * aMatrix(1,3) + aVector.y * aMatrix(2,3) + aVector.z * aMatrix(3,3);
		return tempVector;
	}
	template <class T> void operator+=(Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		for (int row = 1; row <= 3; row++)
		{
			for (int column = 1; column <= 3; column++)
			{
				aMatrix0(row, column) += aMatrix1(row, column);
			}
		}
	}
	template <class T> void operator-=(Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		for (int row = 1; row <= 3; row++)
		{
			for (int column = 1; column <= 3; column++)
			{
				aMatrix0(row,column) -= aMatrix1(row, column);
			}
		}
	}
	template <class T> void operator*=(Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		Matrix3x3<T> tempMatrix;

		for (T row = 1; row <= 3; row++)
		{
			for (T column = 1; column <= 3; column++)
			{
				tempMatrix(row, column) = aMatrix0(row, 1) * aMatrix1(1, column) + aMatrix0(row, 2) * aMatrix1(2, column) + aMatrix0(row, 3) * aMatrix1(3, column);
			}
		}
		aMatrix0 = tempMatrix;
	}
	template <class T> bool operator==(const Matrix3x3<T>& aMatrix0, const Matrix3x3<T>& aMatrix1)
	{
		for (int row = 1; row <= 3; row++)
		{
			for (int column = 1; column <= 3; column++)
			{
				if (aMatrix0(row, column) != aMatrix1(row, column))
				{
					return false;
				}
			}
		}
		return true;
	}

	template<class T> inline Matrix3x3<T>::Matrix3x3()
	{
		for (int row = 1; row <= 3; row++)
		{
			for (int column = 1; column <= 3; column++)
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
	template<class T> inline Matrix3x3<T>::Matrix3x3(const Matrix3x3<T>& aMatrix)
	{
		for (int row = 1; row <= 3; row++)
		{
			for (int column = 1; column <= 3; column++)
			{
				objects[row - 1][column - 1] = aMatrix(row, column);
			}
		}
	}
	template<class T> inline Matrix3x3<T>::Matrix3x3(const Matrix4x4<T>& aMatrix)
	{
		for (int row = 1; row <= 3; row++)
		{
			for (int column = 1; column <= 3; column++)
			{
				objects[row-1][column-1] = aMatrix(row,column);
			}
		}
	}

	template<class T> inline Matrix3x3<T> Matrix3x3<T>::CreateRotationAroundX(T aAngleInRadians)
	{
		Matrix3x3<T> temp;
		temp(2,2) = std::cos(aAngleInRadians);
		temp(2,3) = std::sin(aAngleInRadians);
		temp(3,2) = -std::sin(aAngleInRadians);
		temp(3,3) = std::cos(aAngleInRadians);

		return temp;
	}
	template<class T> inline Matrix3x3<T> Matrix3x3<T>::CreateRotationAroundY(T aAngleInRadians)
	{
		Matrix3x3<T> temp;
		temp(1,1) = std::cos(aAngleInRadians);
		temp(1,3) = -std::sin(aAngleInRadians);
		temp(3,1) = std::sin(aAngleInRadians);
		temp(3,3) = std::cos(aAngleInRadians);

		return temp;
	}
	template<class T> inline Matrix3x3<T> Matrix3x3<T>::CreateRotationAroundZ(T aAngleInRadians)
	{
		Matrix3x3<T> temp;
		temp(1,1) = std::cos(aAngleInRadians);
		temp(1,2) = std::sin(aAngleInRadians);
		temp(2,1) = -std::sin(aAngleInRadians);
		temp(2,2) = std::cos(aAngleInRadians);

		return temp;
	}

	template<class T> inline Matrix3x3<T> Matrix3x3<T>::Transpose(const Matrix3x3<T>& aMatrixToTranspose)
	{
		Matrix3x3<T> temp = Matrix3x3<T>(aMatrixToTranspose);

		temp(2,1) = aMatrixToTranspose(1,2);
		temp(1,2) = aMatrixToTranspose(2,1);

		temp(3,1) = aMatrixToTranspose(1,3);
		temp(1,3) = aMatrixToTranspose(3,1);

		temp(3,2) = aMatrixToTranspose(2,3);
		temp(2,3) = aMatrixToTranspose(3,2);

		return temp;
	}

}

typedef CommonUtilities::Matrix3x3<float> Matrix3x3f;
typedef CommonUtilities::Matrix3x3<int> Matrix3x3i;
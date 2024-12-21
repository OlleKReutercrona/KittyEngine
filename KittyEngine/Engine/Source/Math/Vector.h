#pragma once

#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

template <typename T>
CommonUtilities::Vector3<T> CommonUtilities::Vector4<T>::xyz()
{
	return Vector3<T>(x, y, z);
}

template <typename T>
CommonUtilities::Vector4<T>::Vector4(CommonUtilities::Vector3<T> aVector, T aW)
{
	x = aVector.x;
	y = aVector.y;
	z = aVector.z;
	w = aW;
}
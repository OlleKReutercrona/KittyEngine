#pragma once
#include <Engine/Source/Math/Vector3.h>

enum class eShapes
{
	eNULL,
	eBox,
	eSphere,
	eCapsule,
};

namespace KE
{
	class Shape
	{
	public:
		Shape(const Vector3f& aPosition) : myPosition(aPosition) {};
		virtual ~Shape() {};

		eShapes myShapeData = eShapes::eNULL;
		Vector3f myOffset = {0,0,0};
		const Vector3f& myPosition;
	protected:
	};



	class Sphere : public Shape
	{
	public:
		Sphere(const float aRadius, Vector3f& aPosition) : 
			myRadius(aRadius), Shape(aPosition) 
		{
			myShapeData = eShapes::eSphere;
		};
		~Sphere() {};

		float myRadius;
	private:
	};



	class Box : public Shape
	{
	public:
		Box(Vector3f aMin, Vector3f aMax, const Vector3f& aPosition) : myMin(aMin), myMax(aMax), Shape(aPosition)
		{
			myShapeData = eShapes::eBox;
		}
		Box(float aSize, const Vector3f& aPosition) : Shape(aPosition)
		{
			myMin.x = -(aSize / 2.0f);
			myMin.y = -(aSize / 2.0f);
			myMin.z = -(aSize / 2.0f);

			myMax.x = (aSize / 2.0f);
			myMax.y = (aSize / 2.0f);
			myMax.z = (aSize / 2.0f);

			myShapeData = eShapes::eBox;

		}
		~Box() {};

		Vector3f myMin;
		Vector3f myMax;
	protected:
	};

	class Capsule
	{
	public:
		Capsule(const float aRadius, const float aLength)
		{
			radius = aRadius;
			length = aLength;

			myShapeData = eShapes::eCapsule;
		}

		float radius = 0.0f;
		float length = 0.0f;
		eShapes myShapeData = eShapes::eNULL;
	};
}
#pragma once
#include "Engine/Source/Math/Ray.h"
#include "Engine/Source/Math/Plane.h"
#include "Engine/Source/Math/Line.h"

#include "Engine/Source/Collision/Shape.h"


// Good info about collision tests: http://www.miguelcasillas.com/?mcportfolio=collision-detection-c

template <typename T>
T Min(T a, T b) {
	return a < b ? a : b;
}

// Intersect Sorters
inline bool Intersect(const KE::Shape* aFirst, const KE::Shape* aSecond);
inline bool Intersect(const KE::Shape* aShape, Rayf& aRay, Vector3f& aOutIntersectionPoint);
inline bool Intersect(const KE::Shape* aShape, Rayf& aRay);

// Ray Tests
inline bool IntersectBoxRay(const KE::Box& aBox, Rayf& aRay);
inline bool IntersectBoxRay(const KE::Box& aSource, Rayf& aRay, Vector3f& aOutIntersectionPoint);
inline bool IntersectionPlaneRay(const Plane& aPlane, const Rayf& aRay, Vector3f& aOutIntersectionPoint);
inline bool IntersectSphereRay(const KE::Sphere& aSphere, const Rayf& aRay);
inline bool IntersectSphereRay(const KE::Sphere& aSphere, const Rayf& aRay, Vector3f& aOutIntersectionPoint);

// Box Tests
inline bool Intersect(const KE::Box& aSource, const KE::Box& aCollider);
inline bool IntersectBoxPoint(const KE::Box aBoxCollider, const Vector3f& aPosition);
inline bool IntersectBoxPoint(const KE::Box aBoxCollider, const Vector3f& aPosition, const Vector3f& aNormal);
inline bool IntersectBoxPoint(const Vector2f& aBox, const Vector2f& aPoint);

// Sphere tests
inline float SqDistPointAABB(const Vector3f& aPoint, const KE::Box& aBox);
inline bool Intersect(const KE::Box& aBox, const KE::Sphere& aSphere);
inline bool Intersect(const KE::Sphere& aFirst, const KE::Sphere& aSecond);
inline bool IntersectSpherePoint(const KE::Sphere& aFirst, const Vector3f& aSecond);

inline bool IntersectLineLine(Linef& aLine0, Linef& aLine1, Vector2f& aOutIntersectPoint);
//inline bool IntersectLineLineTEST(Linef& aLine0, Linef& aLine1, Vector2f& aOutIntersectPoint);
///	\--------------------------------------------/
/// |	\\\\\\+------------------------+//////	 |
/// |	<<<<<<		   RAY TESTS		>>>>>>	 |
/// |	//////+------------------------+\\\\\\	 |
/// /--------------------------------------------\

inline bool IntersectBoxRay(const KE::Box& aBox, Rayf& aRay)
{
	// r.dir is unit direction vector of ray
	Vector3f dirfrac;
	dirfrac.x = 1.0f / aRay.GetDirection().x;
	dirfrac.y = 1.0f / aRay.GetDirection().y;
	dirfrac.z = 1.0f / aRay.GetDirection().z;
	// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
	// r.org is origin of ray
    float t1 = (aBox.myMin.x + aBox.myPosition.x + aBox.myOffset.x - aRay.GetOrigin().x) * dirfrac.x;
	float t2 = (aBox.myMax.x + aBox.myPosition.x + aBox.myOffset.x - aRay.GetOrigin().x) * dirfrac.x;

	float t3 = (aBox.myMin.y + aBox.myPosition.y + aBox.myOffset.y - aRay.GetOrigin().y) * dirfrac.y;
	float t4 = (aBox.myMax.y + aBox.myPosition.y + aBox.myOffset.y - aRay.GetOrigin().y) * dirfrac.y;

	float t5 = (aBox.myMin.z + aBox.myPosition.z + aBox.myOffset.z - aRay.GetOrigin().z) * dirfrac.z;
	float t6 = (aBox.myMax.z + aBox.myPosition.z + aBox.myOffset.z - aRay.GetOrigin().z) * dirfrac.z;

	float tmin = fmax(fmax(fmin(t1, t2), fmin(t3, t4)), fmin(t5, t6));
	float tmax = fmin(fmin(fmax(t1, t2), fmax(t3, t4)), fmax(t5, t6));

	float t;
	// if tmax < 0, ray (line) is intersecting AABB, but the whole AABB is behind us
	if (tmax < 0)
	{
		t = tmax;
		return false;
	}

	// if tmin > tmax, ray doesn't intersect AABB
	if (tmin > tmax)
	{
		t = tmax;
		return false;
	}

	t = tmin;
	return true;
}

inline bool IntersectBoxRay(const KE::Box& aSource, Rayf& aRay, Vector3f& aOutIntersectionPoint)
{
	Plane planes[6];
	planes[0] = Plane(aSource.myMin + aSource.myPosition + aSource.myOffset, Vector3f(-1, 0, 0));
	planes[1] = Plane(aSource.myMin + aSource.myPosition + aSource.myOffset, Vector3f(0, -1, 0));
	planes[2] = Plane(aSource.myMin + aSource.myPosition + aSource.myOffset, Vector3f(0, 0, -1));
	planes[3] = Plane(aSource.myMax + aSource.myPosition + aSource.myOffset, Vector3f(1, 0, 0));
	planes[4] = Plane(aSource.myMax + aSource.myPosition + aSource.myOffset, Vector3f(0, 1, 0));
	planes[5] = Plane(aSource.myMax + aSource.myPosition + aSource.myOffset, Vector3f(0, 0, 1));

   	float intersectionDistance = FLT_MAX;
	Vector3f closestIntersectionPoint;
	for (int i = 0; i < 6; i++)
	{
		if (IntersectionPlaneRay(planes[i], aRay, aOutIntersectionPoint))
		{
			if (IntersectBoxPoint(aSource, aOutIntersectionPoint, planes[i].GetNormal()))
			{
				Vector3f distance = aOutIntersectionPoint - aRay.GetOrigin();

				if (distance.Length() < intersectionDistance && distance.Length() > 0.001f)
				{
					intersectionDistance = static_cast<float>(distance.Length());
					closestIntersectionPoint = aOutIntersectionPoint;
				}
			}
		}
	}
	if (intersectionDistance == FLT_MAX)
	{
		return false;
	}

	aOutIntersectionPoint = closestIntersectionPoint;
	return true;
}

inline bool IntersectionPlaneRay(const Plane& aPlane, const Rayf& aRay, Vector3f& aOutIntersectionPoint)
{
	float d = aPlane.GetPoint().Dot(aPlane.GetNormal());
	float distance = (d - aRay.GetOrigin().Dot(aPlane.GetNormal())) / (aRay.GetDirection().Dot(aPlane.GetNormal()));
	if (aPlane.IsOnPlane(aRay.GetOrigin()))
	{
		aOutIntersectionPoint = aRay.GetOrigin();
		return true;
	}

	if (abs(aRay.GetDirection().Dot(aPlane.GetNormal())) < 0.0001f)
	{
		return false;
	}
	if (distance < 0)
	{
		return false;
	}

	aOutIntersectionPoint = aRay.GetOrigin() + aRay.GetDirection().GetNormalized() * distance;
	return true;
}

inline bool IntersectSphereRay(const KE::Sphere& aSphere, const Rayf& aRay)
{
	Vector3f rayToSphereCenter = aSphere.myPosition - aRay.GetOrigin();

	float midCornerToSphereCenter = rayToSphereCenter.Dot(aRay.GetDirection()) / aRay.GetDirection().Length();

	if (midCornerToSphereCenter < 0)
	{
		return false;
	}

	float rayOrginToSphereCenter = sqrt(rayToSphereCenter.LengthSqr() - midCornerToSphereCenter * midCornerToSphereCenter);

	if (rayOrginToSphereCenter > aSphere.myRadius)
	{
		return false;
	}

	return true;
}

inline bool IntersectSphereRay(const KE::Sphere& aSphere, const Rayf& aRay, Vector3f& aOutIntersectionPoint)
{
	Vector3f rayToSphereCenter = aSphere.myPosition - aRay.GetOrigin();

	float midCornerToSphereCenter = rayToSphereCenter.Dot(aRay.GetDirection()) / aRay.GetDirection().Length();

	if (midCornerToSphereCenter < 0)
	{
		return false;
	}

	float rayOrginToSphereCenter = sqrt(rayToSphereCenter.LengthSqr() - midCornerToSphereCenter * midCornerToSphereCenter);

	if (rayOrginToSphereCenter > aSphere.myRadius)
	{
		return false;
	}

	float interSectToMidCorner = sqrt(aSphere.myRadius * aSphere.myRadius - rayOrginToSphereCenter * rayOrginToSphereCenter);

	aOutIntersectionPoint = aRay.GetOrigin() + aRay.GetDirection() * (midCornerToSphereCenter - interSectToMidCorner);
	return true;
}

///	\--------------------------------------------/
/// |	\\\\\\+------------------------+//////	 |
/// |	<<<<<<	  INTERSECT SORTERS		>>>>>>	 |
/// |	//////+------------------------+\\\\\\	 |
/// /--------------------------------------------\

inline bool Intersect(const KE::Shape* aFirst, const KE::Shape* aSecond)
{
	//const KE::Box* box1 = nullptr;
	//const KE::Box* box2 = nullptr;
	//const KE::Sphere* sphere1 = nullptr;
	//const KE::Sphere* sphere2 = nullptr;

	//switch (aFirst->myShapeData)
	//{
	//	case eShapes::eBox:
	//	{
	//		box1 = (KE::Box*)aFirst;
	//	}
	//	case eShapes::eSphere:
	//	{
	//		sphere1 = (KE::Sphere*)aFirst;
	//	}
	//}

	//switch (aSecond->myShapeData)
	//{
	//	case eShapes::eBox:
	//	{
	//		box2 = (KE::Box*)aSecond;
	//	}
	//	case eShapes::eSphere:
	//	{
	//		sphere2 = (KE::Sphere*)aSecond;
	//	}
	//}

	//Intersect(
	//	box1 != nullptr ? *box1 : 
	//		box2 != nullptr ? *box2 : 
	//			sphere1 != nullptr ? *sphere1 : 
	//				*sphere2,
	//	box1 != nullptr ? *box1 : 
	//		box2 != nullptr ? *box2 : 
	//			sphere1 != nullptr ? *sphere1 : 
	//				*sphere2);

	//Intersect(*box1, *sphere1);

	switch (aFirst->myShapeData)
	{
		case eShapes::eBox: 
		{ 
			switch (aSecond->myShapeData)
			{
				case eShapes::eBox:
				{
					const KE::Box* box1 = dynamic_cast<const KE::Box*>(aFirst);
					const KE::Box* box2 = dynamic_cast<const KE::Box*>(aSecond);

					return Intersect(*box1, *box2);
				}
				case eShapes::eSphere:
				{
					const KE::Box* box = dynamic_cast<const KE::Box*>(aFirst);
					const KE::Sphere* sphere = dynamic_cast<const KE::Sphere*>(aSecond);

					return Intersect(*box, *sphere);
				}
			}
		}
		case eShapes::eSphere: 
		{
			switch (aSecond->myShapeData)
			{
			case eShapes::eBox:
			{
				const KE::Sphere* sphere = dynamic_cast<const KE::Sphere*>(aFirst);
				const KE::Box* box = dynamic_cast<const KE::Box*>(aSecond);

				return Intersect(*box, *sphere);
			}
			case eShapes::eSphere:
			{
				const KE::Sphere* sphere1 = dynamic_cast<const KE::Sphere*>(aFirst);
				const KE::Sphere* sphere2 = dynamic_cast<const KE::Sphere*>(aSecond);

				return Intersect(*sphere1, *sphere2);
			}
			break;
			}
		}
		default:
			break;
	}

	return false;
}

inline bool Intersect(const KE::Shape* aShape, Rayf& aRay, Vector3f& aOutIntersectionPoint)
{
	switch (aShape->myShapeData)
	{
	case eShapes::eBox:
	{
		return IntersectBoxRay(*dynamic_cast<const KE::Box*>(aShape), aRay, aOutIntersectionPoint);
	}
	case eShapes::eSphere:
	{
		return IntersectSphereRay(*dynamic_cast<const KE::Sphere*>(aShape), aRay, aOutIntersectionPoint);
	}
	default:
		break;
	}

	return false;
}

inline bool Intersect(const KE::Shape* aShape, Rayf& aRay)
{
	switch (aShape->myShapeData)
	{
	case eShapes::eBox:
	{
		return IntersectBoxRay(*dynamic_cast<const KE::Box*>(aShape), aRay);
	}
	case eShapes::eSphere: 
	{
		return IntersectSphereRay(*dynamic_cast<const KE::Sphere*>(aShape), aRay);
	}
	default:
		break;
	}

	return false;
}

///	\--------------------------------------------/
/// |	\\\\\\+------------------------+//////	 |
/// |	<<<<<<	      BOX TESTS			>>>>>>	 |
/// |	//////+------------------------+\\\\\\	 |
/// /--------------------------------------------\

inline bool Intersect(const KE::Box& aBox1, const KE::Box& aBox2)
{
	bool xL = aBox1.myMax.x + aBox1.myPosition.x  + aBox1.myOffset.x > aBox2.myMin.x + aBox2.myPosition.x  + aBox2.myOffset.x;
	bool xR = aBox1.myMin.x + aBox1.myPosition.x  + aBox1.myOffset.x < aBox2.myMax.x + aBox2.myPosition.x  + aBox2.myOffset.x;
	bool yL = aBox1.myMax.y + aBox1.myPosition.y  + aBox1.myOffset.y > aBox2.myMin.y + aBox2.myPosition.y  + aBox2.myOffset.y;
	bool yR = aBox1.myMin.y + aBox1.myPosition.y  + aBox1.myOffset.y < aBox2.myMax.y + aBox2.myPosition.y  + aBox2.myOffset.y;
	bool zL = aBox1.myMax.z + aBox1.myPosition.z  + aBox1.myOffset.z > aBox2.myMin.z + aBox2.myPosition.z  + aBox2.myOffset.z;
	bool zR = aBox1.myMin.z + aBox1.myPosition.z  + aBox1.myOffset.z < aBox2.myMax.z + aBox2.myPosition.z  + aBox2.myOffset.z;

	return 	xL && xR && yL && yR && zL && zR;
}

// Returns whether a position is inside the box or not
inline bool IntersectBoxPoint(const KE::Box aBoxCollider, const Vector3f& aPosition)
{
	return (
		((aPosition.x >= aBoxCollider.myMin.x) && (aPosition.x <= aBoxCollider.myMax.x)) &&
		((aPosition.y >= aBoxCollider.myMin.y) && (aPosition.y <= aBoxCollider.myMax.y)) &&
		((aPosition.z >= aBoxCollider.myMin.z) && (aPosition.z <= aBoxCollider.myMax.z))
		);
}

inline bool IntersectBoxPoint(const Vector2f& aBox, const Vector2f& aPoint)
{
	Vector2f minB = { -aBox.x, -aBox.y };
	Vector2f maxB = { aBox.x, aBox.y };

	bool xR = (aPoint.x >= minB.x);
	bool xL = (aPoint.x <= maxB.x);
	bool yR = (aPoint.y >= minB.y);
	bool yL = (aPoint.y <= maxB.y);

	bool returnValue = xR && xL && yR && yL;

	return (
		((aPoint.x >= minB.x) && (aPoint.x <= maxB.x)) &&
		((aPoint.y >= minB.y) && (aPoint.y <= maxB.y))
		);
}

inline bool IntersectBoxPoint(const KE::Box aBoxCollider, const Vector3f& aPosition, const Vector3f& aNormal)
{
	Vector3f low  = aBoxCollider.myMin + aBoxCollider.myPosition + aBoxCollider.myOffset;
	Vector3f high = aBoxCollider.myMax + aBoxCollider.myPosition + aBoxCollider.myOffset;
	return (
		((aPosition.x >= low.x) && (aPosition.x <= high.x) || aNormal.x != 0) &&
		((aPosition.y >= low.y) && (aPosition.y <= high.y) || aNormal.y != 0) &&
		((aPosition.z >= low.z) && (aPosition.z <= high.z) || aNormal.z != 0)
		);
}

///	\--------------------------------------------/
/// |	\\\\\\+------------------------+//////	 |
/// |	<<<<<<		SPHERE TESTS		>>>>>>	 |
/// |	//////+------------------------+\\\\\\	 |
/// /--------------------------------------------\

inline float SqDistPointAABB(const Vector3f& aPoint, const KE::Box& aBox)
{
	float sqDist = 0.0f;
	Vector3f min = aBox.myMin + aBox.myPosition + aBox.myOffset;
	Vector3f max = aBox.myMax + aBox.myPosition + aBox.myOffset;

	for (unsigned int i = 0; i < 3; i++)
	{
		float v = aPoint[i];

		if (v < min[i])
		{
			sqDist += (min[i] - v) * (min[i] - v);
		}

		if (v > max[i])
		{
			sqDist += (v - max[i]) * (v - max[i]);
		}
	}

	return sqDist;
}

inline bool Intersect(const KE::Box& aBox, const KE::Sphere& aSphere)
{
	/// https://gamedev.stackexchange.com/questions/156870/how-do-i-implement-a-aabb-sphere-collision

	float sqDist = SqDistPointAABB(aSphere.myPosition + aSphere.myOffset, aBox);


	return sqDist <= aSphere.myRadius * aSphere.myRadius;
}

inline bool Intersect(const KE::Sphere& aFirst, const KE::Sphere& aSecond)
{
	/// https://studiofreya.com/3d-math-and-physics/simple-sphere-sphere-collision-detection-and-collision-response/

	Vector3f distVector = (aFirst.myPosition + aFirst.myOffset) - (aSecond.myPosition + aSecond.myOffset);
	float dist = distVector.Length();

	float sumRadius = aFirst.myRadius + aSecond.myRadius;


	return dist < sumRadius;
}

inline bool IntersectSpherePoint(const KE::Sphere& aSphere, const Vector3f& aPoint)
{
	/// https://math.stackexchange.com/questions/3118238/check-if-3d-point-is-inside-sphere

	double d = sqrt(
		pow(aPoint.x - aSphere.myPosition.x + aSphere.myOffset.x, 2) +
		pow(aPoint.y - aSphere.myPosition.y + aSphere.myOffset.y, 2) +
		pow(aPoint.z - aSphere.myPosition.z + aSphere.myOffset.z, 2));

	return d < aSphere.myRadius;
}

///	\--------------------------------------------/
/// |	\\\\\\+------------------------+//////	 |
/// |	<<<<<<		  LINE TESTS		>>>>>>	 |
/// |	//////+------------------------+\\\\\\	 |
/// /--------------------------------------------\

inline bool IntersectLineLine(Linef& aLine0, Linef& aLine1, Vector2f& aOutIntersectPoint)
{
		float x1 = aLine0.myPoint.x;
		float y1 = aLine0.myPoint.y;
		float x2 = aLine1.myPoint.x;
		float y2 = aLine1.myPoint.y;

		float m1 = aLine0.myDirection.y / aLine0.myDirection.x;
		float m2 = aLine1.myDirection.y / aLine1.myDirection.x;

		if (!std::isfinite(abs(m1)) && !std::isfinite(abs(m2))) {
			// Lines are both vertical (perpendicular), and they intersect at (x1, y2) or (x2, y1).
			aOutIntersectPoint.x = x1;
			aOutIntersectPoint.y = y2;
			return true;
		}

		if (!std::isfinite(abs(m1))) {
			// Line aLine0 is vertical, and the intersection is calculated using the equation of line aLine1.
			aOutIntersectPoint.x = x1;
			aOutIntersectPoint.y = m2 * (x1 - x2) + y2;
			return true;
		}

		if (!std::isfinite(abs(m2))) {
			// Line aLine1 is vertical, and the intersection is calculated using the equation of line aLine0.
			aOutIntersectPoint.x = x2;
			aOutIntersectPoint.y = m1 * (x2 - x1) + y1;
			return true;
		}

		if (m1 != m2) {
			aOutIntersectPoint.x = (y2 - y1 + m1 * x1 - m2 * x2) / (m1 - m2);
			aOutIntersectPoint.y = m1 * (aOutIntersectPoint.x - x1) + y1;
			return true;
		}

		return false;
	

	// <<[PREVIOUS IMPLEMENTATION - 30 OCT]>> //
	/*float x1 = aLine0.myPoint.x;
	float y1 = aLine0.myPoint.y;
	float x2 = aLine1.myPoint.x;
	float y2 = aLine1.myPoint.y;
	
	float m1 = aLine0.myDirection.y / aLine0.myDirection.x;
	float m2 = aLine1.myDirection.y / aLine1.myDirection.x;
	
	if (!std::isfinite(abs(m1)) || !std::isfinite(abs(m2)))
		return false;
	
	
	if (m1 != m2)
	{
		aOutIntersectPoint.x = (y2 - y1 + m1 * x1 - m2 * x2) / (m1 - m2);
		aOutIntersectPoint.y = m1 * (aOutIntersectPoint.x - x1) + y1;
		return true;
	}
	
	return false;*/
}
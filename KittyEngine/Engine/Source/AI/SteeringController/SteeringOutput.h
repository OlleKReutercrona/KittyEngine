#pragma once
#include <Engine/Source/Math/Vector2.h>

struct SteeringOutput
{
    Vector3f linear = { 0,0,0 };
    float angular = 0;

    void operator+=(SteeringOutput other) 
    {
        linear += other.linear;
        angular += other.angular;
    }
    SteeringOutput operator*(float aScalar) 
    {
        SteeringOutput newValue;

        newValue.linear.x = linear.x * aScalar;
        newValue.linear.y = linear.y * aScalar;
        newValue.linear.z = linear.z * aScalar;
        newValue.angular = angular * aScalar;

        return newValue;
    }
    SteeringOutput operator+(SteeringOutput aSteering) 
    {
        SteeringOutput newValue;

        newValue.linear.x = linear.x + aSteering.linear.x;
        newValue.linear.y = linear.y + aSteering.linear.y;
        newValue.linear.z = linear.z + aSteering.linear.z;
        newValue.angular = angular + aSteering.angular;

        return newValue;
    }
};
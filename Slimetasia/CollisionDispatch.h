#pragma once
#include "PhysicsDefs.h"

struct BaseDetectionAlgorithm;

class CollisionDispatch
{
public:
    CollisionDispatch() = default;

    virtual BaseDetectionAlgorithm* SelectAlgorithm(const CollisionShapeType& shape1, const CollisionShapeType& shape2) = 0;
};
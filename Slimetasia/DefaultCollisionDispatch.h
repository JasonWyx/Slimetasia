#pragma once
#include "CapsuleVsCapsule.h"
#include "CapsuleVsConvexPolygon.h"
#include "CollisionDispatch.h"
#include "CollisionMesh_3D.h"
#include "ConvexPolygonVsConvexPolygon.h"
#include "SphereVsCapsule.h"
#include "SphereVsConvexPolygon.h"
#include "SphereVsSphere.h"

class DefaultCollisionDispatch : public CollisionDispatch
{
public:
    DefaultCollisionDispatch() = default;

    ~DefaultCollisionDispatch() override = default;

    BaseDetectionAlgorithm* SelectAlgorithm(const CollisionShapeType& shape1, const CollisionShapeType& shape2) override;

protected:
    // variables
};
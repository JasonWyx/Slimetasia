#pragma once

#include "CapsuleVsCapsule.h"
#include "CapsuleVsConvexPolygon.h"
#include "CollisionDispatch.h"
#include "ConvexPolygonVsConvexPolygon.h"
#include "SphereVsCapsule.h"
#include "SphereVsConvexPolygon.h"
#include "SphereVsSphere.h"

class PECollisionDispatch : public CollisionDispatch
{
public:
    PECollisionDispatch() {}

    BaseDetectionAlgorithm* SelectAlgorithm(const CollisionShapeType& shape1, const CollisionShapeType& shape2) override;

protected:
    // variables
    CapsuleVsCapsule m_CapsuleCapsule;
    CapsuleVsConvexPolygon m_CapsuleConvex;
    ConvexPolygonVsConvexPolygon m_ConvexConvex;
    SphereVsCapsule m_SphereCapsule;
    SphereVsSphere m_SphereSphere;
    SphereVsConvexPolygon m_SphereConvex;
};

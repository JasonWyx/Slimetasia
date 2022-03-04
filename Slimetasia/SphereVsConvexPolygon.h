#pragma once
#include "BaseDetectionAlgorithm.h"

struct SphereVsConvexPolygon : public BaseDetectionAlgorithm
{
    SphereVsConvexPolygon() = default;

    ~SphereVsConvexPolygon() = default;

    void ComputeCollisionData(Manifold* manifold, RigidbodyComponent* first, RigidbodyComponent* second) override;
};

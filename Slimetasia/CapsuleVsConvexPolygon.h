#pragma once
#include "BaseDetectionAlgorithm.h"

struct CapsuleVsConvexPolygon : public BaseDetectionAlgorithm
{
    CapsuleVsConvexPolygon() = default;

    ~CapsuleVsConvexPolygon() = default;

    void ComputeCollisionData(Manifold* manifold, RigidbodyComponent* first, RigidbodyComponent* second) override;
};
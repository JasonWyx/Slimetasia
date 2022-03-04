#pragma once
#include "BaseDetectionAlgorithm.h"

struct SphereVsSphere : public BaseDetectionAlgorithm
{
    SphereVsSphere() = default;

    ~SphereVsSphere() = default;

    void ComputeCollisionData(Manifold* manifold, RigidbodyComponent* first, RigidbodyComponent* second) override;
};

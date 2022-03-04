#pragma once
#include "BaseDetectionAlgorithm.h"

struct SphereVsCapsule : public BaseDetectionAlgorithm
{
    SphereVsCapsule() = default;

    ~SphereVsCapsule() = default;

    void ComputeCollisionData(Manifold* manifold, RigidbodyComponent* first, RigidbodyComponent* second) override;
};

#pragma once
#include "BaseDetectionAlgorithm.h"

struct CapsuleVsCapsule : public BaseDetectionAlgorithm
{
    CapsuleVsCapsule() = default;

    ~CapsuleVsCapsule() = default;

    void ComputeCollisionData(Manifold* manifold, RigidbodyComponent* first, RigidbodyComponent* second) override;
};

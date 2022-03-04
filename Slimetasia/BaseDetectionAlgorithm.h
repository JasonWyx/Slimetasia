#pragma once

struct Manifold;
class RigidbodyComponent;

struct BaseDetectionAlgorithm
{
    BaseDetectionAlgorithm() = default;

    virtual ~BaseDetectionAlgorithm() = default;

    virtual void ComputeCollisionData(Manifold* manifold, RigidbodyComponent* first, RigidbodyComponent* second) = 0;
};

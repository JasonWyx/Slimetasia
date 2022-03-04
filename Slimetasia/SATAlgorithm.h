#pragma once
#include "CorePrerequisites.h"

struct Manifold;
class RigidbodyComponent;

class SATAlgorithm
{
    SATAlgorithm() = default;

    ~SATAlgorithm() = default;

    void CheckCollidingConvexVsConvex(Manifold* manifold, RigidbodyComponent* first, RigidbodyComponent* second);
};
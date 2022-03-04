#include "CapsuleVsConvexPolygon.h"

#include <cassert>

void CapsuleVsConvexPolygon::ComputeCollisionData(Manifold* manifold, RigidbodyComponent* first, RigidbodyComponent* second)
{
    // ensuring all parameters are valid.
    assert(manifold != nullptr && first != nullptr && second != nullptr);
}

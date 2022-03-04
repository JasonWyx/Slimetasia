#include "ConvexCollider.h"

Vector3 ConvexCollider::GetSupportPoint(const Vector3& dir) const
{
    return Vector3{};
}

REFLECT_INIT(ConvexCollider)
REFLECT_PARENT(CollisionMesh_3D)
REFLECT_END()

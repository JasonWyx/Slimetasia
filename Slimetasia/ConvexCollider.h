#pragma once
#include "CollisionMesh_3D.h"

class ConvexCollider : public CollisionMesh_3D
{
public:
    ConvexCollider(GameObject* parentObject = nullptr, const std::string& name = "CollisionMesh_3D", const CollisionShapeType& shapetype = eCollisionShapeType_CONVEX_POLY,
                   const CollisionShape& shape = eCollisionShape_CONVEX_MESH)
        : CollisionMesh_3D(parentObject, name, shapetype, shape)
    {
    }

    virtual ~ConvexCollider() = default;

    virtual Vector3 GetSupportPoint(const Vector3& dir) const;

    REFLECT()
};

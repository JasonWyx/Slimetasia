#pragma once
#include "MathDefs.h"

class RigidbodyComponent;
class MeshRenderer;

class AABB
{
public:

    AABB();
    AABB(const Vector3& min, const Vector3& max);
    AABB(RigidbodyComponent* rigidbody);

    static AABB BuildFromCenterAndHalfExtents(const Vector3& center, const Vector3& halfExtents);

    // Computes the volume of this aabb.
    float GetVolume() const;
    // Computes the surface area of this aabb.
    float GetSurfaceArea() const;

    // Does this aabb completely contain the given aabb (not an intersection test).
    bool Contains(const AABB& aabb) const;
    // Expand the to include the given point.
    void Expand(const Vector3& point);
    // Combine the two aabbs into a new one
    static AABB Combine(const AABB& lhs, const AABB& rhs);
    // See if this aabb is equal to another (with epsilon). Used for unit testing.
    bool Compare(const AABB& rhs, float epsilon) const;

    void Transform(const Vector3& scale, const Matrix3& rotation, const Vector3& translation);

    static bool AABBAABB(const Vector3& aabbMin0, const Vector3& aabbMax0, const Vector3& aabbMin1, const Vector3& aabbMax1);
    static bool RayAabb(const Vector3& rayStart, const Vector3& rayDir, const Vector3& aabbMin, const Vector3& aabbMax, float& t);

    Vector3 GetMin() const;
    Vector3 GetMax() const;
    Vector3 GetCenter() const;
    Vector3 GetHalfSize() const;

    void DebugDraw(const unsigned int& parentID) const;

    Vector3 m_Min;
    Vector3 m_Max;
};